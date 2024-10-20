# FFmpegPlayer

A derivative work of [EIRTeam.FFmpeg](https://github.com/EIRTeam/EIRTeam.FFmpeg),
Licensed under MIT license (see [LICENSE-EIRTeam-gdextension.txt](addons/ffmpegplayer/LICENSE-EIRTeam-gdextension.txt))

This extension also uses parts of the [FFmpeg](https://ffmpeg.org/) library,  
Licensed under the LGPL license (see [LICENSE-ffmpeg.txt](addons/ffmpegplayer/LICENSE-ffmpeg.txt))  
(Compiled with non-GPL and non-free options)

## How to Use

To add this GDextension to your Godot project, copy and paste the `addons` directory next to the project file. Then, when you open the project, you'll need to restart the editor so the changes can take effect. Once that's done, `VideoStreamFFmpeg` and other classes will be avaliable to use. However, if exporting doesn't work and there's an error saying that the GDExtension can't be found, you could delete the cache in the `.godot` folder and re-import everything to fix it.

You also need to make sure the proper licensing attributions and actions are taken by the project. For FFmpeg, be sure to refer to [FFmpeg License and Legal Considerations](https://www.ffmpeg.org/legal.html) for more information.

## How to Build

This GDExtension uses [CMake](https://cmake.org/) to build the libraries.

* You can also install CMake through Homebrew with `brew install cmake`. If you don't have Homebrew, follow the installation instructions [here](https://brew.sh/) to install it.

Submodules of the [Godot C++ Bindings](https://github.com/godotengine/godot-cpp) and [FFmpeg](https://github.com/FFmpeg/FFmpeg) are included. You will need to run `git submodule init` within the root folder to get them to work. For the bindings and GDExtension, the minimum is `4.1`, however you can upgrade it to a later version if you desire.

The FFmpeg libraries for Windows, Linux, and macOS are included and do not need to be rebuilt. However you could also update them as well. Before doing so, you may need to rename `time.h` to `fftime.h` to fix file name conflicts on compilation of the GDExtension.

Built binaries need to be manually copied into the `addons/ffmpegplayer/win64` directory, or `../linux64`, `../macos`, etc. There isn't a build script to copy them, sorry.

You can also use the build tools from [EIRTeam.FFmpeg](https://github.com/EIRTeam/EIRTeam.FFmpeg) to create the libraries. However, the FFmpeg libraries will need to be changed out for license compliance.

### Windows Setup

TBA

### Linux Setup

TBA

### macOS Setup

Before continuing, ensure XCode, XCode Command Line Tools, and CMake are all installed. You can get XCode for free on the App Store. To install all the XCode tools, run `xcode-select --install`. This will include the Command Line Tools.

### Build Godot Bindings

Since the repositories are already included, in the `godot-cpp` directory, run the following commands to build the bindings:

```sh
cmake ./CMakeLists.txt -DCMAKE_BUILD_TYPE=Release
make
```

For macOS, append the flag `-DCMAKE_OSX_ARCHITECTURES=x86_64`.

### Building FFmpeg

The libraries can be built easily using [ffmpeg-windows-build-helpers](https://github.com/rdp/ffmpeg-windows-build-helpers) on a **Linux system** using the following command line:

```sh
./cross_compile_ffmpeg.sh --build-ffmpeg-static=n --build-ffmpeg-shared=y --gcc-cpu-count=3 --compiler-flavors=multi --enable-gpl=n --disable-nonfree=y
```

After building, the needed contents are found inside `sandbox/win64/ffmpeg_git_lgpl_master_shared/bin`. Yes, creating a whole Linux install just to use this tool is easier than trying to build FFmpeg on Windows.

However, if you don't know how to use the helpers, or you keep getting errors trying to use it, you can alternatively use the configure file.

#### Using FFmpeg Configure

Due to the fact that FFmpeg updates incredibly frequently, it's recommended to use a specific commit for compiling for each OS.

* The commit currently used for FFmpegPlayer is [a577d31](https://github.com/FFmpeg/FFmpeg/commit/a577d313b2c14c855ab8aa69bbe3527bd7727212)
* You can reset to this commit by running these commands from the root folder:

```sh
cd FFmpeg
git reset --hard a577d313b2c14c855ab8aa69bbe3527bd7727212
```

Once the setup is complete, build the libraries using the following commands in the FFmpeg submodule:

```sh
./configure --enable-pthreads --enable-shared --disable-static --disable-zlib --disable-programs --disable-doc --disable-manpages --disable-podpages --disable-txtpages --disable-ffplay --disable-ffprobe --disable-ffmpeg --arch=x86_64
make
```

To cross-compile to Windows from a Unix system, use these commands:

```sh
./configure --enable-shared --disable-static --disable-zlib --disable-programs --disable-doc --disable-manpages --disable-podpages --disable-txtpages --disable-ffplay --disable-ffprobe --disable-ffmpeg --arch=x86_64 --target-os=mingw32 --cross-prefix=x86_64-w64-mingw32-
make
```

To cross-compile to MacOS from Linux, install the [homebrew-macos-cross-toolchains](https://github.com/messense/homebrew-macos-cross-toolchains), then use these commands:

```sh
./configure --enable-pthreads --enable-shared --disable-static --disable-zlib --disable-programs --disable-doc --disable-manpages --disable-podpages --disable-txtpages --disable-ffplay --disable-ffprobe --disable-ffmpeg --arch=x86_64  --target-os=linux --cross-prefix=x86_64-linux-gnu-
make
```

After building, there should be several new files inside FFmpeg. Copy these files into `FFmpegPlayer/libraries/x64`, Their name may be slightly different than what's listed below depending on the version you've compiled, but should still be easy to find using these folders:

* avcodec
* avdevice
* avfilter
* avformat
* avutil
* swresample
* swscale

If compiled for Windows, these files will be formated as "[folder]-[ver].dll". If compiled for Linux, they will be formatted like "lib[folder].so.[ver]". If compiled for macOS, the files will be "lib[folder].[ver].dylib"

### Build GDExtension

To build the GDExtension, open Terminal and set the current directory to the root folder. Once that's set, use the following commands:

```sh
cmake ./CMakeLists.txt
make
```

This will create the main library in the bin folder with your OS. Finally, copy all the FFmpeg libraries you put in the `libraries` folder and the ones in the `bin` folder, into the respective OS's directory inside the addon.

### Test Godot Project

Once you've built the GDExtension, you're now ready to use it in your projects. However, you also need to test it to make sure it works. You can do this by doing the following:

* Download [Godot 4.1](https://github.com/godotengine/godot-builds/releases/tag/4.1-stable) or greater, and create a new
project, or open an existing project.
* Copy the `addons` folder in this repository into your Godot project. It should tell you that the editor needs to restart for changes to take effect.
* Test the FFmpegPlayer by using a non-ogv video as an `VideoStreamFFmpeg` and set the `VideoStreamPlayer`'s stream to that.
