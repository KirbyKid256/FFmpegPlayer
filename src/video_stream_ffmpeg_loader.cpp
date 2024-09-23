/**************************************************************************/
/*  video_stream_ffmpeg_loader.cpp                                        */
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

#include "video_stream_ffmpeg_loader.h"
extern "C" {
#include "libavformat/avformat.h"
}
#include "video_stream_ffmpeg.h"

void VideoStreamFFmpegLoader::_update_recognized_extension_cache() const {
    if (recognized_extension_cache.size() > 0) {
        return;
    }
    void *iteration_state = nullptr;
    const AVInputFormat *current_fmt = nullptr;

    while ((current_fmt = av_demuxer_iterate(&iteration_state)) != nullptr) {
        if (current_fmt->extensions == nullptr) {
            continue;
        }
        PackedStringArray demuxer_exts = String(current_fmt->extensions).split(",", false);
        const_cast<VideoStreamFFmpegLoader *>(this)->recognized_extension_cache.append_array(demuxer_exts);
    }
}

String VideoStreamFFmpegLoader::_get_resource_type(const String &p_path) const {
    _update_recognized_extension_cache();
    if (recognized_extension_cache.has(p_path.get_extension())) {
        return "VideoStreamFFmpeg";
    }
    return "";
}

Ref<Resource> VideoStreamFFmpegLoader::load_internal(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) const {
    Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
    if (f.is_null()) {
        if (r_error) {
            *r_error = ERR_CANT_OPEN;
        }
        return Ref<Resource>();
    }

    Ref<VideoStreamFFmpeg> stream;
    stream.instantiate();
    stream->set_file(p_path);

    if (r_error) {
        *r_error = OK;
    }

    return stream;
}

bool VideoStreamFFmpegLoader::_handles_type(const StringName &p_type) const {
    return ClassDB::is_parent_class(p_type, "VideoStream");
}

PackedStringArray VideoStreamFFmpegLoader::_get_recognized_extensions() const {
    _update_recognized_extension_cache();
    return recognized_extension_cache;
}

Variant VideoStreamFFmpegLoader::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
    return load_internal(p_path, p_original_path, nullptr, p_use_sub_threads, nullptr, (CacheMode)p_cache_mode);
}
