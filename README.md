# FFmpegPlayer

A derivative work of [EIRTeam.FFmpeg](https://github.com/EIRTeam/EIRTeam.FFmpeg),
Licensed under MIT license (see [LICENSE-EIRTeam-gdextension.txt](addons/ffmpegplayer/LICENSE-EIRTeam-gdextension.txt))

This extension also uses parts of the [FFmpeg](https://ffmpeg.org/) library,  
Licensed under the LGPL license (see [LICENSE-ffmpeg.txt](addons/ffmpegplayer/LICENSE-ffmpeg.txt))  
(Compiled with non-GPL and non-free options)

Note that on Linux specifically, you need to install these libraries in order for this extension to work on either end (for some reason).
```sh
sudo apt-get install libva2 libva-drm2 libva-x11-2 libvdpau1 -y
```

## How to Use

First and foremost, you will need to install [Git](https://git-scm.com/downloads) in any way that works. Going forward I'll assume you've already done so, so I won't include it in future dependency installation code.

To add this GDextension to your Godot project, press the green `Code` button to download this repository as a zip or clone it somewhere using Git. Then, copy and paste the `addons` directory next to `project.godot`. Then, when you open the project, you'll need to restart the editor so the changes can take effect. Once that's done, `VideoStreamFFmpeg` and other classes will be avaliable to use. However, if exporting doesn't work and there's an error saying that the GDExtension can't be found, you could try deleting the cache in the `.godot` folder and re-import everything to fix it.

You also need to make sure the proper licensing attributions and actions are taken by the project. For FFmpeg, be sure to refer to [FFmpeg License and Legal Considerations](https://www.ffmpeg.org/legal.html) for more information.

## How to Build

This GDExtension uses [CMake](https://cmake.org/) to build the libraries.

* You can also install CMake through Homebrew with `brew install cmake`. If you don't have Homebrew, follow the installation instructions [here](https://brew.sh/) to install it.

Before continuing, I will assume you already have [Git](https://git-scm.com/downloads) installed. However if you do not, you will need to install it beforehand because the Code Blocks for installing dependencies will NOT contain Git.

Submodules of the [Godot C++ Bindings](https://github.com/godotengine/godot-cpp) and [FFmpeg](https://github.com/FFmpeg/FFmpeg) are included. You will need to run `git submodule init && git submodule update` within the root folder to get them to work. For the bindings and GDExtension, the minimum is `4.1`, however you can upgrade it to a later version if you desire.

The FFmpeg libraries for Windows, Linux, and macOS are included and do not need to be rebuilt. However you could also update them as well. Before doing so, you may need to rename `time.h` to `fftime.h` to fix file name conflicts on compilation of the GDExtension.

Built binaries need to be manually copied into the `addons/ffmpegplayer/win64` directory, or `../linux64`, `../macos`, etc. There isn't a build script to copy them over, sorry.

You can also use the build tools from [EIRTeam.FFmpeg](https://github.com/EIRTeam/EIRTeam.FFmpeg) to create the libraries. However, the FFmpeg libraries will need to be changed out for license compliance.

### Windows Setup

The libraries can be built easily using [ffmpeg-windows-build-helpers](https://github.com/rdp/ffmpeg-windows-build-helpers) on a **Linux system or Virtual Machine**. Before running the script, you'll need to install these dependencies:

```sh
sudo apt-get update
sudo apt-get install subversion ragel curl texinfo g++ ed bison flex cvs yasm automake libtool autoconf gcc cmake make pkg-config zlib1g-dev unzip pax nasm gperf autogen bzip2 autoconf-archive p7zip-full meson clang python-is-python3 -y
```

* Please notify me if there are any after `python-is-python3` that you do NOT need to install, I just copied this from the Linux command to make sure ti actually compiles.

To compile the Godot CPP Bindings and GDExtension for Windows on Linux, you'll also need to install these:
```sh
sudo apt install g++-mingw-w64-x86-64-posix -y
```

Before running the script however, if you're using a version of Ubuntu that's 24.04 or higher, you WILL have to set FFmpeg to an older version. Specifically [n6.1.2](https://github.com/FFmpeg/FFmpeg/tree/n6.1.2) at most. Using the latest version currently causes a fatal error when compiling.

* While the contained FFmpeg submodule is already set to this version, you can reset to this tag on any copy of the FFmpeg repository by running `git reset --hard n6.1.2` from the folder.
* You'll also need to fix a line in `FFmpeg/libavcodec/libsvtav1.c` that's causing the compilation to break.

```sh
# This is the line to replace
svt_ret = svt_av1_enc_init_handle(&svt_enc->svt_handle, svt_enc, &svt_enc->enc_params);
# This is what to replace it with
svt_ret = svt_av1_enc_init_handle(&svt_enc->svt_handle, &svt_enc->enc_params);
```

Once you have all the dependencies, use the following command in the Build Helpers root:

```sh
./cross_compile_ffmpeg.sh --build-ffmpeg-static=n --build-ffmpeg-shared=y --gcc-cpu-count=3 --compiler-flavors=win64 --enable-gpl=n
```

After building, the needed contents are found inside `sandbox/win64/ffmpeg_git_lgpl_master_shared/bin`. Yes, creating a whole Linux install just to use this tool is easier than trying to build FFmpeg on Windows.

### Linux Setup

Before continuing, ensure you have all of the dependencies required for configuring FFmpeg:

```sh
sudo apt-get update
sudo apt-get install libass-dev libgnutls28-dev libmp3lame-dev libsdl2-dev libva-dev libvdpau-dev libvorbis-dev libxcb-xfixes0-dev meson ninja-build pkg-config yasm nasm -y
```

### macOS Setup

Before continuing, ensure XCode, XCode Command Line Tools, and CMake are all installed. You can get XCode for free on the App Store. To install all the XCode tools, run `xcode-select --install`. This will include the Command Line Tools.

Then, to install the dependencies, you'll need to install [HomeBrew](https://brew.sh/) first, then run the following command:
```sh
brew install yasm automake fdk-aac lame libass libtool libvorbis libvpx opus sdl12-compat shtool texi2html theora x264 x265 xvid nasm
```

### Build Godot Bindings

Since the repositories are already included, in the `godot-cpp` directory, run the following commands to build the bindings:

```sh
cmake ./CMakeLists.txt -DCMAKE_BUILD_TYPE=Release
make
```

For macOS, append this flag to the above: `-DCMAKE_OSX_ARCHITECTURES=x86_64`.

#### Windows Bindings

If you're building for Windows and you've compiled the FFmpeg libraries on your Linux machine/VM, switch back over to your Windows. To compile the Godot CPP  bindings, you'll need to run the code above, but replace `make` with `cmake --build .` to build the bindings.

### Building FFmpeg

If you're building for Windows, you can skip this next sub-section. However, if you are not compiling for Windows, this next section applies to both Linux and MacOS.

#### Using FFmpeg Configure

Due to the fact that FFmpeg updates incredibly frequently, it's recommended to use a specific commit for compiling for each OS. However, it is not required.
Once the setup is complete, build the libraries using the following commands in the FFmpeg submodule:

```sh
./configure --enable-pthreads --enable-shared --disable-static --disable-zlib --disable-programs --disable-doc --disable-manpages --disable-podpages --disable-txtpages --disable-ffplay --disable-ffprobe --disable-ffmpeg --arch=x86_64
make
```

To cross-compile to MacOS from Linux, install the [homebrew-macos-cross-toolchains](https://github.com/messense/homebrew-macos-cross-toolchains), then use these commands:

```sh
./configure --enable-pthreads --enable-shared --disable-static --disable-zlib --disable-programs --disable-doc --disable-manpages --disable-podpages --disable-txtpages --disable-ffplay --disable-ffprobe --disable-ffmpeg --arch=x86_64  --target-os=linux --cross-prefix=x86_64-linux-gnu-
make
```

After building, there should be several new files inside FFmpeg. Copy these files into `FFmpegPlayer/libraries/x64`, Their name may be slightly different than what's listed below depending on the version you've compiled, but should still be easy to find using these folders:

* avcodec
* avformat
* avutil
* swresample
* swscale

If compiled for Windows, these files will be formated as "[folder]-[ver].dll" and "[folder].lib" with the prior path being `./ffmpeg-windows-build-helpers/sandbox/win64/ffmpeg_git_lgpl_master_shared/bin`. If compiled for Linux, they will be formatted like "lib[folder].so.[ver]". If compiled for macOS, the files will be "lib[folder].[ver].dylib". For Linux and MacOS, the path would just be `./FFmpeg`.

### Build GDExtension

To build the GDExtension, open Terminal and set the current directory to the root folder. Once that's set, use the following commands:

```sh
cmake ./CMakeLists.txt -DCMAKE_BUILD_TYPE=Release
make
```

This will create the main library in the bin folder with your OS. Finally, copy all the FFmpeg libraries you put in the `libraries` folder and the ones in the `bin` folder, into the respective OS's directory inside the addon.

#### Building for Windows

For Windows, you will need to grab `libavutil/avconfig.h` from your Linux build/VM and copy it into the Windows one so it compiles correctly. Then, use the same `cmake ./CMakeLists.txt -DCMAKE_BUILD_TYPE=Release` as above in the root folder and replace `make` with `cmake --build .` to build the GDExtension.

### Test Godot Project

Once you've built the GDExtension, you're now ready to use it in your projects. However, you also need to test it to make sure it works. You can do this by doing the following:

* Download [Godot 4.1](https://github.com/godotengine/godot-builds/releases/tag/4.1-stable) or greater, and create a new
project, or open an existing project.
* Copy the `addons` folder in this repository into your Godot project. It should tell you that the editor needs to restart for changes to take effect.
* Test the FFmpegPlayer by using a non-ogv video as an `VideoStreamFFmpeg` and set the `VideoStreamPlayer`'s stream to that.
