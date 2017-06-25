

# Grit Engine

This is the central repository for the [Grit Game Engine](http://www.gritengine.com) project.

From here can be built the engine executable itself, the launcher, and various tools. These are
mostly useless without the accompanying media tree (the [Game
Directory](https://sourceforge.net/projects/gritengine/) which is available on Sourceforge via
Subversion. Therefore to get everything, execute the following:

```
git clone --recursive https://github.com/sparkprime/grit-engine.git grit-engine
svn checkout https://svn.code.sf.net/p/gritengine/code/trunk grit-engine/media
```

The subversion tree also contains prebuilt up-to-date executables (Linux & Windows) so the majority
of developers only need that. Grit can be substantially modified through Lua scripting, and this
potential should be exhausted before modifying C++ code.

Build files are provided for Linux (`Makefile` and `.../*grit.mk`) and Visual Studio 2013 project
files. Building C++ takes about an hour on Windows and 10 minutes on Linux. Scripts are available
for copying new executables into Subversion, if it is checked out in the `media/` directory.


# Windows

Only Visual Studio Express 2017 is supported. The "Community" version is free (as in beer). Download
it from the Microsoft site.


## Requirements

You will need the DirectX9 SDK (Google it), install that on your system (in `Program Files`). The
install adds a system-wide environment variable `DXSDK_DIR` pointing to the install directory. This
is used by the Visual Studio build. If Visual studio is running, you will have to restart it to make
it 'see' the new environment variable.


## Regular build

Open grit-engine.sln and build the whole solution with the *Normal* configuration. This will build
all the tools and dependencies.


## Debug Build

Debugging with Visual Studio requires the engine to be built with the *Debug* configuration. To run
in the debugger, execute the `engine` project from inside Visual Studio. You may need to set the
working directory to the `media/` directory (from the `engine` project properties).


## Modifying the Build

The build uses hand-written MSVC build files. Each executable and library has a project file, and
properties files are used to layer additional build options without duplicating them between project
files. They are structured as follows:
* 
`grit-engine.sln`: Collects together all the projects.
* 
`solution.props`: Build options for all libraries and executables. Options that are the same for
both Debug and Normal configurations live here.
* 
`solution_debug.props`: Additional options when compiling in debug mode. Options that are the same
for all object files live here.
* 
`solution_normal.props`: Additional options when compiling in normal mode. Options that are the same
for all object files live here.
* 
`pch.props`: Options for enabling the precompiled header, used for top-level apps.
* 
`path/to/my-project/my-project.vcxproj`: An executable or library to build. Build options that are
specific to the library itself (like warning levels) live here.
* 
`path/to/my-project/my-project.props`: Build options required by clients of a library and the
library itself (typically defines and include paths).


# Linux

The following instructions are for Ubuntu. If you're using another distro, you'll have to figure it
out for yourself but hopefully the Ubuntu instructions will help. Note that the make files require
GNU make, which may be called gmake on your system.


## Requirements

```
sudo apt-get install subversion g++ make pkg-config gdb valgrind \
libfreeimage-dev libzzip-dev libfreetype6-dev libglu1-mesa-dev \
libxt-dev libxaw7-dev libglew1.5-dev libxrandr-dev \
libgoogle-perftools-dev libopenal-dev libreadline-dev freeglut3-dev \
nvidia-cg-toolkit libvorbis-dev xutils-dev libicu-dev
```


## Building

Simply running `make -j 8` in the root (adjust for your number of cores) will build everything.
Executables for the current platform are left in the root directory. You can add it to your PATH.


## Debugging

You can debug Grit with `gdb` or `valgrind`. If the assembly is too obscure, disable optimizations
by overriding the OPT variable as so:

```
make -j 8 OPT=
```

Note that this will not rebuild anything that is already built, so you might want to first delete
specific object files -- the ones containing the code you're debugging, and then rebuilding them
without optimizations.


## Modifying the Build

The makefiles are handwritten. They use a lot of GNU make specific features. Each sub-project and
dependency defines a `grit.mk` file which defines the additional source files and required build
options. These are all included by the root `Makefile` which computes the actual build rules.
