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

#ifndef ET_VIDEO_STREAM_H
#define ET_VIDEO_STREAM_H

// Headers for building as GDExtension plug-in.
#include <godot_cpp/classes/video_stream.hpp>
#include <godot_cpp/classes/video_stream_playback.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/templates/list.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;


#include "video_decoder.h"

class FFmpegVideoStreamPlayback : public VideoStreamPlayback {
    GDCLASS(FFmpegVideoStreamPlayback, VideoStreamPlayback);
    const int LENIENCE_BEFORE_SEEK = 2500;
    double playback_position = 0.0f;

    Ref<VideoDecoder> decoder;
    List<Ref<DecodedFrame>> available_frames;
    List<Ref<DecodedAudioFrame>> available_audio_frames;
    Ref<DecodedFrame> last_frame;
#ifndef FFMPEG_MT_GPU_UPLOAD
    Ref<ImageTexture> last_frame_texture;
#endif
    Ref<Image> last_frame_image;
    Ref<ImageTexture> texture;
    bool looping = false;
    bool buffering = false;
    int frames_processed = 0;
    void seek_into_sync();
    double get_current_frame_time();
    bool check_next_frame_valid(Ref<DecodedFrame> p_decoded_frame);
    bool check_next_audio_frame_valid(Ref<DecodedAudioFrame> p_decoded_frame);
    bool paused = false;
    bool playing = false;

protected:
    void clear();
    static void _bind_methods(){}; // Required by GDExtension, do not remove

public:
    void load(Ref<FileAccess> p_file_access);

    bool _is_paused() const;
    void _update(double p_delta);
    bool _is_playing() const;
    void _set_paused(bool p_paused);
    void _play();
    void _stop();
    void _seek(double p_time);
    double _get_length() const;
    Ref<Texture2D> _get_texture() const;
    double _get_playback_position() const;
    int _get_mix_rate() const;
    int _get_channels() const;

    FFmpegVideoStreamPlayback();
};

class FFmpegVideoStream : public VideoStream {
    GDCLASS(FFmpegVideoStream, VideoStream);

protected:
    static void _bind_methods(){}; // Required by GDExtension, do not remove

public:
    Ref<VideoStreamPlayback> _instantiate_playback() {
        Ref<FileAccess> fa = FileAccess::open(get_file(), FileAccess::READ);
        if (!fa.is_valid()) {
            return Ref<VideoStreamPlayback>();
        }
        Ref<FFmpegVideoStreamPlayback> pb;
        pb.instantiate();
        pb->load(fa);
        return pb;
    }
};

#endif // ET_VIDEO_STREAM_H
