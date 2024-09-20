/**************************************************************************/
/*  video_stream_ffmpeg.cpp                                               */
/**************************************************************************/
/*                     The original file was part of:                     */
/*                             EIRTeam.FFmpeg                             */
/*                         https://ph.eirteam.moe                         */
/**************************************************************************/
/* Copyright (c) 2023-present Álex Román (EIRTeam) & contributors.        */
/*                                                                        */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/
/* The below code has been modified from the original for purposes of     */
/* code clarity, removing unneeded files and functions, and integrating   */
/* with a different project layout and build tool set, but no significant */
/* changes were made to functionality have been made.                     */
/**************************************************************************/

#include "video_stream_ffmpeg.h"

#include <godot_cpp/variant/utility_functions.hpp>

void VideoStreamFFmpegPlayback::seek_into_sync() {
    decoder->seek(playback_position);
    Vector<Ref<DecodedFrame>> decoded_frames;
    for (Ref<DecodedFrame> df : available_frames) {
        decoded_frames.push_back(df);
    }
    decoder->return_frames(decoded_frames);
    available_frames.clear();
    available_audio_frames.clear();
}

double VideoStreamFFmpegPlayback::get_current_frame_time() {
    if (last_frame.is_valid()) {
        return last_frame->get_time();
    }
    return 0.0f;
}

bool VideoStreamFFmpegPlayback::check_next_frame_valid(Ref<DecodedFrame> p_decoded_frame) {
    // in the case of looping, we may start a seek back to the beginning but still receive some lingering frames from the end of the last loop. these should be allowed to continue playing.
    if (looping && Math::abs((p_decoded_frame->get_time() - decoder->get_duration()) - playback_position) < LENIENCE_BEFORE_SEEK)
        return true;

    return p_decoded_frame->get_time() <= playback_position && Math::abs(p_decoded_frame->get_time() - playback_position) < LENIENCE_BEFORE_SEEK;
}

bool VideoStreamFFmpegPlayback::check_next_audio_frame_valid(Ref<DecodedAudioFrame> p_decoded_frame) {
    // in the case of looping, we may start a seek back to the beginning but still receive some lingering frames from the end of the last loop. these should be allowed to continue playing.
    if (looping && Math::abs((p_decoded_frame->get_time() - decoder->get_duration()) - playback_position) < LENIENCE_BEFORE_SEEK)
        return true;

    return p_decoded_frame->get_time() <= playback_position && Math::abs(p_decoded_frame->get_time() - playback_position) < LENIENCE_BEFORE_SEEK;
}

void VideoStreamFFmpegPlayback::_update(double p_delta) {
    if (paused || !playing) {
        return;
    }

    playback_position += p_delta * 1000.0f;

    if (decoder->get_decoder_state() == VideoDecoder::DecoderState::END_OF_STREAM && available_frames.size() == 0) {
        // if at the end of the stream but our playback enters a valid time region again, a seek operation is required to get the decoder back on track.
        if (playback_position < decoder->get_duration()) {
            seek_into_sync();
        } else {
            playing = false;
        }
    }

    Ref<DecodedFrame> peek_frame = available_frames.size() > 0 ? available_frames.front()->get() : nullptr;
    bool out_of_sync = false;

    if (peek_frame.is_valid()) {
        out_of_sync = Math::abs(playback_position - peek_frame->get_time()) > LENIENCE_BEFORE_SEEK;

        if (looping) {
            out_of_sync &= Math::abs(playback_position - decoder->get_duration() - peek_frame->get_time()) > LENIENCE_BEFORE_SEEK &&
                    Math::abs(playback_position + decoder->get_duration() - peek_frame->get_time()) > LENIENCE_BEFORE_SEEK;
        }
    }

    if (out_of_sync) {
        UtilityFunctions::print(vformat("Video too far out of sync (%.2f), seeking to %.2f", peek_frame->get_time(), playback_position));
        seek_into_sync();
    }

    double frame_time = get_current_frame_time();

    bool got_new_frame = false;

    List<Ref<DecodedFrame>>::Element *next_frame = available_frames.front();
    while (next_frame && (check_next_frame_valid(next_frame->get()) || just_seeked)) {
        just_seeked = false;

        if (last_frame.is_valid()) {
            decoder->return_frame(last_frame);
        }
        last_frame = next_frame->get();
        last_frame_image = last_frame->get_image();
#ifdef FFMPEG_MT_GPU_UPLOAD
        last_frame_texture = last_frame->get_texture();
#endif
        got_new_frame = true;
        next_frame = next_frame->next();
        available_frames.pop_front();
    }
#ifndef FFMPEG_MT_GPU_UPLOAD
    if (got_new_frame) {
        if (texture.is_valid()) {
            if (texture->get_size() != last_frame_image->get_size() || texture->get_format() != last_frame_image->get_format()) {
                texture->set_image(last_frame_image); // should never happen, but life has many doors ed-boy...
            } else {
                texture->update(last_frame_image);
            }
        }
    }
#endif

    if (available_frames.size() == 0) {
        for (Ref<DecodedFrame> frame : decoder->get_decoded_frames()) {
            available_frames.push_back(frame);
        }
    }

    Ref<DecodedAudioFrame> peek_audio_frame;
    if (available_audio_frames.size() > 0) {
        peek_audio_frame = available_audio_frames.front()->get();
    }

    bool audio_out_of_sync = false;

    if (peek_audio_frame.is_valid()) {
        audio_out_of_sync = Math::abs(playback_position - peek_audio_frame->get_time()) > LENIENCE_BEFORE_SEEK;

        if (looping) {
            out_of_sync &= Math::abs(playback_position - decoder->get_duration() - peek_audio_frame->get_time()) > LENIENCE_BEFORE_SEEK &&
                    Math::abs(playback_position + decoder->get_duration() - peek_audio_frame->get_time()) > LENIENCE_BEFORE_SEEK;
        }
    }

    if (audio_out_of_sync) {
        // TODO: seek audio stream individually if it desyncs
    }

    List<Ref<DecodedAudioFrame>>::Element *next_audio_frame = available_audio_frames.front();
    while (next_audio_frame && check_next_audio_frame_valid(next_audio_frame->get())) {
        Ref<DecodedAudioFrame> audio_frame = next_audio_frame->get();
        int sample_count = audio_frame->get_sample_data().size() / decoder->get_audio_channel_count();
        mix_audio(sample_count, audio_frame->get_sample_data(), 0);
        next_audio_frame = next_audio_frame->next();
        available_audio_frames.pop_front();
    }
    if (available_audio_frames.size() == 0) {
        for (Ref<DecodedAudioFrame> frame : decoder->get_decoded_audio_frames()) {
            available_audio_frames.push_back(frame);
        }
    }

    buffering = decoder->is_running() && available_frames.size() == 0;

    if (frame_time != get_current_frame_time()) {
        frames_processed++;
    }
}

