# FFmpegPlayer

A derivative work of [EIRTeam.FFmpeg](https://github.com/EIRTeam/EIRTeam.FFmpeg/tree/master),  
Licensed under MIT license (see [LICENSE-EIRTeam-gdextension.txt](addons/ffmpegplayer/LICENSE-EIRTeam-gdextension.txt))

This extension also uses parts of the [FFmpeg](https://ffmpeg.org/) library,  
Licensed under the LGPL license (see [LICENSE-ffmpeg.txt](addons/ffmpegplayer/LICENSE-ffmpeg.txt))  
(Compiled with non-GPL and non-free options)

# How to Use

Should be as simple as copy/pasting the `addons` directory next to the godot project, then the ffmpeg player or something will be visible in the editor.

Also need to make sure the proper licensing attributions and actions are taken by the project **to be documented later**

# How to Build

Use [CMake](https://cmake.org/), don't use SCons

You'll also need the [godot c++ bindings](https://github.com/godotengine/godot-cpp) for the appropriate version of godot. Clone that so that the godot-cpp directory is in this gdextension directory, then build it

The FFmpeg libraries are included and should not need to be rebuilt, but just in case, they can be built using [ffmpeg-windows-build-helpers](https://github.com/rdp/ffmpeg-windows-build-helpers) on a **Linux system** using the following command line:

```
./cross_compile_ffmpeg.sh --build-ffmpeg-static=n --build-ffmpeg-shared=y --gcc-cpu-count=3 --compiler-flavors=multi --enable-gpl=n --disable-nonfree=y
```

After building, the needed contents are found inside `sandbox/win64/ffmpeg_git_lgpl_master_shared/bin`

Yes, creating a whole Linux install just to use this tool is easier than trying to build FFmpeg on Windows.

For linux/unix/MacOS compilation of FFmpeg, it is easiest to use the [FFmpeg repo](https://github.com/FFmpeg/FFmpeg) and the following configure options:

```
./configure --enable-pthreads --enable-shared --disable-static --disable-zlib --disable-programs --disable-doc --disable-manpages --disable-podpages --disable-txtpages --disable-ffplay --disable-ffprobe --disable-ffmpeg --arch=x86_64
```

You may also need to rename `time.h` to `fftime.h` to fix file name conflicts on compilation of the gdextension.

Built binaries need to be manually copied into the `addons/ffmpegplayer/win64` directory, there isn't a build script to copy them, sorry

Or, yaknow, it's functionally identical to the very first link in this readme, so just download and build that if you want, but the FFmpeg dlls will need to be changed out for license compliance.