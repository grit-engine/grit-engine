# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

COMPILER=c:/mingw/bin/g++ -W -Wall -ansi

THREADED=semithreaded

OGRE_BUILD_DIR=c:/media/cygwin/home/spark/ogrebuild/shoggoth/cvs_local

DEPENDENCIES_DIR=$(OGRE_BUILD_DIR)/../../dependencies_mingw
BOOST_DIR=../../boost-trunk

OGRE_CFLAGS=-isystem $(DEPENDENCIES_DIR)/include \
            -I $(BOOST_DIR) \
            -isystem $(OGRE_BUILD_DIR)/OgreMain/include \
            -isystem $(OGRE_BUILD_DIR)/RenderSystems/Direct3D9/include \
            -isystem $(OGRE_BUILD_DIR)/RenderSystems/GL/src/win32 \
            -isystem $(OGRE_BUILD_DIR)/RenderSystems/GL/src/nvparse \
            -isystem $(OGRE_BUILD_DIR)/RenderSystems/GL/src/atifs/include \
            -isystem $(OGRE_BUILD_DIR)/RenderSystems/GL/src/GLSL/include \
            -isystem $(OGRE_BUILD_DIR)/RenderSystems/GL/include \
            -isystem $(OGRE_BUILD_DIR)/PlugIns/CgProgramManager/include \
            -isystem $(OGRE_BUILD_DIR)/PlugIns/ParticleFX/include \
            -isystem $(OGRE_BUILD_DIR)/PlugIns/OctreeSceneManager/include  \
            -D_WIN32 \
            -DOGRE_STATIC_LIB \
            -DOGRE_GUI_WIN32 \
            -DOGRE_CONFIG_LITTLE_ENDIAN \
            -DOGRE_DOUBLE_PRECISION=0 \
            -DOGRE_NO_DEVIL=1 \
            -DOGRE_NO_FREEIMAGE=0 \
            $(shell test "$(THREADED)" = "threaded" && echo -DOGRE_THREAD_SUPPORT=1) \
            $(shell test "$(THREADED)" = "semithreaded" && echo -DOGRE_THREAD_SUPPORT=2)

                       #$(DEPENDENCIES_DIR)/lib/libboost_thread-gcc34-mt.dll.a \

OGRE_CMDLINE_LDFLAGS = $(OGRE_BUILD_DIR)/dave_obj/libogre_$(THREADED).a \
                       $(BOOST_DIR)/bin.v2/libs/thread/build/gcc-mingw-3.4.5/release/link-static/runtime-link-static/threading-multi/libboost_thread-mgw34-mt-s-1_35.lib \
                       $(DEPENDENCIES_DIR)/lib/Release/libcg.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libFreeImage.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libfreetype230.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libLibJPEG.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libLibMNG.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libLibPNG.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libLibTIFF.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libzlib.a \
                       $(DEPENDENCIES_DIR)/lib/Release/libzziplib.a \
                       -ld3dx9 -ld3d9 -ldxerr9 \
                       -lopengl32 -lglu32 -lgdi32 \
                       -Wl,--enable-auto-image-base -Wl,--add-stdcall-alias OgreWin32Resources.res

OGRE_LDFLAGS=$(OGRE_CMDLINE_LDFLAGS)



LUA_CFLAGS=-isystem "../lua-5.1.2/src"
LUA_LDFLAGS= #liblua.a comes from ../src/common.mk

DX_CFLAGS=-isystem "$(DXSDK_DIR)Include"
DX_LDFLAGS="$(DXSDK_DIR)Lib\x86\dxguid.lib" "$(DXSDK_DIR)Lib\x86\dinput8.lib"


CFLAGS=-O1 -ffast-math -march=pentium4 -mtune=pentium4 $(OGRE_CFLAGS) $(DX_CFLAGS) $(LUA_CFLAGS) -DNO_PLUGINS
CMDLINE_LDFLAGS=$(OGRE_CMDLINE_LDFLAGS)
LDFLAGS=$(OGRE_LDFLAGS) $(DX_LDFLAGS) $(LUA_LDFLAGS)
CLDFLAGS=$(CFLAGS) $(LDFLAGS)


TARGETS=txdread dffread imgread ifpread matbin ideread iplread grit.exe

all: $(TARGETS)

include ../src/common.mk
