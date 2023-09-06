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

#include "register_types.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

#include "ffmpeg_video_stream.h"
#include "video_stream_ffmpeg_loader.h"

Ref<VideoStreamFFMpegLoader> ffmpeg_loader;

static void print_codecs() {
    const AVCodecDescriptor *desc = NULL;
    char msg[512] = { 0 };
    UtilityFunctions::print("Supported video codecs:");
    while ((desc = avcodec_descriptor_next(desc))) {
        const AVCodec *codec = NULL;
        void *i = NULL;
        bool found = false;
        while ((codec = av_codec_iterate(&i))) {
            if (codec->id == desc->id && av_codec_is_decoder(codec)) {
                if (!found && (avcodec_find_decoder(desc->id) || avcodec_find_encoder(desc->id))) {
                    snprintf(msg, sizeof(msg) - 1, "\t%s%s%s",
                            avcodec_find_decoder(desc->id) ? "decode " : "",
                            avcodec_find_encoder(desc->id) ? "encode " : "",
                            desc->name);
                    found = true;
                    UtilityFunctions::print(msg);
                }
                if (strcmp(codec->name, desc->name) != 0) {
                    snprintf(msg, sizeof(msg) - 1, "\t  codec: %s", codec->name);
                    UtilityFunctions::print(msg);
                }
            }
        }
    }
}

void initialize_ffmpeg_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    print_codecs();
    GDREGISTER_ABSTRACT_CLASS(FFmpegVideoStreamPlayback);
    GDREGISTER_ABSTRACT_CLASS(VideoStreamFFMpegLoader);
    GDREGISTER_CLASS(FFmpegVideoStream);
    ffmpeg_loader.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(ffmpeg_loader);
}

void uninitialize_ffmpeg_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    ResourceLoader::get_singleton()->remove_resource_format_loader(ffmpeg_loader);
    ffmpeg_loader.unref();
}

extern "C" {

GDExtensionBool GDE_EXPORT ffmpeg_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(&initialize_ffmpeg_module);
    init_obj.register_terminator(&uninitialize_ffmpeg_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SERVERS);

    return init_obj.init();
}

} // extern "C"