Error VideoStreamFFmpegPlayback::load(Ref<FileAccess> p_file_access) {
    decoder = Ref<VideoDecoder>(memnew(VideoDecoder(p_file_access)));

    decoder->start_decoding();
    Vector2i size = decoder->get_size();
    if (decoder->get_decoder_state() == VideoDecoder::FAULTED) {
        return FAILED;
    }

    texture = ImageTexture::create_from_image(Image::create(size.x, size.y, false, Image::FORMAT_RGBA8));
    return OK;
}

bool VideoStreamFFmpegPlayback::_is_paused() const {
    return paused;
}

bool VideoStreamFFmpegPlayback::_is_playing() const {
    return playing;
}

void VideoStreamFFmpegPlayback::_set_paused(bool p_paused) {
    paused = p_paused;
}

void VideoStreamFFmpegPlayback::_play() {
    if (decoder->get_decoder_state() == VideoDecoder::FAULTED) {
        playing = false;
        return;
    }
    clear();
    playback_position = 0;
    decoder->seek(0, true);
    just_seeked = true;
    playing = true;
}

void VideoStreamFFmpegPlayback::_stop() {
    if (playing) {
        clear();
        playback_position = 0.0f;
        decoder->seek(playback_position, true);
        just_seeked = true;
    }
    playing = false;
}

void VideoStreamFFmpegPlayback::_seek(double p_time) {
    decoder->seek(p_time * 1000.0f);
    just_seeked = true;
    available_frames.clear();
    available_audio_frames.clear();
    playback_position = p_time * 1000.0f;
}

double VideoStreamFFmpegPlayback::_get_length() const {
    return decoder->get_duration() / 1000.0f;
}

Ref<Texture2D> VideoStreamFFmpegPlayback::_get_texture() const {
#ifdef FFMPEG_MT_GPU_UPLOAD
    return last_frame_texture;
#else
    return texture;
#endif
}

double VideoStreamFFmpegPlayback::_get_playback_position() const {
    return playback_position / 1000.0;
}

int VideoStreamFFmpegPlayback::_get_mix_rate() const {
    return decoder->get_audio_mix_rate();
}

int VideoStreamFFmpegPlayback::_get_channels() const {
    return decoder->get_audio_channel_count();
}

VideoStreamFFmpegPlayback::VideoStreamFFmpegPlayback() {
}

void VideoStreamFFmpegPlayback::clear() {
    last_frame.unref();
    last_frame_texture.unref();
    available_frames.clear();
    available_audio_frames.clear();
    frames_processed = 0;
    playing = false;
}
