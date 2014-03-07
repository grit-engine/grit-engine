# (c) David Cunningham and the Grit Game Engine project 2012
# Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

# this Makefile is just used to set up some variables, the muscle is in
# ../src/common.mk which is used by both linux and win32 builds

ROOT=../..

include $(ROOT)/common.mk

TARGETS=$(ROOT)/grit_core/media/Grit.$(GRIT_EXEC_SUFFIX) \
        $(ROOT)/exporters/blender_scripts/addons/grit_blender/grit_col_conv.$(GRIT_EXEC_SUFFIX) \

all: $(TARGETS)


depend:
	makedepend -Y -f Makefile ../src/*.cpp ../src/linux/*.cpp ../src/win32/*.cpp ../src/gfx/*.cpp ../src/physics/*.cpp ../src/audio/*.cpp ../src/net/*.cpp -I $(ROOT)/dependencies/util -I ../depend_stubs
	sed -i 's_^[.][.]/src/linux/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/win32/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/gfx/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/physics/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/audio/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/net/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/\([^.]*[.]o:\)_\1_g' Makefile


.PHONY: depend


CXXFLAGS += $(GRIT_BASE_CXXFLAGS) $(OGRE_CXXFLAGS) $(BULLET_CXXFLAGS) $(LUA_CXXFLAGS) $(ICU_CXXFLAGS) $(OPENAL_CXXFLAGS) $(QUEX_CXXFLAGS) $(FREEIMAGE_CXXFLAGS)
LDFLAGS  += $(GRIT_BASE_LDFLAGS) $(OGRE_LDFLAGS) $(BULLET_LDFLAGS) $(LUA_LDFLAGS) $(ICU_LDFLAGS) $(OPENAL_LDFLAGS) $(QUEX_LDFLAGS) $(FREEIMAGE_LDFLAGS)
LDLIBS   += $(GRIT_BASE_LDLIBS) $(OGRE_LDLIBS) $(BULLET_LDLIBS) $(LUA_LDLIBS) $(ICU_LDLIBS) $(OPENAL_LDLIBS) $(QUEX_LDLIBS) $(FREEIMAGE_LDLIBS)
ifeq ($(GRIT_GOOGLE_PERF_TOOLS),true)
    CXXFLAGS += $(PERF_CXXFLAGS)
    LDFLAGS  += $(PERF_LDFLAGS)
    LDLIBS   += $(PERF_LDLIBS)
endif


%.o: $(ROOT)/dependencies/util/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

%.o: ../src/win32/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

%.o: ../src/linux/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

%.o: ../src/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

%.o: ../src/gfx/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

%.o: ../src/audio/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

%.o: ../src/physics/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

%.o: ../src/net/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

#These two can't be compiled with pedantic (quex generated files)
TColLexer-core-engine.o: ../src/physics/TColLexer-core-engine.cpp
TColLexer.o: ../src/physics/TColLexer.cpp
	@$(COMPILING)
	@$(CXX) -c $< -o $@ $(CXXFLAGS)


GRIT_OBJ=\
        AudioDiskResource.o \
        audio.o \
        BackgroundLoader.o \
        BColParser.o \
        BulletDebugDrawer.o \
        CollisionMesh.o \
        colour_conversion.o \
        console.o \
        DenseIndexMap.o \
        DiskResource.o \
        ExternalTable.o \
        GfxBody.o \
        gfx_disk_resource.o \
        GfxFertileNode.o \
        GfxFont.o \
        gfx_hud.o \
        GfxInstances.o \
        GfxLight.o \
        GfxMaterial.o \
        GfxNode.o \
        gfx.o \
        gfx_option.o \
        GfxParticleSystem.o \
        GfxPipeline.o \
        GfxRangedInstances.o \
        GfxSkyBody.o \
        GfxSkyMaterial.o \
        GfxTextBuffer.o \
        GritClass.o \
        grit_lua_util.o \
        GritObject.o \
        input_filter.o \
        KeyboardX11.o \
        ldbglue.o \
        lua_utf8.o \
        lua_util.o \
        lua_wrappers_audio.o \
        lua_wrappers_core.o \
        lua_wrappers_disk_resource.o \
        lua_wrappers_gfx.o \
        lua_wrappers_gpuprog.o \
        lua_wrappers_gritobj.o \
        lua_wrappers_material.o \
        lua_wrappers_net.o \
        lua_wrappers_physics.o \
        lua_wrappers_primitives.o \
        main.o \
        MouseX11.o \
        NetAddress.o \
        NetManager.o \
        NetMessage.o \
        net.o \
        OggVorbisDecoder.o \
        path_util.o \
        PhysicalMaterial.o \
        PhysicsWorld.o \
        posix_sleep.o \
        ray.o \
        Streamer.o \
        TColLexer-core-engine.o \
        TColLexer.o \
        TColParser.o \
        unicode_util.o \
        x11_clipboard.o \



%/Grit.$(GRIT_EXEC_SUFFIX): Grit.$(GRIT_EXEC_SUFFIX)
	strip -s $< -o $@
	
Grit.$(GRIT_EXEC_SUFFIX): $(GRIT_OBJ) $(OGRE_ARCHIVE) $(BULLET_ARCHIVES) $(LUA_ARCHIVES) $(ICU_ARCHIVES)
	@$(LINKING)
	@$(CXX) $(GRIT_OBJ) -o $@ $(LDFLAGS) $(LDLIBS)



COL_CONV_OBJ = grit_col_conv.o \
               BColParser.o \
               TColParser.o \
               TColLexer.o \
               TColLexer-core-engine.o  \

%/grit_col_conv.$(GRIT_EXEC_SUFFIX): grit_col_conv.$(GRIT_EXEC_SUFFIX)
	strip -s $< -o $@

grit_col_conv.$(GRIT_EXEC_SUFFIX): $(COL_CONV_OBJ)
	@$(LINKING)
	@$(CXX) $(COL_CONV_OBJ) -o $@ $(LDFLAGS) $(LDLIBS)



clean:
	rm -fv $(TARGETS) *.o




# DO NOT DELETE

BackgroundLoader.o: ../../dependencies/util/sleep.h
BackgroundLoader.o: ../src/BackgroundLoader.h ../depend_stubs/vector
BackgroundLoader.o: ../src/vect_util.h ../depend_stubs/algorithm
BackgroundLoader.o: ../src/CentralisedLog.h ../depend_stubs/iostream
BackgroundLoader.o: ../depend_stubs/sstream
BackgroundLoader.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
BackgroundLoader.o: ../../dependencies/util/console.h
BackgroundLoader.o: ../../dependencies/util/exception.h
BackgroundLoader.o: ../depend_stubs/cstdlib ../depend_stubs/string
BackgroundLoader.o: ../../dependencies/util/intrinsics.h
BackgroundLoader.o: ../depend_stubs/list
BackgroundLoader.o: ../depend_stubs/boost/thread/thread.hpp
BackgroundLoader.o: ../depend_stubs/boost/thread/condition.hpp
BackgroundLoader.o: ../src/DiskResource.h ../depend_stubs/set
BackgroundLoader.o: ../src/main.h ../depend_stubs/lua.h ../src/Mouse.h
BackgroundLoader.o: ../src/Keyboard.h ../src/BulletDebugDrawer.h
BackgroundLoader.o: ../depend_stubs/OgreFrameListener.h
BackgroundLoader.o: ../depend_stubs/OgreManualObject.h
BackgroundLoader.o: ../depend_stubs/btBulletCollisionCommon.h
BackgroundLoader.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
BackgroundLoader.o: ../src/CentralisedLog.h ../src/SharedPtr.h
BackgroundLoader.o: ../depend_stubs/functional
BackgroundLoader.o: ../depend_stubs/btBulletDynamicsCommon.h
BackgroundLoader.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
BackgroundLoader.o: ../../dependencies/util/math_util.h
BackgroundLoader.o: ../depend_stubs/cmath ../depend_stubs/cfloat
BackgroundLoader.o: ../src/physics/TColParser.h
BackgroundLoader.o: ../src/physics/TColLexer ../depend_stubs/istream
BackgroundLoader.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
BackgroundLoader.o: ../src/physics/TColLexer-token_ids
BackgroundLoader.o: ../depend_stubs/cstdio
BackgroundLoader.o: ../depend_stubs/quex/code_base/Token
BackgroundLoader.o: ../depend_stubs/quex/code_base/definitions
BackgroundLoader.o: ../depend_stubs/quex/code_base/buffer/Buffer
BackgroundLoader.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/QuexMode
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Analyser
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Counter
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Accumulator
BackgroundLoader.o: ../depend_stubs/quex/code_base/temporary_macros_on
BackgroundLoader.o: ../depend_stubs/quex/code_base/temporary_macros_off
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/constructor.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Counter.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Accumulator.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/mode_handling.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/misc.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/buffer_access.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Analyser.i
BackgroundLoader.o: ../src/physics/CollisionMesh.h
BackgroundLoader.o: ../depend_stubs/OgreDataStream.h
BackgroundLoader.o: ../src/DiskResource.h ../src/physics/BColParser.h
BackgroundLoader.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
BackgroundLoader.o: ../src/physics/PhysicalMaterial.h
BackgroundLoader.o: ../src/GritObject.h ../src/SharedPtr.h
BackgroundLoader.o: ../src/Streamer.h ../src/GritObject.h
BackgroundLoader.o: ../src/ExternalTable.h ../src/LuaPtr.h
BackgroundLoader.o: ../../dependencies/util/SplineTable.h
BackgroundLoader.o: ../src/LuaPtr.h ../src/gfx/gfx_disk_resource.h
BackgroundLoader.o: ../depend_stubs/OgreResource.h
BackgroundLoader.o: ../depend_stubs/OgreTexture.h
BackgroundLoader.o: ../depend_stubs/OgreMesh.h
BackgroundLoader.o: ../src/BackgroundLoader.h
BulletDebugDrawer.o: ../depend_stubs/OgreSceneManager.h
BulletDebugDrawer.o: ../depend_stubs/OgreMaterialManager.h
BulletDebugDrawer.o: ../depend_stubs/OgreRoot.h
BulletDebugDrawer.o: ../src/BulletDebugDrawer.h
BulletDebugDrawer.o: ../depend_stubs/OgreFrameListener.h
BulletDebugDrawer.o: ../depend_stubs/OgreManualObject.h
BulletDebugDrawer.o: ../depend_stubs/btBulletCollisionCommon.h
BulletDebugDrawer.o: ../src/CentralisedLog.h ../depend_stubs/iostream
BulletDebugDrawer.o: ../depend_stubs/sstream
BulletDebugDrawer.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
BulletDebugDrawer.o: ../../dependencies/util/console.h
BulletDebugDrawer.o: ../../dependencies/util/exception.h
BulletDebugDrawer.o: ../depend_stubs/cstdlib ../depend_stubs/string
BulletDebugDrawer.o: ../../dependencies/util/intrinsics.h
BulletDebugDrawer.o: ../src/gfx/gfx.h ../src/vect_util.h
BulletDebugDrawer.o: ../depend_stubs/vector ../depend_stubs/algorithm
BulletDebugDrawer.o: ../src/SharedPtr.h ../depend_stubs/functional
BulletDebugDrawer.o: ../../dependencies/util/math_util.h
BulletDebugDrawer.o: ../depend_stubs/cmath ../depend_stubs/cfloat
BulletDebugDrawer.o: ../src/gfx/gfx_disk_resource.h
BulletDebugDrawer.o: ../depend_stubs/OgreResource.h
BulletDebugDrawer.o: ../depend_stubs/OgreTexture.h
BulletDebugDrawer.o: ../depend_stubs/OgreMesh.h
BulletDebugDrawer.o: ../src/CentralisedLog.h ../src/BackgroundLoader.h
BulletDebugDrawer.o: ../src/vect_util.h ../src/gfx/GfxNode.h
BulletDebugDrawer.o: ../depend_stubs/OgreSceneNode.h
BulletDebugDrawer.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
BulletDebugDrawer.o: ../depend_stubs/OgreMeshManager.h
BulletDebugDrawer.o: ../depend_stubs/OgreOctreeSceneManager.h
BulletDebugDrawer.o: ../depend_stubs/OgreVector3.h
BulletDebugDrawer.o: ../depend_stubs/OgreQuaternion.h
BulletDebugDrawer.o: ../depend_stubs/OgreCustomCompositionPass.h
BulletDebugDrawer.o: ../depend_stubs/OgreCompositor.h
BulletDebugDrawer.o: ../depend_stubs/OgreOctreePlugin.h
BulletDebugDrawer.o: ../depend_stubs/OgreCgPlugin.h
BulletDebugDrawer.o: ../depend_stubs/OgreRenderSystem.h
BulletDebugDrawer.o: ../depend_stubs/OgreGLRenderSystem.h
BulletDebugDrawer.o: ../src/gfx/gfx.h ../src/gfx/gfx_option.h
BulletDebugDrawer.o: ../src/gfx/GfxParticleSystem.h
BulletDebugDrawer.o: ../depend_stubs/utility ../depend_stubs/lua.h
BulletDebugDrawer.o: ../depend_stubs/lauxlib.h
BulletDebugDrawer.o: ../depend_stubs/lualib.h
BulletDebugDrawer.o: ../depend_stubs/OgreRenderWindow.h
BulletDebugDrawer.o: ../src/gfx/gfx_internal.h
BulletDebugDrawer.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
BulletDebugDrawer.o: ../depend_stubs/btBulletDynamicsCommon.h
BulletDebugDrawer.o: ../src/physics/TColParser.h
BulletDebugDrawer.o: ../src/physics/TColLexer ../depend_stubs/istream
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
BulletDebugDrawer.o: ../src/physics/TColLexer-token_ids
BulletDebugDrawer.o: ../depend_stubs/cstdio
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/Token
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/definitions
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/buffer/Buffer
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/QuexMode
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Analyser
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Counter
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Accumulator
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/temporary_macros_on
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/temporary_macros_off
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/constructor.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Counter.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/misc.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Analyser.i
BulletDebugDrawer.o: ../src/physics/CollisionMesh.h
BulletDebugDrawer.o: ../depend_stubs/OgreDataStream.h
BulletDebugDrawer.o: ../../dependencies/util/sleep.h
BulletDebugDrawer.o: ../src/DiskResource.h ../depend_stubs/set
BulletDebugDrawer.o: ../src/physics/BColParser.h
BulletDebugDrawer.o: ../depend_stubs/stdint.h
BulletDebugDrawer.o: ../src/physics/LooseEnd.h
BulletDebugDrawer.o: ../src/physics/PhysicalMaterial.h
BulletDebugDrawer.o: ../src/GritObject.h ../src/SharedPtr.h
BulletDebugDrawer.o: ../src/Streamer.h ../src/GritObject.h
BulletDebugDrawer.o: ../src/ExternalTable.h ../src/LuaPtr.h
BulletDebugDrawer.o: ../../dependencies/util/SplineTable.h
BulletDebugDrawer.o: ../src/BackgroundLoader.h ../depend_stubs/list
BulletDebugDrawer.o: ../depend_stubs/boost/thread/thread.hpp
BulletDebugDrawer.o: ../depend_stubs/boost/thread/condition.hpp
BulletDebugDrawer.o: ../src/DiskResource.h ../src/LuaPtr.h
DenseIndexMap.o: ../depend_stubs/sstream ../src/DenseIndexMap.h
DenseIndexMap.o: ../depend_stubs/vector ../src/CentralisedLog.h
DenseIndexMap.o: ../depend_stubs/iostream
DenseIndexMap.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
DenseIndexMap.o: ../../dependencies/util/console.h
DenseIndexMap.o: ../../dependencies/util/exception.h
DenseIndexMap.o: ../depend_stubs/cstdlib ../depend_stubs/string
DenseIndexMap.o: ../../dependencies/util/intrinsics.h
DiskResource.o: ../src/gfx/gfx_disk_resource.h
DiskResource.o: ../depend_stubs/OgreResource.h
DiskResource.o: ../depend_stubs/OgreTexture.h
DiskResource.o: ../depend_stubs/OgreMesh.h
DiskResource.o: ../../dependencies/util/math_util.h
DiskResource.o: ../depend_stubs/string ../depend_stubs/iostream
DiskResource.o: ../depend_stubs/cmath ../depend_stubs/cfloat
DiskResource.o: ../src/CentralisedLog.h ../src/BackgroundLoader.h
DiskResource.o: ../depend_stubs/vector ../src/vect_util.h
DiskResource.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
DiskResource.o: ../depend_stubs/sstream
DiskResource.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
DiskResource.o: ../../dependencies/util/console.h
DiskResource.o: ../../dependencies/util/exception.h
DiskResource.o: ../depend_stubs/cstdlib
DiskResource.o: ../../dependencies/util/intrinsics.h
DiskResource.o: ../src/audio/AudioDiskResource.h
DiskResource.o: ../depend_stubs/AL/al.h
DiskResource.o: ../depend_stubs/OgreResourceGroupManager.h
DiskResource.o: ../src/main.h ../depend_stubs/lua.h ../src/Mouse.h
DiskResource.o: ../src/Keyboard.h ../src/BackgroundLoader.h
DiskResource.o: ../depend_stubs/list
DiskResource.o: ../depend_stubs/boost/thread/thread.hpp
DiskResource.o: ../depend_stubs/boost/thread/condition.hpp
DiskResource.o: ../src/DiskResource.h ../depend_stubs/set
DiskResource.o: ../src/BulletDebugDrawer.h
DiskResource.o: ../depend_stubs/OgreFrameListener.h
DiskResource.o: ../depend_stubs/OgreManualObject.h
DiskResource.o: ../depend_stubs/btBulletCollisionCommon.h
DiskResource.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
DiskResource.o: ../src/SharedPtr.h ../depend_stubs/functional
DiskResource.o: ../depend_stubs/btBulletDynamicsCommon.h
DiskResource.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
DiskResource.o: ../src/physics/TColParser.h ../src/physics/TColLexer
DiskResource.o: ../depend_stubs/istream
DiskResource.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
DiskResource.o: ../src/physics/TColLexer-token_ids
DiskResource.o: ../depend_stubs/cstdio
DiskResource.o: ../depend_stubs/quex/code_base/Token
DiskResource.o: ../depend_stubs/quex/code_base/definitions
DiskResource.o: ../depend_stubs/quex/code_base/buffer/Buffer
DiskResource.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
DiskResource.o: ../depend_stubs/quex/code_base/template/QuexMode
DiskResource.o: ../depend_stubs/quex/code_base/template/Analyser
DiskResource.o: ../depend_stubs/quex/code_base/template/Counter
DiskResource.o: ../depend_stubs/quex/code_base/template/Accumulator
DiskResource.o: ../depend_stubs/quex/code_base/temporary_macros_on
DiskResource.o: ../depend_stubs/quex/code_base/temporary_macros_off
DiskResource.o: ../depend_stubs/quex/code_base/template/constructor.i
DiskResource.o: ../depend_stubs/quex/code_base/template/Counter.i
DiskResource.o: ../depend_stubs/quex/code_base/template/Accumulator.i
DiskResource.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
DiskResource.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
DiskResource.o: ../depend_stubs/quex/code_base/template/mode_handling.i
DiskResource.o: ../depend_stubs/quex/code_base/template/misc.i
DiskResource.o: ../depend_stubs/quex/code_base/template/buffer_access.i
DiskResource.o: ../depend_stubs/quex/code_base/template/Analyser.i
DiskResource.o: ../src/physics/CollisionMesh.h
DiskResource.o: ../depend_stubs/OgreDataStream.h
DiskResource.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
DiskResource.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
DiskResource.o: ../src/physics/LooseEnd.h
DiskResource.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
DiskResource.o: ../src/SharedPtr.h ../src/Streamer.h
DiskResource.o: ../src/GritObject.h ../src/ExternalTable.h
DiskResource.o: ../src/LuaPtr.h ../../dependencies/util/SplineTable.h
DiskResource.o: ../src/LuaPtr.h
ExternalTable.o: ../src/ExternalTable.h ../depend_stubs/map
ExternalTable.o: ../depend_stubs/string ../depend_stubs/lua.h
ExternalTable.o: ../depend_stubs/lauxlib.h
ExternalTable.o: ../../dependencies/util/math_util.h
ExternalTable.o: ../depend_stubs/iostream ../depend_stubs/cmath
ExternalTable.o: ../depend_stubs/cfloat ../src/SharedPtr.h
ExternalTable.o: ../src/LuaPtr.h ../depend_stubs/lualib.h
ExternalTable.o: ../src/CentralisedLog.h ../depend_stubs/sstream
ExternalTable.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ExternalTable.o: ../../dependencies/util/console.h
ExternalTable.o: ../../dependencies/util/exception.h
ExternalTable.o: ../depend_stubs/cstdlib
ExternalTable.o: ../../dependencies/util/intrinsics.h
ExternalTable.o: ../../dependencies/util/SplineTable.h
ExternalTable.o: ../src/grit_lua_util.h
ExternalTable.o: ../depend_stubs/OgreException.h
ExternalTable.o: ../../dependencies/util/lua_util.h
ExternalTable.o: ../depend_stubs/limits ../depend_stubs/vector
ExternalTable.o: ../../dependencies/util/lua_wrappers_common.h
ExternalTable.o: ../src/lua_wrappers_primitives.h
GritClass.o: ../src/GritClass.h ../depend_stubs/string
GritClass.o: ../depend_stubs/map ../depend_stubs/lua.h
GritClass.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
GritClass.o: ../src/ExternalTable.h
GritClass.o: ../../dependencies/util/math_util.h
GritClass.o: ../depend_stubs/iostream ../depend_stubs/cmath
GritClass.o: ../depend_stubs/cfloat ../src/SharedPtr.h ../src/LuaPtr.h
GritClass.o: ../src/CentralisedLog.h ../depend_stubs/sstream
GritClass.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GritClass.o: ../../dependencies/util/console.h
GritClass.o: ../../dependencies/util/exception.h
GritClass.o: ../depend_stubs/cstdlib
GritClass.o: ../../dependencies/util/intrinsics.h
GritClass.o: ../../dependencies/util/SplineTable.h
GritClass.o: ../src/grit_lua_util.h ../depend_stubs/OgreException.h
GritClass.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
GritClass.o: ../depend_stubs/vector
GritClass.o: ../../dependencies/util/lua_wrappers_common.h
GritClass.o: ../src/path_util.h
GritObject.o: ../depend_stubs/cmath ../src/main.h
GritObject.o: ../depend_stubs/lua.h ../src/Mouse.h
GritObject.o: ../depend_stubs/vector ../src/Keyboard.h
GritObject.o: ../depend_stubs/string ../src/BackgroundLoader.h
GritObject.o: ../src/vect_util.h ../depend_stubs/algorithm
GritObject.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GritObject.o: ../depend_stubs/sstream
GritObject.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GritObject.o: ../../dependencies/util/console.h
GritObject.o: ../../dependencies/util/exception.h
GritObject.o: ../depend_stubs/cstdlib
GritObject.o: ../../dependencies/util/intrinsics.h
GritObject.o: ../depend_stubs/list
GritObject.o: ../depend_stubs/boost/thread/thread.hpp
GritObject.o: ../depend_stubs/boost/thread/condition.hpp
GritObject.o: ../src/DiskResource.h ../depend_stubs/set
GritObject.o: ../src/BulletDebugDrawer.h
GritObject.o: ../depend_stubs/OgreFrameListener.h
GritObject.o: ../depend_stubs/OgreManualObject.h
GritObject.o: ../depend_stubs/btBulletCollisionCommon.h
GritObject.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
GritObject.o: ../src/CentralisedLog.h ../src/SharedPtr.h
GritObject.o: ../depend_stubs/functional
GritObject.o: ../depend_stubs/btBulletDynamicsCommon.h
GritObject.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
GritObject.o: ../../dependencies/util/math_util.h
GritObject.o: ../depend_stubs/cfloat ../src/physics/TColParser.h
GritObject.o: ../src/physics/TColLexer ../depend_stubs/istream
GritObject.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
GritObject.o: ../src/physics/TColLexer-token_ids
GritObject.o: ../depend_stubs/cstdio
GritObject.o: ../depend_stubs/quex/code_base/Token
GritObject.o: ../depend_stubs/quex/code_base/definitions
GritObject.o: ../depend_stubs/quex/code_base/buffer/Buffer
GritObject.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
GritObject.o: ../depend_stubs/quex/code_base/template/QuexMode
GritObject.o: ../depend_stubs/quex/code_base/template/Analyser
GritObject.o: ../depend_stubs/quex/code_base/template/Counter
GritObject.o: ../depend_stubs/quex/code_base/template/Accumulator
GritObject.o: ../depend_stubs/quex/code_base/temporary_macros_on
GritObject.o: ../depend_stubs/quex/code_base/temporary_macros_off
GritObject.o: ../depend_stubs/quex/code_base/template/constructor.i
GritObject.o: ../depend_stubs/quex/code_base/template/Counter.i
GritObject.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GritObject.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
GritObject.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
GritObject.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GritObject.o: ../depend_stubs/quex/code_base/template/misc.i
GritObject.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GritObject.o: ../depend_stubs/quex/code_base/template/Analyser.i
GritObject.o: ../src/physics/CollisionMesh.h
GritObject.o: ../depend_stubs/OgreDataStream.h
GritObject.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
GritObject.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
GritObject.o: ../src/physics/LooseEnd.h
GritObject.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
GritObject.o: ../src/SharedPtr.h ../src/Streamer.h ../src/GritObject.h
GritObject.o: ../src/ExternalTable.h ../src/LuaPtr.h
GritObject.o: ../../dependencies/util/SplineTable.h ../src/LuaPtr.h
GritObject.o: ../src/GritClass.h ../src/grit_lua_util.h
GritObject.o: ../depend_stubs/OgreException.h
GritObject.o: ../../dependencies/util/lua_util.h
GritObject.o: ../depend_stubs/limits
GritObject.o: ../../dependencies/util/lua_wrappers_common.h
GritObject.o: ../src/path_util.h ../src/lua_wrappers_gritobj.h
Streamer.o: ../src/Streamer.h ../depend_stubs/map
Streamer.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
Streamer.o: ../depend_stubs/lualib.h ../src/GritObject.h
Streamer.o: ../depend_stubs/vector ../depend_stubs/set
Streamer.o: ../depend_stubs/string ../src/SharedPtr.h
Streamer.o: ../../dependencies/util/math_util.h
Streamer.o: ../depend_stubs/iostream ../depend_stubs/cmath
Streamer.o: ../depend_stubs/cfloat ../src/main.h ../src/Mouse.h
Streamer.o: ../src/Keyboard.h ../src/BackgroundLoader.h
Streamer.o: ../src/vect_util.h ../depend_stubs/algorithm
Streamer.o: ../src/CentralisedLog.h ../depend_stubs/sstream
Streamer.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
Streamer.o: ../../dependencies/util/console.h
Streamer.o: ../../dependencies/util/exception.h
Streamer.o: ../depend_stubs/cstdlib
Streamer.o: ../../dependencies/util/intrinsics.h ../depend_stubs/list
Streamer.o: ../depend_stubs/boost/thread/thread.hpp
Streamer.o: ../depend_stubs/boost/thread/condition.hpp
Streamer.o: ../src/DiskResource.h ../src/BulletDebugDrawer.h
Streamer.o: ../depend_stubs/OgreFrameListener.h
Streamer.o: ../depend_stubs/OgreManualObject.h
Streamer.o: ../depend_stubs/btBulletCollisionCommon.h
Streamer.o: ../src/physics/PhysicsWorld.h ../src/CentralisedLog.h
Streamer.o: ../src/SharedPtr.h ../depend_stubs/functional
Streamer.o: ../depend_stubs/btBulletDynamicsCommon.h
Streamer.o: ../src/physics/TColParser.h ../src/physics/TColLexer
Streamer.o: ../depend_stubs/istream
Streamer.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
Streamer.o: ../src/physics/TColLexer-token_ids ../depend_stubs/cstdio
Streamer.o: ../depend_stubs/quex/code_base/Token
Streamer.o: ../depend_stubs/quex/code_base/definitions
Streamer.o: ../depend_stubs/quex/code_base/buffer/Buffer
Streamer.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
Streamer.o: ../depend_stubs/quex/code_base/template/QuexMode
Streamer.o: ../depend_stubs/quex/code_base/template/Analyser
Streamer.o: ../depend_stubs/quex/code_base/template/Counter
Streamer.o: ../depend_stubs/quex/code_base/template/Accumulator
Streamer.o: ../depend_stubs/quex/code_base/temporary_macros_on
Streamer.o: ../depend_stubs/quex/code_base/temporary_macros_off
Streamer.o: ../depend_stubs/quex/code_base/template/constructor.i
Streamer.o: ../depend_stubs/quex/code_base/template/Counter.i
Streamer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
Streamer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
Streamer.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
Streamer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
Streamer.o: ../depend_stubs/quex/code_base/template/misc.i
Streamer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
Streamer.o: ../depend_stubs/quex/code_base/template/Analyser.i
Streamer.o: ../src/physics/CollisionMesh.h
Streamer.o: ../depend_stubs/OgreDataStream.h
Streamer.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
Streamer.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
Streamer.o: ../src/physics/LooseEnd.h
Streamer.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
Streamer.o: ../src/ExternalTable.h ../src/LuaPtr.h
Streamer.o: ../../dependencies/util/SplineTable.h ../src/LuaPtr.h
Streamer.o: ../src/option.h ../depend_stubs/ostream ../src/GritClass.h
Streamer.o: ../src/grit_lua_util.h ../depend_stubs/OgreException.h
Streamer.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
Streamer.o: ../../dependencies/util/lua_wrappers_common.h
Streamer.o: ../src/path_util.h ../src/CacheFriendlyRangeSpaceSIMD.h
Streamer.o: ../src/SSEAllocator.h
binding_stack.o: ../../dependencies/util/lua_util.h
binding_stack.o: ../depend_stubs/string ../depend_stubs/sstream
binding_stack.o: ../depend_stubs/limits ../depend_stubs/vector
binding_stack.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
binding_stack.o: ../depend_stubs/lualib.h
binding_stack.o: ../../dependencies/util/exception.h
binding_stack.o: ../depend_stubs/cstdlib ../depend_stubs/iostream
binding_stack.o: ../../dependencies/util/intrinsics.h
binding_stack.o: ../../dependencies/util/math_util.h
binding_stack.o: ../depend_stubs/cmath ../depend_stubs/cfloat
binding_stack.o: ../src/LuaPtr.h ../src/CentralisedLog.h
binding_stack.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
binding_stack.o: ../../dependencies/util/console.h
binding_stack.o: ../src/binding_stack.h ../depend_stubs/set
binding_stack.o: ../depend_stubs/map
grit_lua_util.o: ../depend_stubs/cstdlib ../depend_stubs/map
grit_lua_util.o: ../src/LuaPtr.h ../depend_stubs/lua.h
grit_lua_util.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
grit_lua_util.o: ../src/CentralisedLog.h ../depend_stubs/iostream
grit_lua_util.o: ../depend_stubs/sstream
grit_lua_util.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
grit_lua_util.o: ../../dependencies/util/console.h
grit_lua_util.o: ../../dependencies/util/exception.h
grit_lua_util.o: ../depend_stubs/string
grit_lua_util.o: ../../dependencies/util/intrinsics.h
grit_lua_util.o: ../src/grit_lua_util.h
grit_lua_util.o: ../depend_stubs/OgreException.h
grit_lua_util.o: ../../dependencies/util/lua_util.h
grit_lua_util.o: ../depend_stubs/limits ../depend_stubs/vector
grit_lua_util.o: ../../dependencies/util/math_util.h
grit_lua_util.o: ../depend_stubs/cmath ../depend_stubs/cfloat
grit_lua_util.o: ../../dependencies/util/lua_wrappers_common.h
grit_lua_util.o: ../src/path_util.h
input_filter.o: ../../dependencies/util/lua_util.h
input_filter.o: ../depend_stubs/string ../depend_stubs/sstream
input_filter.o: ../depend_stubs/limits ../depend_stubs/vector
input_filter.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
input_filter.o: ../depend_stubs/lualib.h
input_filter.o: ../../dependencies/util/exception.h
input_filter.o: ../depend_stubs/cstdlib ../depend_stubs/iostream
input_filter.o: ../../dependencies/util/intrinsics.h
input_filter.o: ../../dependencies/util/math_util.h
input_filter.o: ../depend_stubs/cmath ../depend_stubs/cfloat
input_filter.o: ../src/LuaPtr.h ../src/CentralisedLog.h
input_filter.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
input_filter.o: ../../dependencies/util/console.h
input_filter.o: ../src/input_filter.h ../depend_stubs/map
input_filter.o: ../src/gfx/gfx_hud.h ../src/SharedPtr.h
input_filter.o: ../depend_stubs/functional ../src/grit_lua_util.h
input_filter.o: ../depend_stubs/OgreException.h
input_filter.o: ../../dependencies/util/lua_wrappers_common.h
input_filter.o: ../src/path_util.h ../src/vect_util.h
input_filter.o: ../depend_stubs/algorithm ../src/LuaPtr.h
input_filter.o: ../src/gfx/GfxPipeline.h ../src/gfx/gfx_internal.h
input_filter.o: ../depend_stubs/Ogre.h
input_filter.o: ../depend_stubs/OgreMeshManager.h
input_filter.o: ../depend_stubs/OgreOctreeSceneManager.h
input_filter.o: ../depend_stubs/OgreVector3.h
input_filter.o: ../depend_stubs/OgreQuaternion.h
input_filter.o: ../depend_stubs/OgreCustomCompositionPass.h
input_filter.o: ../depend_stubs/OgreCompositor.h
input_filter.o: ../depend_stubs/OgreOctreePlugin.h
input_filter.o: ../depend_stubs/OgreCgPlugin.h
input_filter.o: ../depend_stubs/OgreRenderSystem.h
input_filter.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
input_filter.o: ../src/gfx/gfx_option.h ../src/gfx/GfxFont.h
input_filter.o: ../src/BackgroundLoader.h ../src/vect_util.h
input_filter.o: ../src/main.h ../src/Mouse.h ../src/Keyboard.h
input_filter.o: ../src/BackgroundLoader.h ../depend_stubs/list
input_filter.o: ../depend_stubs/boost/thread/thread.hpp
input_filter.o: ../depend_stubs/boost/thread/condition.hpp
input_filter.o: ../src/DiskResource.h ../depend_stubs/set
input_filter.o: ../src/BulletDebugDrawer.h
input_filter.o: ../depend_stubs/OgreFrameListener.h
input_filter.o: ../depend_stubs/OgreManualObject.h
input_filter.o: ../depend_stubs/btBulletCollisionCommon.h
input_filter.o: ../src/physics/PhysicsWorld.h ../src/CentralisedLog.h
input_filter.o: ../depend_stubs/btBulletDynamicsCommon.h
input_filter.o: ../src/physics/TColParser.h ../src/physics/TColLexer
input_filter.o: ../depend_stubs/istream
input_filter.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
input_filter.o: ../src/physics/TColLexer-token_ids
input_filter.o: ../depend_stubs/cstdio
input_filter.o: ../depend_stubs/quex/code_base/Token
input_filter.o: ../depend_stubs/quex/code_base/definitions
input_filter.o: ../depend_stubs/quex/code_base/buffer/Buffer
input_filter.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
input_filter.o: ../depend_stubs/quex/code_base/template/QuexMode
input_filter.o: ../depend_stubs/quex/code_base/template/Analyser
input_filter.o: ../depend_stubs/quex/code_base/template/Counter
input_filter.o: ../depend_stubs/quex/code_base/template/Accumulator
input_filter.o: ../depend_stubs/quex/code_base/temporary_macros_on
input_filter.o: ../depend_stubs/quex/code_base/temporary_macros_off
input_filter.o: ../depend_stubs/quex/code_base/template/constructor.i
input_filter.o: ../depend_stubs/quex/code_base/template/Counter.i
input_filter.o: ../depend_stubs/quex/code_base/template/Accumulator.i
input_filter.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
input_filter.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
input_filter.o: ../depend_stubs/quex/code_base/template/mode_handling.i
input_filter.o: ../depend_stubs/quex/code_base/template/misc.i
input_filter.o: ../depend_stubs/quex/code_base/template/buffer_access.i
input_filter.o: ../depend_stubs/quex/code_base/template/Analyser.i
input_filter.o: ../src/physics/CollisionMesh.h
input_filter.o: ../depend_stubs/OgreDataStream.h
input_filter.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
input_filter.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
input_filter.o: ../src/physics/LooseEnd.h
input_filter.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
input_filter.o: ../src/SharedPtr.h ../src/Streamer.h
input_filter.o: ../src/GritObject.h ../src/ExternalTable.h
input_filter.o: ../../dependencies/util/SplineTable.h
input_filter.o: ../src/gfx/gfx_disk_resource.h
input_filter.o: ../depend_stubs/OgreResource.h
input_filter.o: ../depend_stubs/OgreTexture.h
input_filter.o: ../depend_stubs/OgreMesh.h ../src/gfx/GfxTextBuffer.h
input_filter.o: ../depend_stubs/cstdint
input_filter.o: ../depend_stubs/OgreHardwareBufferManager.h
input_filter.o: ../depend_stubs/OgreHardwareVertexBuffer.h
input_filter.o: ../depend_stubs/OgreHardwareIndexBuffer.h
ldbglue.o: ../depend_stubs/lua.h
lua_wrappers_core.o: ../depend_stubs/limits.h
lua_wrappers_core.o: ../depend_stubs/sys/mman.h
lua_wrappers_core.o: ../../dependencies/util/sleep.h
lua_wrappers_core.o: ../src/grit_lua_util.h ../depend_stubs/string
lua_wrappers_core.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
lua_wrappers_core.o: ../depend_stubs/lualib.h
lua_wrappers_core.o: ../depend_stubs/OgreException.h
lua_wrappers_core.o: ../src/CentralisedLog.h ../depend_stubs/iostream
lua_wrappers_core.o: ../depend_stubs/sstream
lua_wrappers_core.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_core.o: ../../dependencies/util/console.h
lua_wrappers_core.o: ../../dependencies/util/exception.h
lua_wrappers_core.o: ../depend_stubs/cstdlib
lua_wrappers_core.o: ../../dependencies/util/intrinsics.h
lua_wrappers_core.o: ../../dependencies/util/lua_util.h
lua_wrappers_core.o: ../depend_stubs/limits ../depend_stubs/vector
lua_wrappers_core.o: ../../dependencies/util/math_util.h
lua_wrappers_core.o: ../depend_stubs/cmath ../depend_stubs/cfloat
lua_wrappers_core.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_core.o: ../src/Keyboard.h ../src/Mouse.h
lua_wrappers_core.o: ../src/BackgroundLoader.h ../src/vect_util.h
lua_wrappers_core.o: ../depend_stubs/algorithm ../depend_stubs/list
lua_wrappers_core.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_core.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_core.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_core.o: ../src/clipboard.h ../src/main.h
lua_wrappers_core.o: ../src/BulletDebugDrawer.h
lua_wrappers_core.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_core.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_core.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_core.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
lua_wrappers_core.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_core.o: ../depend_stubs/functional
lua_wrappers_core.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_core.o: ../src/physics/TColParser.h
lua_wrappers_core.o: ../src/physics/TColLexer ../depend_stubs/istream
lua_wrappers_core.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_core.o: ../src/physics/TColLexer-token_ids
lua_wrappers_core.o: ../depend_stubs/cstdio
lua_wrappers_core.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_core.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_core.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_core.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_core.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_core.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_core.o: ../src/physics/CollisionMesh.h
lua_wrappers_core.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_core.o: ../src/DiskResource.h ../src/physics/BColParser.h
lua_wrappers_core.o: ../depend_stubs/stdint.h
lua_wrappers_core.o: ../src/physics/LooseEnd.h
lua_wrappers_core.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_core.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_core.o: ../src/Streamer.h ../src/GritObject.h
lua_wrappers_core.o: ../src/ExternalTable.h ../src/LuaPtr.h
lua_wrappers_core.o: ../../dependencies/util/SplineTable.h
lua_wrappers_core.o: ../src/LuaPtr.h
lua_wrappers_core.o: ../../dependencies/util/lua_utf8.h
lua_wrappers_core.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_core.o: ../depend_stubs/OgreResource.h
lua_wrappers_core.o: ../depend_stubs/OgreTexture.h
lua_wrappers_core.o: ../depend_stubs/OgreMesh.h
lua_wrappers_core.o: ../src/BackgroundLoader.h
lua_wrappers_core.o: ../src/lua_wrappers_primitives.h
lua_wrappers_core.o: ../src/path_util.h ../src/lua_wrappers_gritobj.h
lua_wrappers_core.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_core.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_core.o: ../src/grit_lua_util.h ../src/gfx/gfx.h
lua_wrappers_core.o: ../src/vect_util.h ../src/gfx/GfxFertileNode.h
lua_wrappers_core.o: ../src/gfx/GfxNode.h
lua_wrappers_core.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_core.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
lua_wrappers_core.o: ../depend_stubs/OgreMeshManager.h
lua_wrappers_core.o: ../depend_stubs/OgreOctreeSceneManager.h
lua_wrappers_core.o: ../depend_stubs/OgreVector3.h
lua_wrappers_core.o: ../depend_stubs/OgreQuaternion.h
lua_wrappers_core.o: ../depend_stubs/OgreCustomCompositionPass.h
lua_wrappers_core.o: ../depend_stubs/OgreCompositor.h
lua_wrappers_core.o: ../depend_stubs/OgreOctreePlugin.h
lua_wrappers_core.o: ../depend_stubs/OgreCgPlugin.h
lua_wrappers_core.o: ../depend_stubs/OgreRenderSystem.h
lua_wrappers_core.o: ../depend_stubs/OgreGLRenderSystem.h
lua_wrappers_core.o: ../src/gfx/gfx_option.h ../src/gfx/GfxBody.h
lua_wrappers_core.o: ../src/gfx/GfxMaterial.h
lua_wrappers_core.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_core.o: ../src/gfx/GfxSkyBody.h
lua_wrappers_core.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_core.o: ../src/gfx/GfxSkyMaterial.h ../src/gfx/GfxLight.h
lua_wrappers_core.o: ../src/gfx/GfxParticleSystem.h
lua_wrappers_core.o: ../depend_stubs/utility ../src/gfx/GfxInstances.h
lua_wrappers_core.o: ../src/DenseIndexMap.h
lua_wrappers_core.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_core.o: ../src/gfx/GfxRangedInstances.h ../src/Streamer.h
lua_wrappers_core.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_core.o: ../src/SSEAllocator.h ../src/gfx/gfx_hud.h
lua_wrappers_core.o: ../src/path_util.h ../src/gfx/GfxPipeline.h
lua_wrappers_core.o: ../src/gfx/GfxFont.h ../src/main.h
lua_wrappers_core.o: ../src/gfx/GfxTextBuffer.h
lua_wrappers_core.o: ../depend_stubs/cstdint
lua_wrappers_core.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_core.o: ../src/audio/lua_wrappers_audio.h
lua_wrappers_core.o: ../src/audio/audio.h ../depend_stubs/AL/al.h
lua_wrappers_core.o: ../src/audio/AudioDiskResource.h
lua_wrappers_core.o: ../depend_stubs/OgreResourceGroupManager.h
lua_wrappers_core.o: ../src/physics/lua_wrappers_physics.h
lua_wrappers_core.o: ../src/physics/PhysicsWorld.h
lua_wrappers_core.o: ../src/net/lua_wrappers_net.h ../src/net/net.h
lua_wrappers_core.o: ../depend_stubs/sys/socket.h
lua_wrappers_core.o: ../depend_stubs/arpa/inet.h
lua_wrappers_core.o: ../depend_stubs/sys/param.h
lua_wrappers_core.o: ../depend_stubs/sys/types.h
lua_wrappers_core.o: ../depend_stubs/netdb.h
lua_wrappers_core.o: ../depend_stubs/sys/ioctl.h
lua_wrappers_disk_resource.o: ../src/grit_lua_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/string
lua_wrappers_disk_resource.o: ../depend_stubs/lua.h
lua_wrappers_disk_resource.o: ../depend_stubs/lauxlib.h
lua_wrappers_disk_resource.o: ../depend_stubs/lualib.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreException.h
lua_wrappers_disk_resource.o: ../src/CentralisedLog.h
lua_wrappers_disk_resource.o: ../depend_stubs/iostream
lua_wrappers_disk_resource.o: ../depend_stubs/sstream
lua_wrappers_disk_resource.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_disk_resource.o: ../../dependencies/util/console.h
lua_wrappers_disk_resource.o: ../../dependencies/util/exception.h
lua_wrappers_disk_resource.o: ../depend_stubs/cstdlib
lua_wrappers_disk_resource.o: ../../dependencies/util/intrinsics.h
lua_wrappers_disk_resource.o: ../../dependencies/util/lua_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/limits
lua_wrappers_disk_resource.o: ../depend_stubs/vector
lua_wrappers_disk_resource.o: ../../dependencies/util/math_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/cmath
lua_wrappers_disk_resource.o: ../depend_stubs/cfloat
lua_wrappers_disk_resource.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreResource.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreTexture.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMesh.h
lua_wrappers_disk_resource.o: ../src/CentralisedLog.h
lua_wrappers_disk_resource.o: ../src/BackgroundLoader.h
lua_wrappers_disk_resource.o: ../src/vect_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/algorithm
lua_wrappers_disk_resource.o: ../src/physics/CollisionMesh.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_disk_resource.o: ../../dependencies/util/sleep.h
lua_wrappers_disk_resource.o: ../src/DiskResource.h
lua_wrappers_disk_resource.o: ../depend_stubs/set ../src/SharedPtr.h
lua_wrappers_disk_resource.o: ../depend_stubs/functional
lua_wrappers_disk_resource.o: ../src/physics/TColParser.h
lua_wrappers_disk_resource.o: ../src/physics/TColLexer
lua_wrappers_disk_resource.o: ../depend_stubs/map
lua_wrappers_disk_resource.o: ../depend_stubs/istream
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_disk_resource.o: ../src/physics/TColLexer-token_ids
lua_wrappers_disk_resource.o: ../depend_stubs/cstdio
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_disk_resource.o: ../src/physics/BColParser.h
lua_wrappers_disk_resource.o: ../depend_stubs/stdint.h
lua_wrappers_disk_resource.o: ../src/physics/LooseEnd.h
lua_wrappers_disk_resource.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_disk_resource.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_disk_resource.o: ../src/DiskResource.h
lua_wrappers_disk_resource.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_disk_resource.o: ../src/grit_lua_util.h ../src/gfx/gfx.h
lua_wrappers_disk_resource.o: ../src/vect_util.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxFertileNode.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxNode.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_internal.h
lua_wrappers_disk_resource.o: ../depend_stubs/Ogre.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMeshManager.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreOctreeSceneManager.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreVector3.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreQuaternion.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreCustomCompositionPass.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreCompositor.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreOctreePlugin.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreCgPlugin.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreRenderSystem.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreGLRenderSystem.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_option.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxBody.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxMaterial.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxSkyBody.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxSkyMaterial.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxLight.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxParticleSystem.h
lua_wrappers_disk_resource.o: ../depend_stubs/utility
lua_wrappers_disk_resource.o: ../src/gfx/GfxInstances.h
lua_wrappers_disk_resource.o: ../src/DenseIndexMap.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_disk_resource.o: ../src/Streamer.h
lua_wrappers_disk_resource.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_disk_resource.o: ../src/SSEAllocator.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_hud.h ../src/path_util.h
lua_wrappers_disk_resource.o: ../src/LuaPtr.h ../src/gfx/GfxPipeline.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxFont.h ../src/main.h
lua_wrappers_disk_resource.o: ../src/Mouse.h ../src/Keyboard.h
lua_wrappers_disk_resource.o: ../src/BackgroundLoader.h
lua_wrappers_disk_resource.o: ../depend_stubs/list
lua_wrappers_disk_resource.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_disk_resource.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_disk_resource.o: ../src/BulletDebugDrawer.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_disk_resource.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_disk_resource.o: ../src/physics/PhysicsWorld.h
lua_wrappers_disk_resource.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_disk_resource.o: ../src/physics/CollisionMesh.h
lua_wrappers_disk_resource.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_disk_resource.o: ../src/Streamer.h ../src/GritObject.h
lua_wrappers_disk_resource.o: ../src/ExternalTable.h ../src/LuaPtr.h
lua_wrappers_disk_resource.o: ../../dependencies/util/SplineTable.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxTextBuffer.h
lua_wrappers_disk_resource.o: ../depend_stubs/cstdint
lua_wrappers_disk_resource.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_disk_resource.o: ../src/physics/lua_wrappers_physics.h
lua_wrappers_disk_resource.o: ../src/physics/PhysicsWorld.h
lua_wrappers_gritobj.o: ../src/GritObject.h ../depend_stubs/map
lua_wrappers_gritobj.o: ../depend_stubs/vector ../depend_stubs/set
lua_wrappers_gritobj.o: ../depend_stubs/string ../src/SharedPtr.h
lua_wrappers_gritobj.o: ../src/GritClass.h ../depend_stubs/lua.h
lua_wrappers_gritobj.o: ../depend_stubs/lauxlib.h
lua_wrappers_gritobj.o: ../depend_stubs/lualib.h
lua_wrappers_gritobj.o: ../src/ExternalTable.h
lua_wrappers_gritobj.o: ../../dependencies/util/math_util.h
lua_wrappers_gritobj.o: ../depend_stubs/iostream ../depend_stubs/cmath
lua_wrappers_gritobj.o: ../depend_stubs/cfloat ../src/LuaPtr.h
lua_wrappers_gritobj.o: ../src/CentralisedLog.h
lua_wrappers_gritobj.o: ../depend_stubs/sstream
lua_wrappers_gritobj.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_gritobj.o: ../../dependencies/util/console.h
lua_wrappers_gritobj.o: ../../dependencies/util/exception.h
lua_wrappers_gritobj.o: ../depend_stubs/cstdlib
lua_wrappers_gritobj.o: ../../dependencies/util/intrinsics.h
lua_wrappers_gritobj.o: ../../dependencies/util/SplineTable.h
lua_wrappers_gritobj.o: ../src/grit_lua_util.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreException.h
lua_wrappers_gritobj.o: ../../dependencies/util/lua_util.h
lua_wrappers_gritobj.o: ../depend_stubs/limits
lua_wrappers_gritobj.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_gritobj.o: ../src/path_util.h ../src/main.h
lua_wrappers_gritobj.o: ../src/Mouse.h ../src/Keyboard.h
lua_wrappers_gritobj.o: ../src/BackgroundLoader.h ../src/vect_util.h
lua_wrappers_gritobj.o: ../depend_stubs/algorithm ../depend_stubs/list
lua_wrappers_gritobj.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_gritobj.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_gritobj.o: ../src/DiskResource.h
lua_wrappers_gritobj.o: ../src/BulletDebugDrawer.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_gritobj.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_gritobj.o: ../src/physics/PhysicsWorld.h
lua_wrappers_gritobj.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_gritobj.o: ../depend_stubs/functional
lua_wrappers_gritobj.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_gritobj.o: ../src/physics/TColParser.h
lua_wrappers_gritobj.o: ../src/physics/TColLexer
lua_wrappers_gritobj.o: ../depend_stubs/istream
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_gritobj.o: ../src/physics/TColLexer-token_ids
lua_wrappers_gritobj.o: ../depend_stubs/cstdio
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_gritobj.o: ../src/physics/CollisionMesh.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_gritobj.o: ../../dependencies/util/sleep.h
lua_wrappers_gritobj.o: ../src/DiskResource.h
lua_wrappers_gritobj.o: ../src/physics/BColParser.h
lua_wrappers_gritobj.o: ../depend_stubs/stdint.h
lua_wrappers_gritobj.o: ../src/physics/LooseEnd.h
lua_wrappers_gritobj.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_gritobj.o: ../src/GritObject.h ../src/Streamer.h
lua_wrappers_gritobj.o: ../src/LuaPtr.h ../src/lua_wrappers_gritobj.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_primitives.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_core.h
lua_wrappers_primitives.o: ../depend_stubs/algorithm
lua_wrappers_primitives.o: ../src/lua_wrappers_primitives.h
lua_wrappers_primitives.o: ../src/grit_lua_util.h
lua_wrappers_primitives.o: ../depend_stubs/string
lua_wrappers_primitives.o: ../depend_stubs/lua.h
lua_wrappers_primitives.o: ../depend_stubs/lauxlib.h
lua_wrappers_primitives.o: ../depend_stubs/lualib.h
lua_wrappers_primitives.o: ../depend_stubs/OgreException.h
lua_wrappers_primitives.o: ../src/CentralisedLog.h
lua_wrappers_primitives.o: ../depend_stubs/iostream
lua_wrappers_primitives.o: ../depend_stubs/sstream
lua_wrappers_primitives.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_primitives.o: ../../dependencies/util/console.h
lua_wrappers_primitives.o: ../../dependencies/util/exception.h
lua_wrappers_primitives.o: ../depend_stubs/cstdlib
lua_wrappers_primitives.o: ../../dependencies/util/intrinsics.h
lua_wrappers_primitives.o: ../../dependencies/util/lua_util.h
lua_wrappers_primitives.o: ../depend_stubs/limits
lua_wrappers_primitives.o: ../depend_stubs/vector
lua_wrappers_primitives.o: ../../dependencies/util/math_util.h
lua_wrappers_primitives.o: ../depend_stubs/cmath
lua_wrappers_primitives.o: ../depend_stubs/cfloat
lua_wrappers_primitives.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_primitives.o: ../../dependencies/util/SplineTable.h
lua_wrappers_primitives.o: ../depend_stubs/map
main.o: ../depend_stubs/cerrno ../depend_stubs/sstream
main.o: ../src/linux/KeyboardX11.h ../depend_stubs/set
main.o: ../depend_stubs/map ../depend_stubs/X11/Xlib.h
main.o: ../depend_stubs/X11/Xutil.h ../depend_stubs/X11/keysym.h
main.o: ../src/Keyboard.h ../depend_stubs/string
main.o: ../depend_stubs/vector ../src/linux/MouseX11.h ../src/Mouse.h
main.o: ../src/main.h ../depend_stubs/lua.h ../src/Mouse.h
main.o: ../src/Keyboard.h ../src/BackgroundLoader.h ../src/vect_util.h
main.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
main.o: ../depend_stubs/iostream
main.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
main.o: ../../dependencies/util/console.h
main.o: ../../dependencies/util/exception.h ../depend_stubs/cstdlib
main.o: ../../dependencies/util/intrinsics.h ../depend_stubs/list
main.o: ../depend_stubs/boost/thread/thread.hpp
main.o: ../depend_stubs/boost/thread/condition.hpp
main.o: ../src/DiskResource.h ../src/BulletDebugDrawer.h
main.o: ../depend_stubs/OgreFrameListener.h
main.o: ../depend_stubs/OgreManualObject.h
main.o: ../depend_stubs/btBulletCollisionCommon.h
main.o: ../src/physics/PhysicsWorld.h ../src/CentralisedLog.h
main.o: ../src/SharedPtr.h ../depend_stubs/functional
main.o: ../depend_stubs/btBulletDynamicsCommon.h
main.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
main.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
main.o: ../depend_stubs/cfloat ../src/physics/TColParser.h
main.o: ../src/physics/TColLexer ../depend_stubs/istream
main.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
main.o: ../src/physics/TColLexer-token_ids ../depend_stubs/cstdio
main.o: ../depend_stubs/quex/code_base/Token
main.o: ../depend_stubs/quex/code_base/definitions
main.o: ../depend_stubs/quex/code_base/buffer/Buffer
main.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
main.o: ../depend_stubs/quex/code_base/template/QuexMode
main.o: ../depend_stubs/quex/code_base/template/Analyser
main.o: ../depend_stubs/quex/code_base/template/Counter
main.o: ../depend_stubs/quex/code_base/template/Accumulator
main.o: ../depend_stubs/quex/code_base/temporary_macros_on
main.o: ../depend_stubs/quex/code_base/temporary_macros_off
main.o: ../depend_stubs/quex/code_base/template/constructor.i
main.o: ../depend_stubs/quex/code_base/template/Counter.i
main.o: ../depend_stubs/quex/code_base/template/Accumulator.i
main.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
main.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
main.o: ../depend_stubs/quex/code_base/template/mode_handling.i
main.o: ../depend_stubs/quex/code_base/template/misc.i
main.o: ../depend_stubs/quex/code_base/template/buffer_access.i
main.o: ../depend_stubs/quex/code_base/template/Analyser.i
main.o: ../src/physics/CollisionMesh.h
main.o: ../depend_stubs/OgreDataStream.h
main.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
main.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
main.o: ../src/physics/LooseEnd.h ../src/physics/PhysicalMaterial.h
main.o: ../src/GritObject.h ../src/SharedPtr.h ../src/Streamer.h
main.o: ../src/GritObject.h ../src/ExternalTable.h ../src/LuaPtr.h
main.o: ../../dependencies/util/SplineTable.h ../src/LuaPtr.h
main.o: ../src/grit_lua_util.h ../depend_stubs/OgreException.h
main.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
main.o: ../../dependencies/util/lua_wrappers_common.h
main.o: ../src/lua_wrappers_core.h ../src/gfx/gfx.h ../src/vect_util.h
main.o: ../src/gfx/gfx_disk_resource.h ../depend_stubs/OgreResource.h
main.o: ../depend_stubs/OgreTexture.h ../depend_stubs/OgreMesh.h
main.o: ../src/BackgroundLoader.h ../src/gfx/GfxNode.h
main.o: ../depend_stubs/OgreSceneNode.h ../src/gfx/gfx_internal.h
main.o: ../depend_stubs/Ogre.h ../depend_stubs/OgreMeshManager.h
main.o: ../depend_stubs/OgreOctreeSceneManager.h
main.o: ../depend_stubs/OgreVector3.h ../depend_stubs/OgreQuaternion.h
main.o: ../depend_stubs/OgreCustomCompositionPass.h
main.o: ../depend_stubs/OgreCompositor.h
main.o: ../depend_stubs/OgreOctreePlugin.h
main.o: ../depend_stubs/OgreCgPlugin.h
main.o: ../depend_stubs/OgreRenderSystem.h
main.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
main.o: ../src/gfx/gfx_option.h ../src/gfx/GfxParticleSystem.h
main.o: ../depend_stubs/utility ../depend_stubs/OgreRoot.h
main.o: ../depend_stubs/OgreRenderWindow.h ../src/audio/audio.h
main.o: ../depend_stubs/AL/al.h ../src/audio/AudioDiskResource.h
main.o: ../depend_stubs/OgreResourceGroupManager.h ../src/net/net.h
main.o: ../depend_stubs/sys/socket.h ../depend_stubs/arpa/inet.h
main.o: ../depend_stubs/sys/param.h ../depend_stubs/sys/types.h
main.o: ../depend_stubs/netdb.h ../depend_stubs/sys/ioctl.h
path_util.o: ../src/CentralisedLog.h ../depend_stubs/iostream
path_util.o: ../depend_stubs/sstream
path_util.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
path_util.o: ../../dependencies/util/console.h
path_util.o: ../../dependencies/util/exception.h
path_util.o: ../depend_stubs/cstdlib ../depend_stubs/string
path_util.o: ../../dependencies/util/intrinsics.h ../src/path_util.h
path_util.o: ../depend_stubs/vector ../src/grit_lua_util.h
path_util.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
path_util.o: ../depend_stubs/lualib.h ../depend_stubs/OgreException.h
path_util.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
path_util.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
path_util.o: ../depend_stubs/cfloat
path_util.o: ../../dependencies/util/lua_wrappers_common.h
KeyboardX11.o: ../depend_stubs/iostream ../depend_stubs/sstream
KeyboardX11.o: ../../dependencies/util/unicode_util.h
KeyboardX11.o: ../depend_stubs/cstdlib ../depend_stubs/string
KeyboardX11.o: ../src/linux/KeyboardX11.h ../depend_stubs/set
KeyboardX11.o: ../depend_stubs/map ../depend_stubs/X11/Xlib.h
KeyboardX11.o: ../depend_stubs/X11/Xutil.h
KeyboardX11.o: ../depend_stubs/X11/keysym.h ../src/Keyboard.h
KeyboardX11.o: ../depend_stubs/vector ../src/CentralisedLog.h
MouseX11.o: ../depend_stubs/sstream ../src/linux/MouseX11.h
MouseX11.o: ../depend_stubs/X11/Xlib.h ../src/Mouse.h
MouseX11.o: ../depend_stubs/vector ../src/CentralisedLog.h
x11_clipboard.o: ../depend_stubs/cstdlib ../depend_stubs/string
KeyboardDirectInput8.o: ../depend_stubs/assert.h
KeyboardDirectInput8.o: ../depend_stubs/iostream
KeyboardDirectInput8.o: ../src/win32/KeyboardDirectInput8.h
KeyboardDirectInput8.o: ../depend_stubs/map ../depend_stubs/set
KeyboardDirectInput8.o: ../depend_stubs/dinput.h
KeyboardDirectInput8.o: ../src/Keyboard.h ../depend_stubs/string
KeyboardDirectInput8.o: ../depend_stubs/vector
KeyboardDirectInput8.o: ../src/CentralisedLog.h
KeyboardWinAPI.o: ../depend_stubs/assert.h
KeyboardWinAPI.o: ../depend_stubs/iostream
KeyboardWinAPI.o: ../../dependencies/util/unicode_util.h
KeyboardWinAPI.o: ../depend_stubs/cstdlib ../depend_stubs/string
KeyboardWinAPI.o: ../src/win32/KeyboardWinAPI.h
KeyboardWinAPI.o: ../depend_stubs/map ../depend_stubs/set
KeyboardWinAPI.o: ../depend_stubs/windows.h ../src/Keyboard.h
KeyboardWinAPI.o: ../depend_stubs/vector ../src/CentralisedLog.h
MouseDirectInput8.o: ../depend_stubs/assert.h
MouseDirectInput8.o: ../depend_stubs/iostream
MouseDirectInput8.o: ../src/win32/MouseDirectInput8.h
MouseDirectInput8.o: ../depend_stubs/dinput.h ../src/Mouse.h
MouseDirectInput8.o: ../depend_stubs/vector
MouseDirectInput8.o: ../src/CentralisedLog.h
win32_clipboard.o: ../depend_stubs/cstdlib
win32_clipboard.o: ../depend_stubs/string
win32_clipboard.o: ../depend_stubs/windows.h
win32_clipboard.o: ../../dependencies/util/unicode_util.h
Clutter.o: ../depend_stubs/OgreHardwareBufferManager.h
Clutter.o: ../depend_stubs/OgreMaterialManager.h
Clutter.o: ../depend_stubs/OgreSubMesh.h
Clutter.o: ../depend_stubs/OgreMesh.h ../src/main.h
Clutter.o: ../depend_stubs/lua.h ../src/Mouse.h
Clutter.o: ../depend_stubs/vector ../src/Keyboard.h
Clutter.o: ../depend_stubs/string ../src/BackgroundLoader.h
Clutter.o: ../src/vect_util.h ../depend_stubs/algorithm
Clutter.o: ../src/CentralisedLog.h ../depend_stubs/iostream
Clutter.o: ../depend_stubs/sstream
Clutter.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
Clutter.o: ../../dependencies/util/console.h
Clutter.o: ../../dependencies/util/exception.h
Clutter.o: ../depend_stubs/cstdlib
Clutter.o: ../../dependencies/util/intrinsics.h
Clutter.o: ../depend_stubs/list
Clutter.o: ../depend_stubs/boost/thread/thread.hpp
Clutter.o: ../depend_stubs/boost/thread/condition.hpp
Clutter.o: ../src/DiskResource.h ../depend_stubs/set
Clutter.o: ../src/BulletDebugDrawer.h
Clutter.o: ../depend_stubs/OgreFrameListener.h
Clutter.o: ../depend_stubs/OgreManualObject.h
Clutter.o: ../depend_stubs/btBulletCollisionCommon.h
Clutter.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
Clutter.o: ../src/CentralisedLog.h ../src/SharedPtr.h
Clutter.o: ../depend_stubs/functional
Clutter.o: ../depend_stubs/btBulletDynamicsCommon.h
Clutter.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
Clutter.o: ../../dependencies/util/math_util.h
Clutter.o: ../depend_stubs/cmath ../depend_stubs/cfloat
Clutter.o: ../src/physics/TColParser.h ../src/physics/TColLexer
Clutter.o: ../depend_stubs/istream
Clutter.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
Clutter.o: ../src/physics/TColLexer-token_ids
Clutter.o: ../depend_stubs/cstdio
Clutter.o: ../depend_stubs/quex/code_base/Token
Clutter.o: ../depend_stubs/quex/code_base/definitions
Clutter.o: ../depend_stubs/quex/code_base/buffer/Buffer
Clutter.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
Clutter.o: ../depend_stubs/quex/code_base/template/QuexMode
Clutter.o: ../depend_stubs/quex/code_base/template/Analyser
Clutter.o: ../depend_stubs/quex/code_base/template/Counter
Clutter.o: ../depend_stubs/quex/code_base/template/Accumulator
Clutter.o: ../depend_stubs/quex/code_base/temporary_macros_on
Clutter.o: ../depend_stubs/quex/code_base/temporary_macros_off
Clutter.o: ../depend_stubs/quex/code_base/template/constructor.i
Clutter.o: ../depend_stubs/quex/code_base/template/Counter.i
Clutter.o: ../depend_stubs/quex/code_base/template/Accumulator.i
Clutter.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
Clutter.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
Clutter.o: ../depend_stubs/quex/code_base/template/mode_handling.i
Clutter.o: ../depend_stubs/quex/code_base/template/misc.i
Clutter.o: ../depend_stubs/quex/code_base/template/buffer_access.i
Clutter.o: ../depend_stubs/quex/code_base/template/Analyser.i
Clutter.o: ../src/physics/CollisionMesh.h
Clutter.o: ../depend_stubs/OgreDataStream.h
Clutter.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
Clutter.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
Clutter.o: ../src/physics/LooseEnd.h
Clutter.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
Clutter.o: ../src/SharedPtr.h ../src/Streamer.h
Clutter.o: ../src/GritObject.h ../src/ExternalTable.h
Clutter.o: ../src/LuaPtr.h ../../dependencies/util/SplineTable.h
Clutter.o: ../src/LuaPtr.h ../src/gfx/gfx.h ../src/vect_util.h
GfxBody.o: ../depend_stubs/OgreSceneManager.h
GfxBody.o: ../depend_stubs/OgreSkeleton.h
GfxBody.o: ../depend_stubs/OgreAxisAlignedBox.h
GfxBody.o: ../depend_stubs/OgreVector4.h
GfxBody.o: ../depend_stubs/OgreSkeletonInstance.h
GfxBody.o: ../depend_stubs/OgreAnimation.h
GfxBody.o: ../depend_stubs/OgreOptimisedUtil.h
GfxBody.o: ../depend_stubs/OgreSubMesh.h ../src/CentralisedLog.h
GfxBody.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
GfxBody.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxBody.o: ../depend_stubs/OgreMeshManager.h
GfxBody.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxBody.o: ../depend_stubs/OgreVector3.h
GfxBody.o: ../depend_stubs/OgreQuaternion.h
GfxBody.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxBody.o: ../depend_stubs/OgreCompositor.h
GfxBody.o: ../depend_stubs/OgreOctreePlugin.h
GfxBody.o: ../depend_stubs/OgreCgPlugin.h
GfxBody.o: ../depend_stubs/OgreRenderSystem.h
GfxBody.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
GfxBody.o: ../src/vect_util.h ../depend_stubs/vector
GfxBody.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxBody.o: ../depend_stubs/iostream
GfxBody.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxBody.o: ../../dependencies/util/console.h
GfxBody.o: ../../dependencies/util/exception.h
GfxBody.o: ../depend_stubs/cstdlib
GfxBody.o: ../../dependencies/util/intrinsics.h ../src/SharedPtr.h
GfxBody.o: ../depend_stubs/functional ../src/gfx/gfx_option.h
GfxBody.o: ../src/gfx/GfxBody.h ../depend_stubs/OgreMesh.h
GfxBody.o: ../src/gfx/GfxFertileNode.h ../src/gfx/GfxNode.h
GfxBody.o: ../depend_stubs/OgreSceneNode.h
GfxBody.o: ../../dependencies/util/math_util.h
GfxBody.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxBody.o: ../src/gfx/GfxMaterial.h ../depend_stubs/OgreMaterial.h
GfxFertileNode.o: ../src/gfx/GfxFertileNode.h ../src/SharedPtr.h
GfxFertileNode.o: ../depend_stubs/cstdlib
GfxFertileNode.o: ../depend_stubs/functional ../src/gfx/GfxNode.h
GfxFertileNode.o: ../depend_stubs/OgreSceneNode.h
GfxFertileNode.o: ../../dependencies/util/math_util.h
GfxFertileNode.o: ../depend_stubs/string ../depend_stubs/iostream
GfxFertileNode.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxFertileNode.o: ../src/vect_util.h ../depend_stubs/vector
GfxFertileNode.o: ../depend_stubs/algorithm
GfxFertileNode.o: ../src/CentralisedLog.h ../depend_stubs/sstream
GfxFertileNode.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxFertileNode.o: ../../dependencies/util/console.h
GfxFertileNode.o: ../../dependencies/util/exception.h
GfxFertileNode.o: ../../dependencies/util/intrinsics.h
GfxFertileNode.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
GfxFertileNode.o: ../depend_stubs/OgreMeshManager.h
GfxFertileNode.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxFertileNode.o: ../depend_stubs/OgreVector3.h
GfxFertileNode.o: ../depend_stubs/OgreQuaternion.h
GfxFertileNode.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxFertileNode.o: ../depend_stubs/OgreCompositor.h
GfxFertileNode.o: ../depend_stubs/OgreOctreePlugin.h
GfxFertileNode.o: ../depend_stubs/OgreCgPlugin.h
GfxFertileNode.o: ../depend_stubs/OgreRenderSystem.h
GfxFertileNode.o: ../depend_stubs/OgreGLRenderSystem.h
GfxFertileNode.o: ../src/gfx/gfx.h ../src/gfx/gfx_option.h
GfxFont.o: ../src/gfx/GfxFont.h ../depend_stubs/vector
GfxFont.o: ../depend_stubs/map ../../dependencies/util/math_util.h
GfxFont.o: ../depend_stubs/string ../depend_stubs/iostream
GfxFont.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxFont.o: ../src/BackgroundLoader.h ../src/vect_util.h
GfxFont.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxFont.o: ../depend_stubs/sstream
GfxFont.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxFont.o: ../../dependencies/util/console.h
GfxFont.o: ../../dependencies/util/exception.h
GfxFont.o: ../depend_stubs/cstdlib
GfxFont.o: ../../dependencies/util/intrinsics.h ../src/main.h
GfxFont.o: ../depend_stubs/lua.h ../src/Mouse.h ../src/Keyboard.h
GfxFont.o: ../src/BackgroundLoader.h ../depend_stubs/list
GfxFont.o: ../depend_stubs/boost/thread/thread.hpp
GfxFont.o: ../depend_stubs/boost/thread/condition.hpp
GfxFont.o: ../src/DiskResource.h ../depend_stubs/set
GfxFont.o: ../src/BulletDebugDrawer.h
GfxFont.o: ../depend_stubs/OgreFrameListener.h
GfxFont.o: ../depend_stubs/OgreManualObject.h
GfxFont.o: ../depend_stubs/btBulletCollisionCommon.h
GfxFont.o: ../src/physics/PhysicsWorld.h ../src/CentralisedLog.h
GfxFont.o: ../src/SharedPtr.h ../depend_stubs/functional
GfxFont.o: ../depend_stubs/btBulletDynamicsCommon.h
GfxFont.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
GfxFont.o: ../src/physics/TColParser.h ../src/physics/TColLexer
GfxFont.o: ../depend_stubs/istream
GfxFont.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
GfxFont.o: ../src/physics/TColLexer-token_ids
GfxFont.o: ../depend_stubs/cstdio
GfxFont.o: ../depend_stubs/quex/code_base/Token
GfxFont.o: ../depend_stubs/quex/code_base/definitions
GfxFont.o: ../depend_stubs/quex/code_base/buffer/Buffer
GfxFont.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
GfxFont.o: ../depend_stubs/quex/code_base/template/QuexMode
GfxFont.o: ../depend_stubs/quex/code_base/template/Analyser
GfxFont.o: ../depend_stubs/quex/code_base/template/Counter
GfxFont.o: ../depend_stubs/quex/code_base/template/Accumulator
GfxFont.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxFont.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxFont.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxFont.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxFont.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxFont.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
GfxFont.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
GfxFont.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxFont.o: ../depend_stubs/quex/code_base/template/misc.i
GfxFont.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxFont.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxFont.o: ../src/physics/CollisionMesh.h
GfxFont.o: ../depend_stubs/OgreDataStream.h
GfxFont.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
GfxFont.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
GfxFont.o: ../src/physics/LooseEnd.h
GfxFont.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
GfxFont.o: ../src/SharedPtr.h ../src/Streamer.h
GfxFont.o: ../src/GritObject.h ../src/ExternalTable.h
GfxFont.o: ../src/LuaPtr.h ../../dependencies/util/SplineTable.h
GfxFont.o: ../src/LuaPtr.h ../src/gfx/gfx_disk_resource.h
GfxFont.o: ../depend_stubs/OgreResource.h
GfxFont.o: ../depend_stubs/OgreTexture.h
GfxFont.o: ../depend_stubs/OgreMesh.h
GfxInstances.o: ../depend_stubs/algorithm ../src/main.h
GfxInstances.o: ../depend_stubs/lua.h ../src/Mouse.h
GfxInstances.o: ../depend_stubs/vector ../src/Keyboard.h
GfxInstances.o: ../depend_stubs/string ../src/BackgroundLoader.h
GfxInstances.o: ../src/vect_util.h ../src/CentralisedLog.h
GfxInstances.o: ../depend_stubs/iostream ../depend_stubs/sstream
GfxInstances.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxInstances.o: ../../dependencies/util/console.h
GfxInstances.o: ../../dependencies/util/exception.h
GfxInstances.o: ../depend_stubs/cstdlib
GfxInstances.o: ../../dependencies/util/intrinsics.h
GfxInstances.o: ../depend_stubs/list
GfxInstances.o: ../depend_stubs/boost/thread/thread.hpp
GfxInstances.o: ../depend_stubs/boost/thread/condition.hpp
GfxInstances.o: ../src/DiskResource.h ../depend_stubs/set
GfxInstances.o: ../src/BulletDebugDrawer.h
GfxInstances.o: ../depend_stubs/OgreFrameListener.h
GfxInstances.o: ../depend_stubs/OgreManualObject.h
GfxInstances.o: ../depend_stubs/btBulletCollisionCommon.h
GfxInstances.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
GfxInstances.o: ../src/CentralisedLog.h ../src/SharedPtr.h
GfxInstances.o: ../depend_stubs/functional
GfxInstances.o: ../depend_stubs/btBulletDynamicsCommon.h
GfxInstances.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
GfxInstances.o: ../../dependencies/util/math_util.h
GfxInstances.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxInstances.o: ../src/physics/TColParser.h
GfxInstances.o: ../src/physics/TColLexer ../depend_stubs/istream
GfxInstances.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
GfxInstances.o: ../src/physics/TColLexer-token_ids
GfxInstances.o: ../depend_stubs/cstdio
GfxInstances.o: ../depend_stubs/quex/code_base/Token
GfxInstances.o: ../depend_stubs/quex/code_base/definitions
GfxInstances.o: ../depend_stubs/quex/code_base/buffer/Buffer
GfxInstances.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
GfxInstances.o: ../depend_stubs/quex/code_base/template/QuexMode
GfxInstances.o: ../depend_stubs/quex/code_base/template/Analyser
GfxInstances.o: ../depend_stubs/quex/code_base/template/Counter
GfxInstances.o: ../depend_stubs/quex/code_base/template/Accumulator
GfxInstances.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxInstances.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxInstances.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/misc.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxInstances.o: ../src/physics/CollisionMesh.h
GfxInstances.o: ../depend_stubs/OgreDataStream.h
GfxInstances.o: ../../dependencies/util/sleep.h
GfxInstances.o: ../src/DiskResource.h ../src/physics/BColParser.h
GfxInstances.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
GfxInstances.o: ../src/physics/PhysicalMaterial.h
GfxInstances.o: ../src/GritObject.h ../src/SharedPtr.h
GfxInstances.o: ../src/Streamer.h ../src/GritObject.h
GfxInstances.o: ../src/ExternalTable.h ../src/LuaPtr.h
GfxInstances.o: ../../dependencies/util/SplineTable.h
GfxInstances.o: ../src/LuaPtr.h ../src/gfx/gfx_internal.h
GfxInstances.o: ../depend_stubs/Ogre.h
GfxInstances.o: ../depend_stubs/OgreMeshManager.h
GfxInstances.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxInstances.o: ../depend_stubs/OgreVector3.h
GfxInstances.o: ../depend_stubs/OgreQuaternion.h
GfxInstances.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxInstances.o: ../depend_stubs/OgreCompositor.h
GfxInstances.o: ../depend_stubs/OgreOctreePlugin.h
GfxInstances.o: ../depend_stubs/OgreCgPlugin.h
GfxInstances.o: ../depend_stubs/OgreRenderSystem.h
GfxInstances.o: ../depend_stubs/OgreGLRenderSystem.h
GfxInstances.o: ../src/gfx/gfx.h ../src/vect_util.h
GfxInstances.o: ../src/gfx/gfx_option.h ../src/gfx/GfxMaterial.h
GfxInstances.o: ../depend_stubs/OgreMaterial.h
GfxInstances.o: ../src/gfx/GfxInstances.h ../src/DenseIndexMap.h
GfxInstances.o: ../depend_stubs/OgreMovableObject.h
GfxInstances.o: ../src/gfx/gfx_disk_resource.h
GfxInstances.o: ../depend_stubs/OgreResource.h
GfxInstances.o: ../depend_stubs/OgreTexture.h
GfxInstances.o: ../depend_stubs/OgreMesh.h
GfxInstances.o: ../src/BackgroundLoader.h ../src/gfx/GfxNode.h
GfxInstances.o: ../depend_stubs/OgreSceneNode.h
GfxInstances.o: ../src/gfx/GfxFertileNode.h
GfxLight.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
GfxLight.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxLight.o: ../depend_stubs/OgreMeshManager.h
GfxLight.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxLight.o: ../depend_stubs/OgreVector3.h
GfxLight.o: ../depend_stubs/OgreQuaternion.h
GfxLight.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxLight.o: ../depend_stubs/OgreCompositor.h
GfxLight.o: ../depend_stubs/OgreOctreePlugin.h
GfxLight.o: ../depend_stubs/OgreCgPlugin.h
GfxLight.o: ../depend_stubs/OgreRenderSystem.h
GfxLight.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
GfxLight.o: ../src/vect_util.h ../depend_stubs/vector
GfxLight.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxLight.o: ../depend_stubs/iostream
GfxLight.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxLight.o: ../../dependencies/util/console.h
GfxLight.o: ../../dependencies/util/exception.h
GfxLight.o: ../depend_stubs/cstdlib
GfxLight.o: ../../dependencies/util/intrinsics.h
GfxLight.o: ../src/SharedPtr.h ../depend_stubs/functional
GfxLight.o: ../src/gfx/gfx_option.h ../src/gfx/GfxLight.h
GfxLight.o: ../src/gfx/GfxFertileNode.h ../src/gfx/GfxNode.h
GfxLight.o: ../depend_stubs/OgreSceneNode.h
GfxLight.o: ../../dependencies/util/math_util.h
GfxLight.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxLight.o: ../src/gfx/GfxParticleSystem.h ../depend_stubs/utility
GfxMaterial.o: ../depend_stubs/set ../src/gfx/gfx_internal.h
GfxMaterial.o: ../depend_stubs/string ../depend_stubs/sstream
GfxMaterial.o: ../depend_stubs/Ogre.h
GfxMaterial.o: ../depend_stubs/OgreMeshManager.h
GfxMaterial.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxMaterial.o: ../depend_stubs/OgreVector3.h
GfxMaterial.o: ../depend_stubs/OgreQuaternion.h
GfxMaterial.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxMaterial.o: ../depend_stubs/OgreCompositor.h
GfxMaterial.o: ../depend_stubs/OgreOctreePlugin.h
GfxMaterial.o: ../depend_stubs/OgreCgPlugin.h
GfxMaterial.o: ../depend_stubs/OgreRenderSystem.h
GfxMaterial.o: ../depend_stubs/OgreGLRenderSystem.h
GfxMaterial.o: ../src/gfx/gfx.h ../src/vect_util.h
GfxMaterial.o: ../depend_stubs/vector ../depend_stubs/algorithm
GfxMaterial.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxMaterial.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxMaterial.o: ../../dependencies/util/console.h
GfxMaterial.o: ../../dependencies/util/exception.h
GfxMaterial.o: ../depend_stubs/cstdlib
GfxMaterial.o: ../../dependencies/util/intrinsics.h
GfxMaterial.o: ../src/SharedPtr.h ../depend_stubs/functional
GfxMaterial.o: ../src/gfx/gfx_option.h ../src/gfx/GfxMaterial.h
GfxMaterial.o: ../depend_stubs/OgreMaterial.h ../src/gfx/GfxBody.h
GfxMaterial.o: ../depend_stubs/OgreMesh.h
GfxMaterial.o: ../src/gfx/GfxFertileNode.h ../src/gfx/GfxNode.h
GfxMaterial.o: ../depend_stubs/OgreSceneNode.h
GfxMaterial.o: ../../dependencies/util/math_util.h
GfxMaterial.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxNode.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
GfxNode.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxNode.o: ../depend_stubs/OgreMeshManager.h
GfxNode.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxNode.o: ../depend_stubs/OgreVector3.h
GfxNode.o: ../depend_stubs/OgreQuaternion.h
GfxNode.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxNode.o: ../depend_stubs/OgreCompositor.h
GfxNode.o: ../depend_stubs/OgreOctreePlugin.h
GfxNode.o: ../depend_stubs/OgreCgPlugin.h
GfxNode.o: ../depend_stubs/OgreRenderSystem.h
GfxNode.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
GfxNode.o: ../src/vect_util.h ../depend_stubs/vector
GfxNode.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxNode.o: ../depend_stubs/iostream
GfxNode.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxNode.o: ../../dependencies/util/console.h
GfxNode.o: ../../dependencies/util/exception.h
GfxNode.o: ../depend_stubs/cstdlib
GfxNode.o: ../../dependencies/util/intrinsics.h ../src/SharedPtr.h
GfxNode.o: ../depend_stubs/functional ../src/gfx/gfx_option.h
GfxNode.o: ../src/gfx/GfxNode.h ../depend_stubs/OgreSceneNode.h
GfxNode.o: ../../dependencies/util/math_util.h
GfxNode.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxNode.o: ../src/gfx/GfxFertileNode.h ../src/gfx/GfxBody.h
GfxNode.o: ../depend_stubs/OgreMesh.h ../src/gfx/GfxMaterial.h
GfxNode.o: ../depend_stubs/OgreMaterial.h
GfxParticleSystem.o: ../depend_stubs/string
GfxParticleSystem.o: ../depend_stubs/algorithm ../depend_stubs/map
GfxParticleSystem.o: ../../dependencies/util/math_util.h
GfxParticleSystem.o: ../depend_stubs/iostream
GfxParticleSystem.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxParticleSystem.o: ../src/vect_util.h ../depend_stubs/vector
GfxParticleSystem.o: ../src/CentralisedLog.h
GfxParticleSystem.o: ../depend_stubs/sstream
GfxParticleSystem.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxParticleSystem.o: ../../dependencies/util/console.h
GfxParticleSystem.o: ../../dependencies/util/exception.h
GfxParticleSystem.o: ../depend_stubs/cstdlib
GfxParticleSystem.o: ../../dependencies/util/intrinsics.h
GfxParticleSystem.o: ../src/gfx/gfx.h ../src/SharedPtr.h
GfxParticleSystem.o: ../depend_stubs/functional
GfxParticleSystem.o: ../src/gfx/gfx_internal.h
GfxParticleSystem.o: ../depend_stubs/Ogre.h
GfxParticleSystem.o: ../depend_stubs/OgreMeshManager.h
GfxParticleSystem.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxParticleSystem.o: ../depend_stubs/OgreVector3.h
GfxParticleSystem.o: ../depend_stubs/OgreQuaternion.h
GfxParticleSystem.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxParticleSystem.o: ../depend_stubs/OgreCompositor.h
GfxParticleSystem.o: ../depend_stubs/OgreOctreePlugin.h
GfxParticleSystem.o: ../depend_stubs/OgreCgPlugin.h
GfxParticleSystem.o: ../depend_stubs/OgreRenderSystem.h
GfxParticleSystem.o: ../depend_stubs/OgreGLRenderSystem.h
GfxParticleSystem.o: ../src/gfx/gfx_option.h
GfxParticleSystem.o: ../src/gfx/GfxParticleSystem.h
GfxParticleSystem.o: ../depend_stubs/utility
GfxParticleSystem.o: ../src/gfx/GfxPipeline.h
GfxPipeline.o: ../depend_stubs/OgreString.h
GfxPipeline.o: ../../dependencies/util/math_util.h
GfxPipeline.o: ../depend_stubs/string ../depend_stubs/iostream
GfxPipeline.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxPipeline.o: ../../dependencies/util/sleep.h
GfxPipeline.o: ../src/gfx/GfxPipeline.h ../src/SharedPtr.h
GfxPipeline.o: ../depend_stubs/cstdlib ../depend_stubs/functional
GfxPipeline.o: ../src/vect_util.h ../depend_stubs/vector
GfxPipeline.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxPipeline.o: ../depend_stubs/sstream
GfxPipeline.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxPipeline.o: ../../dependencies/util/console.h
GfxPipeline.o: ../../dependencies/util/exception.h
GfxPipeline.o: ../../dependencies/util/intrinsics.h
GfxPipeline.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
GfxPipeline.o: ../depend_stubs/OgreMeshManager.h
GfxPipeline.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxPipeline.o: ../depend_stubs/OgreVector3.h
GfxPipeline.o: ../depend_stubs/OgreQuaternion.h
GfxPipeline.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxPipeline.o: ../depend_stubs/OgreCompositor.h
GfxPipeline.o: ../depend_stubs/OgreOctreePlugin.h
GfxPipeline.o: ../depend_stubs/OgreCgPlugin.h
GfxPipeline.o: ../depend_stubs/OgreRenderSystem.h
GfxPipeline.o: ../depend_stubs/OgreGLRenderSystem.h
GfxPipeline.o: ../src/gfx/gfx.h ../src/gfx/gfx_option.h
GfxPipeline.o: ../src/gfx/GfxParticleSystem.h
GfxPipeline.o: ../depend_stubs/utility ../src/gfx/GfxSkyBody.h
GfxPipeline.o: ../depend_stubs/OgreMesh.h
GfxPipeline.o: ../depend_stubs/OgreSceneNode.h
GfxPipeline.o: ../src/gfx/gfx_disk_resource.h
GfxPipeline.o: ../depend_stubs/OgreResource.h
GfxPipeline.o: ../depend_stubs/OgreTexture.h
GfxPipeline.o: ../src/CentralisedLog.h ../src/BackgroundLoader.h
GfxPipeline.o: ../src/vect_util.h ../src/gfx/GfxSkyMaterial.h
GfxPipeline.o: ../depend_stubs/OgreMaterial.h
GfxPipeline.o: ../src/gfx/GfxMaterial.h
GfxRangedInstances.o: ../depend_stubs/algorithm ../src/main.h
GfxRangedInstances.o: ../depend_stubs/lua.h ../src/Mouse.h
GfxRangedInstances.o: ../depend_stubs/vector ../src/Keyboard.h
GfxRangedInstances.o: ../depend_stubs/string
GfxRangedInstances.o: ../src/BackgroundLoader.h ../src/vect_util.h
GfxRangedInstances.o: ../src/CentralisedLog.h
GfxRangedInstances.o: ../depend_stubs/iostream
GfxRangedInstances.o: ../depend_stubs/sstream
GfxRangedInstances.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxRangedInstances.o: ../../dependencies/util/console.h
GfxRangedInstances.o: ../../dependencies/util/exception.h
GfxRangedInstances.o: ../depend_stubs/cstdlib
GfxRangedInstances.o: ../../dependencies/util/intrinsics.h
GfxRangedInstances.o: ../depend_stubs/list
GfxRangedInstances.o: ../depend_stubs/boost/thread/thread.hpp
GfxRangedInstances.o: ../depend_stubs/boost/thread/condition.hpp
GfxRangedInstances.o: ../src/DiskResource.h ../depend_stubs/set
GfxRangedInstances.o: ../src/BulletDebugDrawer.h
GfxRangedInstances.o: ../depend_stubs/OgreFrameListener.h
GfxRangedInstances.o: ../depend_stubs/OgreManualObject.h
GfxRangedInstances.o: ../depend_stubs/btBulletCollisionCommon.h
GfxRangedInstances.o: ../src/physics/PhysicsWorld.h
GfxRangedInstances.o: ../depend_stubs/map ../src/CentralisedLog.h
GfxRangedInstances.o: ../src/SharedPtr.h
GfxRangedInstances.o: ../depend_stubs/functional
GfxRangedInstances.o: ../depend_stubs/btBulletDynamicsCommon.h
GfxRangedInstances.o: ../depend_stubs/lauxlib.h
GfxRangedInstances.o: ../depend_stubs/lualib.h
GfxRangedInstances.o: ../../dependencies/util/math_util.h
GfxRangedInstances.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxRangedInstances.o: ../src/physics/TColParser.h
GfxRangedInstances.o: ../src/physics/TColLexer
GfxRangedInstances.o: ../depend_stubs/istream
GfxRangedInstances.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
GfxRangedInstances.o: ../src/physics/TColLexer-token_ids
GfxRangedInstances.o: ../depend_stubs/cstdio
GfxRangedInstances.o: ../depend_stubs/quex/code_base/Token
GfxRangedInstances.o: ../depend_stubs/quex/code_base/definitions
GfxRangedInstances.o: ../depend_stubs/quex/code_base/buffer/Buffer
GfxRangedInstances.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/QuexMode
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Analyser
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Counter
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Accumulator
GfxRangedInstances.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxRangedInstances.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/misc.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxRangedInstances.o: ../src/physics/CollisionMesh.h
GfxRangedInstances.o: ../depend_stubs/OgreDataStream.h
GfxRangedInstances.o: ../../dependencies/util/sleep.h
GfxRangedInstances.o: ../src/DiskResource.h
GfxRangedInstances.o: ../src/physics/BColParser.h
GfxRangedInstances.o: ../depend_stubs/stdint.h
GfxRangedInstances.o: ../src/physics/LooseEnd.h
GfxRangedInstances.o: ../src/physics/PhysicalMaterial.h
GfxRangedInstances.o: ../src/GritObject.h ../src/SharedPtr.h
GfxRangedInstances.o: ../src/Streamer.h ../src/GritObject.h
GfxRangedInstances.o: ../src/ExternalTable.h ../src/LuaPtr.h
GfxRangedInstances.o: ../../dependencies/util/SplineTable.h
GfxRangedInstances.o: ../src/LuaPtr.h ../src/gfx/gfx_internal.h
GfxRangedInstances.o: ../depend_stubs/Ogre.h
GfxRangedInstances.o: ../depend_stubs/OgreMeshManager.h
GfxRangedInstances.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxRangedInstances.o: ../depend_stubs/OgreVector3.h
GfxRangedInstances.o: ../depend_stubs/OgreQuaternion.h
GfxRangedInstances.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxRangedInstances.o: ../depend_stubs/OgreCompositor.h
GfxRangedInstances.o: ../depend_stubs/OgreOctreePlugin.h
GfxRangedInstances.o: ../depend_stubs/OgreCgPlugin.h
GfxRangedInstances.o: ../depend_stubs/OgreRenderSystem.h
GfxRangedInstances.o: ../depend_stubs/OgreGLRenderSystem.h
GfxRangedInstances.o: ../src/gfx/gfx.h ../src/vect_util.h
GfxRangedInstances.o: ../src/gfx/gfx_option.h
GfxRangedInstances.o: ../src/gfx/GfxRangedInstances.h
GfxRangedInstances.o: ../src/Streamer.h
GfxRangedInstances.o: ../src/CacheFriendlyRangeSpaceSIMD.h
GfxRangedInstances.o: ../src/SSEAllocator.h
GfxRangedInstances.o: ../src/gfx/GfxInstances.h
GfxRangedInstances.o: ../src/DenseIndexMap.h
GfxRangedInstances.o: ../depend_stubs/OgreMovableObject.h
GfxRangedInstances.o: ../src/gfx/gfx_disk_resource.h
GfxRangedInstances.o: ../depend_stubs/OgreResource.h
GfxRangedInstances.o: ../depend_stubs/OgreTexture.h
GfxRangedInstances.o: ../depend_stubs/OgreMesh.h
GfxRangedInstances.o: ../src/BackgroundLoader.h
GfxRangedInstances.o: ../src/gfx/GfxNode.h
GfxRangedInstances.o: ../depend_stubs/OgreSceneNode.h
GfxRangedInstances.o: ../src/gfx/GfxFertileNode.h
GfxSkyBody.o: ../depend_stubs/OgreMeshManager.h
GfxSkyBody.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
GfxSkyBody.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxSkyBody.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxSkyBody.o: ../depend_stubs/OgreVector3.h
GfxSkyBody.o: ../depend_stubs/OgreQuaternion.h
GfxSkyBody.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxSkyBody.o: ../depend_stubs/OgreCompositor.h
GfxSkyBody.o: ../depend_stubs/OgreOctreePlugin.h
GfxSkyBody.o: ../depend_stubs/OgreCgPlugin.h
GfxSkyBody.o: ../depend_stubs/OgreRenderSystem.h
GfxSkyBody.o: ../depend_stubs/OgreGLRenderSystem.h
GfxSkyBody.o: ../src/gfx/gfx.h ../src/vect_util.h
GfxSkyBody.o: ../depend_stubs/vector ../depend_stubs/algorithm
GfxSkyBody.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxSkyBody.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxSkyBody.o: ../../dependencies/util/console.h
GfxSkyBody.o: ../../dependencies/util/exception.h
GfxSkyBody.o: ../depend_stubs/cstdlib
GfxSkyBody.o: ../../dependencies/util/intrinsics.h
GfxSkyBody.o: ../src/SharedPtr.h ../depend_stubs/functional
GfxSkyBody.o: ../src/gfx/gfx_option.h ../src/gfx/GfxSkyBody.h
GfxSkyBody.o: ../depend_stubs/OgreMesh.h
GfxSkyBody.o: ../depend_stubs/OgreSceneNode.h
GfxSkyBody.o: ../../dependencies/util/math_util.h
GfxSkyBody.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxSkyBody.o: ../src/gfx/gfx_disk_resource.h
GfxSkyBody.o: ../depend_stubs/OgreResource.h
GfxSkyBody.o: ../depend_stubs/OgreTexture.h
GfxSkyBody.o: ../src/CentralisedLog.h ../src/BackgroundLoader.h
GfxSkyBody.o: ../src/vect_util.h ../src/gfx/GfxSkyMaterial.h
GfxSkyBody.o: ../depend_stubs/OgreMaterial.h
GfxSkyBody.o: ../src/gfx/GfxMaterial.h ../src/gfx/GfxPipeline.h
GfxSkyMaterial.o: ../depend_stubs/string ../depend_stubs/map
GfxSkyMaterial.o: ../depend_stubs/OgreCgProgram.h
GfxSkyMaterial.o: ../src/CentralisedLog.h
GfxSkyMaterial.o: ../src/gfx/gfx_internal.h
GfxSkyMaterial.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxSkyMaterial.o: ../depend_stubs/OgreMeshManager.h
GfxSkyMaterial.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxSkyMaterial.o: ../depend_stubs/OgreVector3.h
GfxSkyMaterial.o: ../depend_stubs/OgreQuaternion.h
GfxSkyMaterial.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxSkyMaterial.o: ../depend_stubs/OgreCompositor.h
GfxSkyMaterial.o: ../depend_stubs/OgreOctreePlugin.h
GfxSkyMaterial.o: ../depend_stubs/OgreCgPlugin.h
GfxSkyMaterial.o: ../depend_stubs/OgreRenderSystem.h
GfxSkyMaterial.o: ../depend_stubs/OgreGLRenderSystem.h
GfxSkyMaterial.o: ../src/gfx/gfx.h ../src/vect_util.h
GfxSkyMaterial.o: ../depend_stubs/vector ../depend_stubs/algorithm
GfxSkyMaterial.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxSkyMaterial.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxSkyMaterial.o: ../../dependencies/util/console.h
GfxSkyMaterial.o: ../../dependencies/util/exception.h
GfxSkyMaterial.o: ../depend_stubs/cstdlib
GfxSkyMaterial.o: ../../dependencies/util/intrinsics.h
GfxSkyMaterial.o: ../src/SharedPtr.h ../depend_stubs/functional
GfxSkyMaterial.o: ../src/gfx/gfx_option.h
GfxSkyMaterial.o: ../src/gfx/GfxSkyMaterial.h
GfxSkyMaterial.o: ../depend_stubs/OgreMaterial.h
GfxSkyMaterial.o: ../src/gfx/GfxMaterial.h ../src/gfx/GfxSkyBody.h
GfxSkyMaterial.o: ../depend_stubs/OgreMesh.h
GfxSkyMaterial.o: ../depend_stubs/OgreSceneNode.h
GfxSkyMaterial.o: ../../dependencies/util/math_util.h
GfxSkyMaterial.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxSkyMaterial.o: ../src/gfx/gfx_disk_resource.h
GfxSkyMaterial.o: ../depend_stubs/OgreResource.h
GfxSkyMaterial.o: ../depend_stubs/OgreTexture.h
GfxSkyMaterial.o: ../src/BackgroundLoader.h ../src/vect_util.h
GfxTextBuffer.o: ../../dependencies/util/unicode_util.h
GfxTextBuffer.o: ../depend_stubs/cstdlib ../depend_stubs/string
GfxTextBuffer.o: ../src/gfx/GfxTextBuffer.h
GfxTextBuffer.o: ../depend_stubs/cstdint
GfxTextBuffer.o: ../depend_stubs/OgreHardwareBufferManager.h
GfxTextBuffer.o: ../depend_stubs/OgreHardwareVertexBuffer.h
GfxTextBuffer.o: ../depend_stubs/OgreHardwareIndexBuffer.h
GfxTextBuffer.o: ../../dependencies/util/math_util.h
GfxTextBuffer.o: ../depend_stubs/iostream ../depend_stubs/cmath
GfxTextBuffer.o: ../depend_stubs/cfloat ../src/gfx/GfxFont.h
GfxTextBuffer.o: ../depend_stubs/vector ../depend_stubs/map
GfxTextBuffer.o: ../src/BackgroundLoader.h ../src/vect_util.h
GfxTextBuffer.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxTextBuffer.o: ../depend_stubs/sstream
GfxTextBuffer.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxTextBuffer.o: ../../dependencies/util/console.h
GfxTextBuffer.o: ../../dependencies/util/exception.h
GfxTextBuffer.o: ../../dependencies/util/intrinsics.h
GfxTextBuffer.o: ../src/main.h ../depend_stubs/lua.h
GfxTextBuffer.o: ../src/Mouse.h ../src/Keyboard.h
GfxTextBuffer.o: ../src/BackgroundLoader.h ../depend_stubs/list
GfxTextBuffer.o: ../depend_stubs/boost/thread/thread.hpp
GfxTextBuffer.o: ../depend_stubs/boost/thread/condition.hpp
GfxTextBuffer.o: ../src/DiskResource.h ../depend_stubs/set
GfxTextBuffer.o: ../src/BulletDebugDrawer.h
GfxTextBuffer.o: ../depend_stubs/OgreFrameListener.h
GfxTextBuffer.o: ../depend_stubs/OgreManualObject.h
GfxTextBuffer.o: ../depend_stubs/btBulletCollisionCommon.h
GfxTextBuffer.o: ../src/physics/PhysicsWorld.h
GfxTextBuffer.o: ../src/CentralisedLog.h ../src/SharedPtr.h
GfxTextBuffer.o: ../depend_stubs/functional
GfxTextBuffer.o: ../depend_stubs/btBulletDynamicsCommon.h
GfxTextBuffer.o: ../depend_stubs/lauxlib.h
GfxTextBuffer.o: ../depend_stubs/lualib.h
GfxTextBuffer.o: ../src/physics/TColParser.h
GfxTextBuffer.o: ../src/physics/TColLexer ../depend_stubs/istream
GfxTextBuffer.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
GfxTextBuffer.o: ../src/physics/TColLexer-token_ids
GfxTextBuffer.o: ../depend_stubs/cstdio
GfxTextBuffer.o: ../depend_stubs/quex/code_base/Token
GfxTextBuffer.o: ../depend_stubs/quex/code_base/definitions
GfxTextBuffer.o: ../depend_stubs/quex/code_base/buffer/Buffer
GfxTextBuffer.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/QuexMode
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Analyser
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Counter
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Accumulator
GfxTextBuffer.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxTextBuffer.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/misc.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxTextBuffer.o: ../src/physics/CollisionMesh.h
GfxTextBuffer.o: ../depend_stubs/OgreDataStream.h
GfxTextBuffer.o: ../../dependencies/util/sleep.h
GfxTextBuffer.o: ../src/DiskResource.h ../src/physics/BColParser.h
GfxTextBuffer.o: ../depend_stubs/stdint.h
GfxTextBuffer.o: ../src/physics/LooseEnd.h
GfxTextBuffer.o: ../src/physics/PhysicalMaterial.h
GfxTextBuffer.o: ../src/GritObject.h ../src/SharedPtr.h
GfxTextBuffer.o: ../src/Streamer.h ../src/GritObject.h
GfxTextBuffer.o: ../src/ExternalTable.h ../src/LuaPtr.h
GfxTextBuffer.o: ../../dependencies/util/SplineTable.h
GfxTextBuffer.o: ../src/LuaPtr.h ../src/gfx/gfx_disk_resource.h
GfxTextBuffer.o: ../depend_stubs/OgreResource.h
GfxTextBuffer.o: ../depend_stubs/OgreTexture.h
GfxTextBuffer.o: ../depend_stubs/OgreMesh.h
gfx.o: ../depend_stubs/sstream
gfx.o: ../depend_stubs/OgreAutoParamDataSource.h
gfx.o: ../../dependencies/util/sleep.h ../src/path_util.h
gfx.o: ../depend_stubs/vector ../depend_stubs/string ../src/main.h
gfx.o: ../depend_stubs/lua.h ../src/Mouse.h ../src/Keyboard.h
gfx.o: ../src/BackgroundLoader.h ../src/vect_util.h
gfx.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
gfx.o: ../depend_stubs/iostream
gfx.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx.o: ../../dependencies/util/console.h
gfx.o: ../../dependencies/util/exception.h ../depend_stubs/cstdlib
gfx.o: ../../dependencies/util/intrinsics.h ../depend_stubs/list
gfx.o: ../depend_stubs/boost/thread/thread.hpp
gfx.o: ../depend_stubs/boost/thread/condition.hpp
gfx.o: ../src/DiskResource.h ../depend_stubs/set
gfx.o: ../src/BulletDebugDrawer.h
gfx.o: ../depend_stubs/OgreFrameListener.h
gfx.o: ../depend_stubs/OgreManualObject.h
gfx.o: ../depend_stubs/btBulletCollisionCommon.h
gfx.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
gfx.o: ../src/CentralisedLog.h ../src/SharedPtr.h
gfx.o: ../depend_stubs/functional
gfx.o: ../depend_stubs/btBulletDynamicsCommon.h
gfx.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
gfx.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
gfx.o: ../depend_stubs/cfloat ../src/physics/TColParser.h
gfx.o: ../src/physics/TColLexer ../depend_stubs/istream
gfx.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
gfx.o: ../src/physics/TColLexer-token_ids ../depend_stubs/cstdio
gfx.o: ../depend_stubs/quex/code_base/Token
gfx.o: ../depend_stubs/quex/code_base/definitions
gfx.o: ../depend_stubs/quex/code_base/buffer/Buffer
gfx.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
gfx.o: ../depend_stubs/quex/code_base/template/QuexMode
gfx.o: ../depend_stubs/quex/code_base/template/Analyser
gfx.o: ../depend_stubs/quex/code_base/template/Counter
gfx.o: ../depend_stubs/quex/code_base/template/Accumulator
gfx.o: ../depend_stubs/quex/code_base/temporary_macros_on
gfx.o: ../depend_stubs/quex/code_base/temporary_macros_off
gfx.o: ../depend_stubs/quex/code_base/template/constructor.i
gfx.o: ../depend_stubs/quex/code_base/template/Counter.i
gfx.o: ../depend_stubs/quex/code_base/template/Accumulator.i
gfx.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
gfx.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
gfx.o: ../depend_stubs/quex/code_base/template/mode_handling.i
gfx.o: ../depend_stubs/quex/code_base/template/misc.i
gfx.o: ../depend_stubs/quex/code_base/template/buffer_access.i
gfx.o: ../depend_stubs/quex/code_base/template/Analyser.i
gfx.o: ../src/physics/CollisionMesh.h
gfx.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
gfx.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
gfx.o: ../src/physics/LooseEnd.h ../src/physics/PhysicalMaterial.h
gfx.o: ../src/GritObject.h ../src/SharedPtr.h ../src/Streamer.h
gfx.o: ../src/GritObject.h ../src/ExternalTable.h ../src/LuaPtr.h
gfx.o: ../../dependencies/util/SplineTable.h ../src/LuaPtr.h
gfx.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
gfx.o: ../depend_stubs/OgreMeshManager.h
gfx.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx.o: ../depend_stubs/OgreVector3.h
gfx.o: ../depend_stubs/OgreQuaternion.h
gfx.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx.o: ../depend_stubs/OgreCompositor.h
gfx.o: ../depend_stubs/OgreOctreePlugin.h
gfx.o: ../depend_stubs/OgreCgPlugin.h
gfx.o: ../depend_stubs/OgreRenderSystem.h
gfx.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
gfx.o: ../src/vect_util.h ../src/gfx/gfx_option.h
gfx.o: ../src/gfx/GfxPipeline.h ../src/gfx/Clutter.h
gfx.o: ../depend_stubs/OgreMovableObject.h
gfx.o: ../depend_stubs/OgreVertexIndexData.h
gfx.o: ../depend_stubs/OgreMaterial.h ../depend_stubs/OgreMesh.h
gfx.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../src/SSEAllocator.h
gfx.o: ../src/Streamer.h ../src/gfx/GfxMaterial.h
gfx.o: ../src/gfx/GfxBody.h ../src/gfx/GfxFertileNode.h
gfx.o: ../src/gfx/GfxNode.h ../depend_stubs/OgreSceneNode.h
gfx.o: ../src/gfx/GfxLight.h ../src/gfx/GfxParticleSystem.h
gfx.o: ../depend_stubs/utility ../src/gfx/GfxSkyMaterial.h
gfx.o: ../src/gfx/GfxSkyBody.h ../src/gfx/gfx_disk_resource.h
gfx.o: ../depend_stubs/OgreResource.h
gfx.o: ../depend_stubs/OgreTexture.h ../src/BackgroundLoader.h
gfx.o: ../src/gfx/gfx_hud.h ../src/grit_lua_util.h
gfx.o: ../depend_stubs/OgreException.h
gfx.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
gfx.o: ../../dependencies/util/lua_wrappers_common.h
gfx.o: ../src/gfx/GfxFont.h ../src/gfx/GfxTextBuffer.h
gfx.o: ../depend_stubs/cstdint
gfx.o: ../depend_stubs/OgreHardwareBufferManager.h
gfx.o: ../depend_stubs/OgreHardwareVertexBuffer.h
gfx.o: ../depend_stubs/OgreHardwareIndexBuffer.h
gfx_disk_resource.o: ../depend_stubs/OgreMesh.h
gfx_disk_resource.o: ../depend_stubs/OgreMeshManager.h
gfx_disk_resource.o: ../depend_stubs/OgreTexture.h
gfx_disk_resource.o: ../depend_stubs/OgreTextureManager.h
gfx_disk_resource.o: ../depend_stubs/OgreMeshSerializer.h
gfx_disk_resource.o: ../depend_stubs/OgreMeshFileFormat.h
gfx_disk_resource.o: ../src/gfx/gfx_disk_resource.h
gfx_disk_resource.o: ../depend_stubs/OgreResource.h
gfx_disk_resource.o: ../../dependencies/util/math_util.h
gfx_disk_resource.o: ../depend_stubs/string
gfx_disk_resource.o: ../depend_stubs/iostream
gfx_disk_resource.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_disk_resource.o: ../src/CentralisedLog.h
gfx_disk_resource.o: ../src/BackgroundLoader.h
gfx_disk_resource.o: ../depend_stubs/vector ../src/vect_util.h
gfx_disk_resource.o: ../depend_stubs/algorithm
gfx_disk_resource.o: ../src/CentralisedLog.h
gfx_disk_resource.o: ../depend_stubs/sstream
gfx_disk_resource.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx_disk_resource.o: ../../dependencies/util/console.h
gfx_disk_resource.o: ../../dependencies/util/exception.h
gfx_disk_resource.o: ../depend_stubs/cstdlib
gfx_disk_resource.o: ../../dependencies/util/intrinsics.h
gfx_disk_resource.o: ../src/gfx/gfx_internal.h
gfx_disk_resource.o: ../depend_stubs/Ogre.h
gfx_disk_resource.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_disk_resource.o: ../depend_stubs/OgreVector3.h
gfx_disk_resource.o: ../depend_stubs/OgreQuaternion.h
gfx_disk_resource.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_disk_resource.o: ../depend_stubs/OgreCompositor.h
gfx_disk_resource.o: ../depend_stubs/OgreOctreePlugin.h
gfx_disk_resource.o: ../depend_stubs/OgreCgPlugin.h
gfx_disk_resource.o: ../depend_stubs/OgreRenderSystem.h
gfx_disk_resource.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_disk_resource.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_disk_resource.o: ../src/SharedPtr.h ../depend_stubs/functional
gfx_disk_resource.o: ../src/gfx/gfx_option.h
gfx_disk_resource.o: ../src/gfx/GfxMaterial.h
gfx_disk_resource.o: ../depend_stubs/OgreMaterial.h
gfx_disk_resource.o: ../src/gfx/GfxBody.h
gfx_disk_resource.o: ../src/gfx/GfxFertileNode.h
gfx_disk_resource.o: ../src/gfx/GfxNode.h
gfx_disk_resource.o: ../depend_stubs/OgreSceneNode.h
gfx_disk_resource.o: ../src/gfx/GfxSkyBody.h
gfx_disk_resource.o: ../src/gfx/GfxSkyMaterial.h
gfx_hud.o: ../depend_stubs/OgreHighLevelGpuProgramManager.h
gfx_hud.o: ../depend_stubs/OgreCgProgram.h ../src/CentralisedLog.h
gfx_hud.o: ../src/path_util.h ../depend_stubs/vector
gfx_hud.o: ../depend_stubs/string ../src/gfx/lua_wrappers_gfx.h
gfx_hud.o: ../src/grit_lua_util.h ../depend_stubs/lua.h
gfx_hud.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
gfx_hud.o: ../depend_stubs/OgreException.h ../src/CentralisedLog.h
gfx_hud.o: ../depend_stubs/iostream ../depend_stubs/sstream
gfx_hud.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx_hud.o: ../../dependencies/util/console.h
gfx_hud.o: ../../dependencies/util/exception.h
gfx_hud.o: ../depend_stubs/cstdlib
gfx_hud.o: ../../dependencies/util/intrinsics.h
gfx_hud.o: ../../dependencies/util/lua_util.h
gfx_hud.o: ../depend_stubs/limits
gfx_hud.o: ../../dependencies/util/math_util.h
gfx_hud.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_hud.o: ../../dependencies/util/lua_wrappers_common.h
gfx_hud.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_hud.o: ../depend_stubs/algorithm ../src/SharedPtr.h
gfx_hud.o: ../depend_stubs/functional ../src/gfx/GfxFertileNode.h
gfx_hud.o: ../src/gfx/GfxNode.h ../depend_stubs/OgreSceneNode.h
gfx_hud.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
gfx_hud.o: ../depend_stubs/OgreMeshManager.h
gfx_hud.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_hud.o: ../depend_stubs/OgreVector3.h
gfx_hud.o: ../depend_stubs/OgreQuaternion.h
gfx_hud.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_hud.o: ../depend_stubs/OgreCompositor.h
gfx_hud.o: ../depend_stubs/OgreOctreePlugin.h
gfx_hud.o: ../depend_stubs/OgreCgPlugin.h
gfx_hud.o: ../depend_stubs/OgreRenderSystem.h
gfx_hud.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_hud.o: ../src/gfx/gfx_option.h ../src/gfx/GfxBody.h
gfx_hud.o: ../depend_stubs/OgreMesh.h ../src/gfx/GfxMaterial.h
gfx_hud.o: ../depend_stubs/OgreMaterial.h ../src/gfx/GfxSkyBody.h
gfx_hud.o: ../src/gfx/gfx_disk_resource.h
gfx_hud.o: ../depend_stubs/OgreResource.h
gfx_hud.o: ../depend_stubs/OgreTexture.h ../src/BackgroundLoader.h
gfx_hud.o: ../src/vect_util.h ../src/gfx/GfxSkyMaterial.h
gfx_hud.o: ../src/gfx/GfxLight.h ../src/gfx/GfxParticleSystem.h
gfx_hud.o: ../depend_stubs/utility ../src/gfx/GfxInstances.h
gfx_hud.o: ../src/DenseIndexMap.h
gfx_hud.o: ../depend_stubs/OgreMovableObject.h
gfx_hud.o: ../src/gfx/GfxRangedInstances.h ../src/Streamer.h
gfx_hud.o: ../src/CacheFriendlyRangeSpaceSIMD.h
gfx_hud.o: ../depend_stubs/map ../src/SSEAllocator.h
gfx_hud.o: ../src/gfx/gfx_hud.h ../src/LuaPtr.h
gfx_hud.o: ../src/gfx/GfxPipeline.h ../src/gfx/GfxFont.h
gfx_hud.o: ../src/main.h ../src/Mouse.h ../src/Keyboard.h
gfx_hud.o: ../src/BackgroundLoader.h ../depend_stubs/list
gfx_hud.o: ../depend_stubs/boost/thread/thread.hpp
gfx_hud.o: ../depend_stubs/boost/thread/condition.hpp
gfx_hud.o: ../src/DiskResource.h ../depend_stubs/set
gfx_hud.o: ../src/BulletDebugDrawer.h
gfx_hud.o: ../depend_stubs/OgreFrameListener.h
gfx_hud.o: ../depend_stubs/OgreManualObject.h
gfx_hud.o: ../depend_stubs/btBulletCollisionCommon.h
gfx_hud.o: ../src/physics/PhysicsWorld.h
gfx_hud.o: ../depend_stubs/btBulletDynamicsCommon.h
gfx_hud.o: ../src/physics/TColParser.h ../src/physics/TColLexer
gfx_hud.o: ../depend_stubs/istream
gfx_hud.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
gfx_hud.o: ../src/physics/TColLexer-token_ids
gfx_hud.o: ../depend_stubs/cstdio
gfx_hud.o: ../depend_stubs/quex/code_base/Token
gfx_hud.o: ../depend_stubs/quex/code_base/definitions
gfx_hud.o: ../depend_stubs/quex/code_base/buffer/Buffer
gfx_hud.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
gfx_hud.o: ../depend_stubs/quex/code_base/template/QuexMode
gfx_hud.o: ../depend_stubs/quex/code_base/template/Analyser
gfx_hud.o: ../depend_stubs/quex/code_base/template/Counter
gfx_hud.o: ../depend_stubs/quex/code_base/template/Accumulator
gfx_hud.o: ../depend_stubs/quex/code_base/temporary_macros_on
gfx_hud.o: ../depend_stubs/quex/code_base/temporary_macros_off
gfx_hud.o: ../depend_stubs/quex/code_base/template/constructor.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/Counter.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/Accumulator.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/mode_handling.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/misc.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/buffer_access.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/Analyser.i
gfx_hud.o: ../src/physics/CollisionMesh.h
gfx_hud.o: ../depend_stubs/OgreDataStream.h
gfx_hud.o: ../../dependencies/util/sleep.h ../src/DiskResource.h
gfx_hud.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
gfx_hud.o: ../src/physics/LooseEnd.h
gfx_hud.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
gfx_hud.o: ../src/SharedPtr.h ../src/Streamer.h
gfx_hud.o: ../src/GritObject.h ../src/ExternalTable.h
gfx_hud.o: ../src/LuaPtr.h ../../dependencies/util/SplineTable.h
gfx_hud.o: ../src/gfx/GfxTextBuffer.h ../depend_stubs/cstdint
gfx_hud.o: ../depend_stubs/OgreHardwareBufferManager.h
gfx_hud.o: ../depend_stubs/OgreHardwareVertexBuffer.h
gfx_hud.o: ../depend_stubs/OgreHardwareIndexBuffer.h
gfx_option.o: ../depend_stubs/string ../depend_stubs/map
gfx_option.o: ../src/gfx/gfx_internal.h ../depend_stubs/sstream
gfx_option.o: ../depend_stubs/Ogre.h
gfx_option.o: ../depend_stubs/OgreMeshManager.h
gfx_option.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_option.o: ../depend_stubs/OgreVector3.h
gfx_option.o: ../depend_stubs/OgreQuaternion.h
gfx_option.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_option.o: ../depend_stubs/OgreCompositor.h
gfx_option.o: ../depend_stubs/OgreOctreePlugin.h
gfx_option.o: ../depend_stubs/OgreCgPlugin.h
gfx_option.o: ../depend_stubs/OgreRenderSystem.h
gfx_option.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_option.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_option.o: ../depend_stubs/vector ../depend_stubs/algorithm
gfx_option.o: ../src/CentralisedLog.h ../depend_stubs/iostream
gfx_option.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx_option.o: ../../dependencies/util/console.h
gfx_option.o: ../../dependencies/util/exception.h
gfx_option.o: ../depend_stubs/cstdlib
gfx_option.o: ../../dependencies/util/intrinsics.h
gfx_option.o: ../src/SharedPtr.h ../depend_stubs/functional
gfx_option.o: ../src/gfx/gfx_option.h ../src/gfx/GfxPipeline.h
gfx_option.o: ../src/option.h ../depend_stubs/ostream
gfx_option.o: ../src/CentralisedLog.h
lua_wrappers_gfx.o: ../../dependencies/util/colour_conversion.h
lua_wrappers_gfx.o: ../../dependencies/util/unicode_util.h
lua_wrappers_gfx.o: ../depend_stubs/cstdlib ../depend_stubs/string
lua_wrappers_gfx.o: ../src/grit_lua_util.h ../depend_stubs/lua.h
lua_wrappers_gfx.o: ../depend_stubs/lauxlib.h
lua_wrappers_gfx.o: ../depend_stubs/lualib.h
lua_wrappers_gfx.o: ../depend_stubs/OgreException.h
lua_wrappers_gfx.o: ../src/CentralisedLog.h
lua_wrappers_gfx.o: ../depend_stubs/iostream
lua_wrappers_gfx.o: ../depend_stubs/sstream
lua_wrappers_gfx.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_gfx.o: ../../dependencies/util/console.h
lua_wrappers_gfx.o: ../../dependencies/util/exception.h
lua_wrappers_gfx.o: ../../dependencies/util/intrinsics.h
lua_wrappers_gfx.o: ../../dependencies/util/lua_util.h
lua_wrappers_gfx.o: ../depend_stubs/limits ../depend_stubs/vector
lua_wrappers_gfx.o: ../../dependencies/util/math_util.h
lua_wrappers_gfx.o: ../depend_stubs/cmath ../depend_stubs/cfloat
lua_wrappers_gfx.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_gfx.o: ../src/lua_wrappers_primitives.h
lua_wrappers_gfx.o: ../src/grit_lua_util.h
lua_wrappers_gfx.o: ../../dependencies/util/SplineTable.h
lua_wrappers_gfx.o: ../depend_stubs/map ../src/main.h
lua_wrappers_gfx.o: ../src/Mouse.h ../src/Keyboard.h
lua_wrappers_gfx.o: ../src/BackgroundLoader.h ../src/vect_util.h
lua_wrappers_gfx.o: ../depend_stubs/algorithm ../depend_stubs/list
lua_wrappers_gfx.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_gfx.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_gfx.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_gfx.o: ../src/BulletDebugDrawer.h
lua_wrappers_gfx.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_gfx.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_gfx.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_gfx.o: ../src/physics/PhysicsWorld.h
lua_wrappers_gfx.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_gfx.o: ../depend_stubs/functional
lua_wrappers_gfx.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_gfx.o: ../src/physics/TColParser.h
lua_wrappers_gfx.o: ../src/physics/TColLexer
lua_wrappers_gfx.o: ../depend_stubs/istream
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_gfx.o: ../src/physics/TColLexer-token_ids
lua_wrappers_gfx.o: ../depend_stubs/cstdio
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_gfx.o: ../src/physics/CollisionMesh.h
lua_wrappers_gfx.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_gfx.o: ../../dependencies/util/sleep.h
lua_wrappers_gfx.o: ../src/DiskResource.h
lua_wrappers_gfx.o: ../src/physics/BColParser.h
lua_wrappers_gfx.o: ../depend_stubs/stdint.h
lua_wrappers_gfx.o: ../src/physics/LooseEnd.h
lua_wrappers_gfx.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_gfx.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_gfx.o: ../src/Streamer.h ../src/GritObject.h
lua_wrappers_gfx.o: ../src/ExternalTable.h ../src/LuaPtr.h
lua_wrappers_gfx.o: ../src/LuaPtr.h ../src/ExternalTable.h
lua_wrappers_gfx.o: ../src/path_util.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_gfx.h ../src/gfx/gfx.h
lua_wrappers_gfx.o: ../src/vect_util.h ../src/gfx/GfxFertileNode.h
lua_wrappers_gfx.o: ../src/gfx/GfxNode.h
lua_wrappers_gfx.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_gfx.o: ../src/gfx/gfx_internal.h
lua_wrappers_gfx.o: ../depend_stubs/Ogre.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMeshManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreOctreeSceneManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreVector3.h
lua_wrappers_gfx.o: ../depend_stubs/OgreQuaternion.h
lua_wrappers_gfx.o: ../depend_stubs/OgreCustomCompositionPass.h
lua_wrappers_gfx.o: ../depend_stubs/OgreCompositor.h
lua_wrappers_gfx.o: ../depend_stubs/OgreOctreePlugin.h
lua_wrappers_gfx.o: ../depend_stubs/OgreCgPlugin.h
lua_wrappers_gfx.o: ../depend_stubs/OgreRenderSystem.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGLRenderSystem.h
lua_wrappers_gfx.o: ../src/gfx/gfx_option.h ../src/gfx/GfxBody.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMesh.h
lua_wrappers_gfx.o: ../src/gfx/GfxMaterial.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_gfx.o: ../src/gfx/GfxSkyBody.h
lua_wrappers_gfx.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_gfx.o: ../depend_stubs/OgreResource.h
lua_wrappers_gfx.o: ../depend_stubs/OgreTexture.h
lua_wrappers_gfx.o: ../src/BackgroundLoader.h
lua_wrappers_gfx.o: ../src/gfx/GfxSkyMaterial.h
lua_wrappers_gfx.o: ../src/gfx/GfxLight.h
lua_wrappers_gfx.o: ../src/gfx/GfxParticleSystem.h
lua_wrappers_gfx.o: ../depend_stubs/utility
lua_wrappers_gfx.o: ../src/gfx/GfxInstances.h
lua_wrappers_gfx.o: ../src/DenseIndexMap.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_gfx.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_gfx.o: ../src/Streamer.h
lua_wrappers_gfx.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_gfx.o: ../src/SSEAllocator.h ../src/gfx/gfx_hud.h
lua_wrappers_gfx.o: ../src/gfx/GfxPipeline.h ../src/gfx/GfxFont.h
lua_wrappers_gfx.o: ../src/gfx/GfxTextBuffer.h
lua_wrappers_gfx.o: ../depend_stubs/cstdint
lua_wrappers_gfx.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_gfx.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_gfx.o: ../depend_stubs/OgreHighLevelGpuProgramManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreTextureManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreSkeletonManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGpuProgramManager.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_gfx.o: ../depend_stubs/OgreCommon.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_gpuprog.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGpuProgram.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreVector3.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreVector4.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreCgProgram.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreGLSLProgram.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreGLSLGpuProgram.h
lua_wrappers_gpuprog.o: ../src/gfx/lua_wrappers_gpuprog.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreGpuProgram.h
lua_wrappers_gpuprog.o: ../src/grit_lua_util.h
lua_wrappers_gpuprog.o: ../depend_stubs/string
lua_wrappers_gpuprog.o: ../depend_stubs/lua.h
lua_wrappers_gpuprog.o: ../depend_stubs/lauxlib.h
lua_wrappers_gpuprog.o: ../depend_stubs/lualib.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreException.h
lua_wrappers_gpuprog.o: ../src/CentralisedLog.h
lua_wrappers_gpuprog.o: ../depend_stubs/iostream
lua_wrappers_gpuprog.o: ../depend_stubs/sstream
lua_wrappers_gpuprog.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_gpuprog.o: ../../dependencies/util/console.h
lua_wrappers_gpuprog.o: ../../dependencies/util/exception.h
lua_wrappers_gpuprog.o: ../depend_stubs/cstdlib
lua_wrappers_gpuprog.o: ../../dependencies/util/intrinsics.h
lua_wrappers_gpuprog.o: ../../dependencies/util/lua_util.h
lua_wrappers_gpuprog.o: ../depend_stubs/limits
lua_wrappers_gpuprog.o: ../depend_stubs/vector
lua_wrappers_gpuprog.o: ../../dependencies/util/math_util.h
lua_wrappers_gpuprog.o: ../depend_stubs/cmath
lua_wrappers_gpuprog.o: ../depend_stubs/cfloat
lua_wrappers_gpuprog.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_material.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_material.o: ../depend_stubs/OgreTechnique.h
lua_wrappers_material.o: ../depend_stubs/OgrePass.h
lua_wrappers_material.o: ../depend_stubs/OgreTextureUnitState.h
lua_wrappers_material.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_material.o: ../depend_stubs/OgreCommon.h
lua_wrappers_material.o: ../src/grit_lua_util.h
lua_wrappers_material.o: ../depend_stubs/string
lua_wrappers_material.o: ../depend_stubs/lua.h
lua_wrappers_material.o: ../depend_stubs/lauxlib.h
lua_wrappers_material.o: ../depend_stubs/lualib.h
lua_wrappers_material.o: ../depend_stubs/OgreException.h
lua_wrappers_material.o: ../src/CentralisedLog.h
lua_wrappers_material.o: ../depend_stubs/iostream
lua_wrappers_material.o: ../depend_stubs/sstream
lua_wrappers_material.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_material.o: ../../dependencies/util/console.h
lua_wrappers_material.o: ../../dependencies/util/exception.h
lua_wrappers_material.o: ../depend_stubs/cstdlib
lua_wrappers_material.o: ../../dependencies/util/intrinsics.h
lua_wrappers_material.o: ../../dependencies/util/lua_util.h
lua_wrappers_material.o: ../depend_stubs/limits
lua_wrappers_material.o: ../depend_stubs/vector
lua_wrappers_material.o: ../../dependencies/util/math_util.h
lua_wrappers_material.o: ../depend_stubs/cmath
lua_wrappers_material.o: ../depend_stubs/cfloat
lua_wrappers_material.o: ../../dependencies/util/lua_wrappers_common.h
ray.o: ../depend_stubs/float.h ../src/gfx/ray.h
ray.o: ../depend_stubs/Ogre.h ../depend_stubs/OgreSceneQuery.h
BColParser.o: ../depend_stubs/cmath ../depend_stubs/sstream
BColParser.o: ../src/portable_io.h ../depend_stubs/cstdlib
BColParser.o: ../depend_stubs/cstdio ../depend_stubs/cassert
BColParser.o: ../depend_stubs/cstring ../depend_stubs/string
BColParser.o: ../depend_stubs/iostream ../depend_stubs/fstream
BColParser.o: ../depend_stubs/stdint.h ../src/CentralisedLog.h
BColParser.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
BColParser.o: ../../dependencies/util/console.h
BColParser.o: ../../dependencies/util/exception.h
BColParser.o: ../../dependencies/util/intrinsics.h
BColParser.o: ../src/physics/col_defaults.h
BColParser.o: ../src/physics/BColParser.h
BColParser.o: ../depend_stubs/vector
BColParser.o: ../src/physics/TColParser.h
BColParser.o: ../../dependencies/util/math_util.h
BColParser.o: ../depend_stubs/cfloat ../src/physics/TColLexer
BColParser.o: ../depend_stubs/map ../depend_stubs/istream
BColParser.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
BColParser.o: ../src/physics/TColLexer-token_ids
BColParser.o: ../depend_stubs/quex/code_base/Token
BColParser.o: ../depend_stubs/quex/code_base/definitions
BColParser.o: ../depend_stubs/quex/code_base/buffer/Buffer
BColParser.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
BColParser.o: ../depend_stubs/quex/code_base/template/QuexMode
BColParser.o: ../depend_stubs/quex/code_base/template/Analyser
BColParser.o: ../depend_stubs/quex/code_base/template/Counter
BColParser.o: ../depend_stubs/quex/code_base/template/Accumulator
BColParser.o: ../depend_stubs/quex/code_base/temporary_macros_on
BColParser.o: ../depend_stubs/quex/code_base/temporary_macros_off
BColParser.o: ../depend_stubs/quex/code_base/template/constructor.i
BColParser.o: ../depend_stubs/quex/code_base/template/Counter.i
BColParser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
BColParser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
BColParser.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
BColParser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
BColParser.o: ../depend_stubs/quex/code_base/template/misc.i
BColParser.o: ../depend_stubs/quex/code_base/template/buffer_access.i
BColParser.o: ../depend_stubs/quex/code_base/template/Analyser.i
CollisionMesh.o: ../depend_stubs/iostream
CollisionMesh.o: ../depend_stubs/cstdlib ../depend_stubs/ctime
CollisionMesh.o: ../depend_stubs/OgreException.h
CollisionMesh.o: ../depend_stubs/OgreTimer.h
CollisionMesh.o: ../depend_stubs/OgreResourceGroupManager.h
CollisionMesh.o: ../depend_stubs/LinearMath/btGeometryUtil.h
CollisionMesh.o: ../depend_stubs/btBulletCollisionCommon.h
CollisionMesh.o: ../depend_stubs/BulletCollision/Gimpact/btGImpactShape.h
CollisionMesh.o: ../depend_stubs/BulletCollision/CollisionDispatch/btInternalEdgeUtility.h
CollisionMesh.o: ../src/CentralisedLog.h ../src/path_util.h
CollisionMesh.o: ../depend_stubs/vector ../depend_stubs/string
CollisionMesh.o: ../src/physics/CollisionMesh.h
CollisionMesh.o: ../depend_stubs/OgreDataStream.h
CollisionMesh.o: ../../dependencies/util/sleep.h
CollisionMesh.o: ../src/DiskResource.h ../depend_stubs/set
CollisionMesh.o: ../src/SharedPtr.h ../depend_stubs/functional
CollisionMesh.o: ../src/physics/TColParser.h
CollisionMesh.o: ../../dependencies/util/math_util.h
CollisionMesh.o: ../depend_stubs/cmath ../depend_stubs/cfloat
CollisionMesh.o: ../src/physics/TColLexer ../depend_stubs/map
CollisionMesh.o: ../depend_stubs/istream
CollisionMesh.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
CollisionMesh.o: ../src/physics/TColLexer-token_ids
CollisionMesh.o: ../depend_stubs/cstdio
CollisionMesh.o: ../depend_stubs/quex/code_base/Token
CollisionMesh.o: ../depend_stubs/quex/code_base/definitions
CollisionMesh.o: ../depend_stubs/quex/code_base/buffer/Buffer
CollisionMesh.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
CollisionMesh.o: ../depend_stubs/quex/code_base/template/QuexMode
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Analyser
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Counter
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Accumulator
CollisionMesh.o: ../depend_stubs/quex/code_base/temporary_macros_on
CollisionMesh.o: ../depend_stubs/quex/code_base/temporary_macros_off
CollisionMesh.o: ../depend_stubs/quex/code_base/template/constructor.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Counter.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Accumulator.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/mode_handling.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/misc.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/buffer_access.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Analyser.i
CollisionMesh.o: ../src/physics/BColParser.h
CollisionMesh.o: ../depend_stubs/stdint.h
CollisionMesh.o: ../src/physics/LooseEnd.h
CollisionMesh.o: ../src/physics/PhysicalMaterial.h
PhysicalMaterial.o: ../src/physics/PhysicalMaterial.h
PhysicalMaterial.o: ../depend_stubs/vector
PhysicalMaterial.o: ../depend_stubs/string
PhysicalMaterial.o: ../src/CentralisedLog.h ../src/path_util.h
PhysicsWorld.o: ../depend_stubs/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h
PhysicsWorld.o: ../depend_stubs/BulletCollision/CollisionDispatch/btInternalEdgeUtility.h
PhysicsWorld.o: ../src/GritObject.h ../depend_stubs/map
PhysicsWorld.o: ../depend_stubs/vector ../depend_stubs/set
PhysicsWorld.o: ../depend_stubs/string ../src/SharedPtr.h
PhysicsWorld.o: ../src/Streamer.h ../depend_stubs/lua.h
PhysicsWorld.o: ../depend_stubs/lauxlib.h
PhysicsWorld.o: ../depend_stubs/lualib.h ../src/GritObject.h
PhysicsWorld.o: ../../dependencies/util/math_util.h
PhysicsWorld.o: ../depend_stubs/iostream ../depend_stubs/cmath
PhysicsWorld.o: ../depend_stubs/cfloat ../src/ExternalTable.h
PhysicsWorld.o: ../src/LuaPtr.h ../src/CentralisedLog.h
PhysicsWorld.o: ../depend_stubs/sstream
PhysicsWorld.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
PhysicsWorld.o: ../../dependencies/util/console.h
PhysicsWorld.o: ../../dependencies/util/exception.h
PhysicsWorld.o: ../depend_stubs/cstdlib
PhysicsWorld.o: ../../dependencies/util/intrinsics.h
PhysicsWorld.o: ../../dependencies/util/SplineTable.h
PhysicsWorld.o: ../src/BackgroundLoader.h ../src/vect_util.h
PhysicsWorld.o: ../depend_stubs/algorithm ../depend_stubs/list
PhysicsWorld.o: ../depend_stubs/boost/thread/thread.hpp
PhysicsWorld.o: ../depend_stubs/boost/thread/condition.hpp
PhysicsWorld.o: ../src/DiskResource.h ../src/main.h
PhysicsWorld.o: ../src/Mouse.h ../src/Keyboard.h
PhysicsWorld.o: ../src/BulletDebugDrawer.h
PhysicsWorld.o: ../depend_stubs/OgreFrameListener.h
PhysicsWorld.o: ../depend_stubs/OgreManualObject.h
PhysicsWorld.o: ../depend_stubs/btBulletCollisionCommon.h
PhysicsWorld.o: ../src/physics/PhysicsWorld.h
PhysicsWorld.o: ../src/CentralisedLog.h ../src/SharedPtr.h
PhysicsWorld.o: ../depend_stubs/functional
PhysicsWorld.o: ../depend_stubs/btBulletDynamicsCommon.h
PhysicsWorld.o: ../src/physics/TColParser.h
PhysicsWorld.o: ../src/physics/TColLexer
PhysicsWorld.o: ../depend_stubs/istream
PhysicsWorld.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
PhysicsWorld.o: ../src/physics/TColLexer-token_ids
PhysicsWorld.o: ../depend_stubs/cstdio
PhysicsWorld.o: ../depend_stubs/quex/code_base/Token
PhysicsWorld.o: ../depend_stubs/quex/code_base/definitions
PhysicsWorld.o: ../depend_stubs/quex/code_base/buffer/Buffer
PhysicsWorld.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/QuexMode
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Analyser
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Counter
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Accumulator
PhysicsWorld.o: ../depend_stubs/quex/code_base/temporary_macros_on
PhysicsWorld.o: ../depend_stubs/quex/code_base/temporary_macros_off
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/constructor.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Counter.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Accumulator.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/mode_handling.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/misc.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/buffer_access.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Analyser.i
PhysicsWorld.o: ../src/physics/CollisionMesh.h
PhysicsWorld.o: ../depend_stubs/OgreDataStream.h
PhysicsWorld.o: ../../dependencies/util/sleep.h
PhysicsWorld.o: ../src/DiskResource.h
PhysicsWorld.o: ../src/physics/BColParser.h
PhysicsWorld.o: ../depend_stubs/stdint.h
PhysicsWorld.o: ../src/physics/LooseEnd.h
PhysicsWorld.o: ../src/physics/PhysicalMaterial.h
PhysicsWorld.o: ../src/LuaPtr.h ../src/option.h
PhysicsWorld.o: ../depend_stubs/ostream ../src/grit_lua_util.h
PhysicsWorld.o: ../depend_stubs/OgreException.h
PhysicsWorld.o: ../../dependencies/util/lua_util.h
PhysicsWorld.o: ../depend_stubs/limits
PhysicsWorld.o: ../../dependencies/util/lua_wrappers_common.h
PhysicsWorld.o: ../src/physics/PhysicsWorld.h
PhysicsWorld.o: ../src/physics/lua_wrappers_physics.h
TColLexer-core-engine.o: ../src/physics/TColLexer
TColLexer-core-engine.o: ../depend_stubs/vector
TColLexer-core-engine.o: ../depend_stubs/map
TColLexer-core-engine.o: ../depend_stubs/istream
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
TColLexer-core-engine.o: ../src/physics/TColLexer-token_ids
TColLexer-core-engine.o: ../depend_stubs/cstdio
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/Token
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/definitions
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/buffer/Buffer
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/QuexMode
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Analyser
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Counter
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Accumulator
TColLexer-core-engine.o: ../depend_stubs/cstdlib
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/temporary_macros_on
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/temporary_macros_off
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/constructor.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Counter.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Accumulator.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/mode_handling.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/misc.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/buffer_access.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Analyser.i
TColLexer.o: ../src/physics/TColLexer ../depend_stubs/vector
TColLexer.o: ../depend_stubs/map ../depend_stubs/istream
TColLexer.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
TColLexer.o: ../src/physics/TColLexer-token_ids
TColLexer.o: ../depend_stubs/cstdio
TColLexer.o: ../depend_stubs/quex/code_base/Token
TColLexer.o: ../depend_stubs/quex/code_base/definitions
TColLexer.o: ../depend_stubs/quex/code_base/buffer/Buffer
TColLexer.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
TColLexer.o: ../depend_stubs/quex/code_base/template/QuexMode
TColLexer.o: ../depend_stubs/quex/code_base/template/Analyser
TColLexer.o: ../depend_stubs/quex/code_base/template/Counter
TColLexer.o: ../depend_stubs/quex/code_base/template/Accumulator
TColLexer.o: ../depend_stubs/cstdlib
TColLexer.o: ../depend_stubs/quex/code_base/temporary_macros_on
TColLexer.o: ../depend_stubs/quex/code_base/temporary_macros_off
TColLexer.o: ../depend_stubs/quex/code_base/template/constructor.i
TColLexer.o: ../depend_stubs/quex/code_base/template/Counter.i
TColLexer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
TColLexer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
TColLexer.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
TColLexer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
TColLexer.o: ../depend_stubs/quex/code_base/template/misc.i
TColLexer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
TColLexer.o: ../depend_stubs/quex/code_base/template/Analyser.i
TColParser.o: ../depend_stubs/iostream ../depend_stubs/cstdlib
TColParser.o: ../../dependencies/util/math_util.h
TColParser.o: ../depend_stubs/string ../depend_stubs/cmath
TColParser.o: ../depend_stubs/cfloat ../src/path_util.h
TColParser.o: ../depend_stubs/vector ../src/CentralisedLog.h
TColParser.o: ../src/physics/TColLexer ../depend_stubs/map
TColParser.o: ../depend_stubs/istream
TColParser.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
TColParser.o: ../src/physics/TColLexer-token_ids
TColParser.o: ../depend_stubs/cstdio
TColParser.o: ../depend_stubs/quex/code_base/Token
TColParser.o: ../depend_stubs/quex/code_base/definitions
TColParser.o: ../depend_stubs/quex/code_base/buffer/Buffer
TColParser.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
TColParser.o: ../depend_stubs/quex/code_base/template/QuexMode
TColParser.o: ../depend_stubs/quex/code_base/template/Analyser
TColParser.o: ../depend_stubs/quex/code_base/template/Counter
TColParser.o: ../depend_stubs/quex/code_base/template/Accumulator
TColParser.o: ../depend_stubs/quex/code_base/temporary_macros_on
TColParser.o: ../depend_stubs/quex/code_base/temporary_macros_off
TColParser.o: ../depend_stubs/quex/code_base/template/constructor.i
TColParser.o: ../depend_stubs/quex/code_base/template/Counter.i
TColParser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
TColParser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
TColParser.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
TColParser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
TColParser.o: ../depend_stubs/quex/code_base/template/misc.i
TColParser.o: ../depend_stubs/quex/code_base/template/buffer_access.i
TColParser.o: ../depend_stubs/quex/code_base/template/Analyser.i
TColParser.o: ../src/physics/TColParser.h
TColParser.o: ../src/physics/col_defaults.h
grit_col_conv.o: ../depend_stubs/cstdlib
grit_col_conv.o: ../depend_stubs/cerrno
grit_col_conv.o: ../depend_stubs/cstring
grit_col_conv.o: ../depend_stubs/iostream
grit_col_conv.o: ../depend_stubs/fstream
grit_col_conv.o: ../depend_stubs/sys/types.h
grit_col_conv.o: ../depend_stubs/sys/stat.h
grit_col_conv.o: ../depend_stubs/sys/mman.h
grit_col_conv.o: ../depend_stubs/fcntl.h ../src/portable_io.h
grit_col_conv.o: ../depend_stubs/cstdio
grit_col_conv.o: ../depend_stubs/cassert
grit_col_conv.o: ../depend_stubs/string
grit_col_conv.o: ../depend_stubs/stdint.h
grit_col_conv.o: ../src/CentralisedLog.h
grit_col_conv.o: ../depend_stubs/sstream
grit_col_conv.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
grit_col_conv.o: ../../dependencies/util/console.h
grit_col_conv.o: ../../dependencies/util/exception.h
grit_col_conv.o: ../../dependencies/util/intrinsics.h
grit_col_conv.o: ../src/physics/TColParser.h
grit_col_conv.o: ../../dependencies/util/math_util.h
grit_col_conv.o: ../depend_stubs/cmath ../depend_stubs/cfloat
grit_col_conv.o: ../src/physics/TColLexer
grit_col_conv.o: ../depend_stubs/vector ../depend_stubs/map
grit_col_conv.o: ../depend_stubs/istream
grit_col_conv.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
grit_col_conv.o: ../src/physics/TColLexer-token_ids
grit_col_conv.o: ../depend_stubs/quex/code_base/Token
grit_col_conv.o: ../depend_stubs/quex/code_base/definitions
grit_col_conv.o: ../depend_stubs/quex/code_base/buffer/Buffer
grit_col_conv.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
grit_col_conv.o: ../depend_stubs/quex/code_base/template/QuexMode
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Analyser
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Counter
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Accumulator
grit_col_conv.o: ../depend_stubs/quex/code_base/temporary_macros_on
grit_col_conv.o: ../depend_stubs/quex/code_base/temporary_macros_off
grit_col_conv.o: ../depend_stubs/quex/code_base/template/constructor.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Counter.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Accumulator.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/mode_handling.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/misc.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/buffer_access.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Analyser.i
grit_col_conv.o: ../src/physics/BColParser.h
lua_wrappers_physics.o: ../depend_stubs/limits
lua_wrappers_physics.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_physics.o: ../src/SharedPtr.h
lua_wrappers_physics.o: ../depend_stubs/cstdlib
lua_wrappers_physics.o: ../depend_stubs/functional
lua_wrappers_physics.o: ../src/Streamer.h
lua_wrappers_physics.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_physics.o: ../depend_stubs/map
lua_wrappers_physics.o: ../depend_stubs/vector
lua_wrappers_physics.o: ../depend_stubs/iostream
lua_wrappers_physics.o: ../depend_stubs/algorithm
lua_wrappers_physics.o: ../depend_stubs/cmath
lua_wrappers_physics.o: ../src/SSEAllocator.h
lua_wrappers_physics.o: ../src/CentralisedLog.h
lua_wrappers_physics.o: ../depend_stubs/sstream
lua_wrappers_physics.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_physics.o: ../../dependencies/util/console.h
lua_wrappers_physics.o: ../../dependencies/util/exception.h
lua_wrappers_physics.o: ../depend_stubs/string
lua_wrappers_physics.o: ../../dependencies/util/intrinsics.h
lua_wrappers_physics.o: ../src/gfx/GfxInstances.h
lua_wrappers_physics.o: ../src/DenseIndexMap.h
lua_wrappers_physics.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_physics.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_physics.o: ../depend_stubs/OgreResource.h
lua_wrappers_physics.o: ../depend_stubs/OgreTexture.h
lua_wrappers_physics.o: ../depend_stubs/OgreMesh.h
lua_wrappers_physics.o: ../../dependencies/util/math_util.h
lua_wrappers_physics.o: ../depend_stubs/cfloat
lua_wrappers_physics.o: ../src/CentralisedLog.h
lua_wrappers_physics.o: ../src/BackgroundLoader.h
lua_wrappers_physics.o: ../src/vect_util.h
lua_wrappers_physics.o: ../src/gfx/GfxNode.h
lua_wrappers_physics.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_physics.o: ../src/vect_util.h
lua_wrappers_physics.o: ../src/gfx/gfx_internal.h
lua_wrappers_physics.o: ../depend_stubs/Ogre.h
lua_wrappers_physics.o: ../depend_stubs/OgreMeshManager.h
lua_wrappers_physics.o: ../depend_stubs/OgreOctreeSceneManager.h
lua_wrappers_physics.o: ../depend_stubs/OgreVector3.h
lua_wrappers_physics.o: ../depend_stubs/OgreQuaternion.h
lua_wrappers_physics.o: ../depend_stubs/OgreCustomCompositionPass.h
lua_wrappers_physics.o: ../depend_stubs/OgreCompositor.h
lua_wrappers_physics.o: ../depend_stubs/OgreOctreePlugin.h
lua_wrappers_physics.o: ../depend_stubs/OgreCgPlugin.h
lua_wrappers_physics.o: ../depend_stubs/OgreRenderSystem.h
lua_wrappers_physics.o: ../depend_stubs/OgreGLRenderSystem.h
lua_wrappers_physics.o: ../src/gfx/gfx.h
lua_wrappers_physics.o: ../src/gfx/gfx_option.h
lua_wrappers_physics.o: ../src/gfx/GfxFertileNode.h
lua_wrappers_physics.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_physics.o: ../src/grit_lua_util.h
lua_wrappers_physics.o: ../depend_stubs/lua.h
lua_wrappers_physics.o: ../depend_stubs/lauxlib.h
lua_wrappers_physics.o: ../depend_stubs/lualib.h
lua_wrappers_physics.o: ../depend_stubs/OgreException.h
lua_wrappers_physics.o: ../../dependencies/util/lua_util.h
lua_wrappers_physics.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_physics.o: ../src/Streamer.h ../src/GritObject.h
lua_wrappers_physics.o: ../depend_stubs/set ../src/SharedPtr.h
lua_wrappers_physics.o: ../src/main.h ../src/Mouse.h
lua_wrappers_physics.o: ../src/Keyboard.h
lua_wrappers_physics.o: ../src/BackgroundLoader.h
lua_wrappers_physics.o: ../depend_stubs/list
lua_wrappers_physics.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_physics.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_physics.o: ../src/DiskResource.h
lua_wrappers_physics.o: ../src/BulletDebugDrawer.h
lua_wrappers_physics.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_physics.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_physics.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_physics.o: ../src/physics/PhysicsWorld.h
lua_wrappers_physics.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_physics.o: ../src/physics/TColParser.h
lua_wrappers_physics.o: ../src/physics/TColLexer
lua_wrappers_physics.o: ../depend_stubs/istream
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_physics.o: ../src/physics/TColLexer-token_ids
lua_wrappers_physics.o: ../depend_stubs/cstdio
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_physics.o: ../src/physics/CollisionMesh.h
lua_wrappers_physics.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_physics.o: ../../dependencies/util/sleep.h
lua_wrappers_physics.o: ../src/DiskResource.h
lua_wrappers_physics.o: ../src/physics/BColParser.h
lua_wrappers_physics.o: ../depend_stubs/stdint.h
lua_wrappers_physics.o: ../src/physics/LooseEnd.h
lua_wrappers_physics.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_physics.o: ../src/GritObject.h
lua_wrappers_physics.o: ../src/ExternalTable.h ../src/LuaPtr.h
lua_wrappers_physics.o: ../../dependencies/util/SplineTable.h
lua_wrappers_physics.o: ../src/LuaPtr.h ../src/path_util.h
lua_wrappers_physics.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_physics.o: ../src/gfx/gfx.h
lua_wrappers_physics.o: ../src/gfx/GfxParticleSystem.h
lua_wrappers_physics.o: ../depend_stubs/utility
lua_wrappers_physics.o: ../depend_stubs/OgreRoot.h
lua_wrappers_physics.o: ../depend_stubs/OgreRenderWindow.h
lua_wrappers_physics.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_physics.o: ../src/grit_lua_util.h
lua_wrappers_physics.o: ../src/gfx/GfxBody.h
lua_wrappers_physics.o: ../src/gfx/GfxMaterial.h
lua_wrappers_physics.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_physics.o: ../src/gfx/GfxSkyBody.h
lua_wrappers_physics.o: ../src/gfx/GfxSkyMaterial.h
lua_wrappers_physics.o: ../src/gfx/GfxLight.h
lua_wrappers_physics.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_physics.o: ../src/gfx/gfx_hud.h
lua_wrappers_physics.o: ../src/gfx/GfxPipeline.h
lua_wrappers_physics.o: ../src/gfx/GfxFont.h
lua_wrappers_physics.o: ../src/gfx/GfxTextBuffer.h
lua_wrappers_physics.o: ../depend_stubs/cstdint
lua_wrappers_physics.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_physics.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_physics.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_physics.o: ../src/physics/PhysicsWorld.h
lua_wrappers_physics.o: ../src/physics/lua_wrappers_physics.h
AudioDiskResource.o: ../src/audio/AudioDiskResource.h
AudioDiskResource.o: ../depend_stubs/AL/al.h
AudioDiskResource.o: ../src/CentralisedLog.h
AudioDiskResource.o: ../src/BackgroundLoader.h
AudioDiskResource.o: ../depend_stubs/vector ../src/vect_util.h
AudioDiskResource.o: ../depend_stubs/algorithm
AudioDiskResource.o: ../src/CentralisedLog.h
AudioDiskResource.o: ../depend_stubs/iostream
AudioDiskResource.o: ../depend_stubs/sstream
AudioDiskResource.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
AudioDiskResource.o: ../../dependencies/util/console.h
AudioDiskResource.o: ../../dependencies/util/exception.h
AudioDiskResource.o: ../depend_stubs/cstdlib
AudioDiskResource.o: ../depend_stubs/string
AudioDiskResource.o: ../../dependencies/util/intrinsics.h
AudioDiskResource.o: ../depend_stubs/OgreResourceGroupManager.h
AudioDiskResource.o: ../src/audio/audio.h ../src/SharedPtr.h
AudioDiskResource.o: ../depend_stubs/functional
AudioDiskResource.o: ../../dependencies/util/math_util.h
AudioDiskResource.o: ../depend_stubs/cmath
AudioDiskResource.o: ../depend_stubs/cfloat
AudioDiskResource.o: ../src/DiskResource.h ../depend_stubs/set
AudioDiskResource.o: ../src/portable_io.h ../depend_stubs/cstdio
AudioDiskResource.o: ../depend_stubs/cassert
AudioDiskResource.o: ../depend_stubs/cstring
AudioDiskResource.o: ../depend_stubs/fstream
AudioDiskResource.o: ../depend_stubs/stdint.h
AudioDiskResource.o: ../src/audio/OggVorbisDecoder.h
AudioDiskResource.o: ../depend_stubs/vorbis/codec.h
AudioDiskResource.o: ../depend_stubs/vorbis/vorbisfile.h
OggVorbisDecoder.o: ../src/audio/OggVorbisDecoder.h
OggVorbisDecoder.o: ../depend_stubs/string
OggVorbisDecoder.o: ../depend_stubs/vorbis/codec.h
OggVorbisDecoder.o: ../depend_stubs/vorbis/vorbisfile.h
OggVorbisDecoder.o: ../depend_stubs/OgreResourceGroupManager.h
OggVorbisDecoder.o: ../src/CentralisedLog.h
audio.o: ../depend_stubs/AL/al.h ../depend_stubs/AL/alc.h
audio.o: ../src/CentralisedLog.h ../src/vect_util.h
audio.o: ../depend_stubs/vector ../depend_stubs/algorithm
audio.o: ../src/CentralisedLog.h ../depend_stubs/iostream
audio.o: ../depend_stubs/sstream
audio.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
audio.o: ../../dependencies/util/console.h
audio.o: ../../dependencies/util/exception.h
audio.o: ../depend_stubs/cstdlib ../depend_stubs/string
audio.o: ../../dependencies/util/intrinsics.h ../src/option.h
audio.o: ../depend_stubs/ostream ../src/audio/audio.h
audio.o: ../src/SharedPtr.h ../depend_stubs/functional
audio.o: ../../dependencies/util/math_util.h
audio.o: ../depend_stubs/cmath ../depend_stubs/cfloat
audio.o: ../src/BackgroundLoader.h ../src/vect_util.h
audio.o: ../src/DiskResource.h ../depend_stubs/set
audio.o: ../src/audio/AudioDiskResource.h
audio.o: ../depend_stubs/OgreResourceGroupManager.h
lua_wrappers_audio.o: ../src/main.h ../depend_stubs/lua.h
lua_wrappers_audio.o: ../src/Mouse.h ../depend_stubs/vector
lua_wrappers_audio.o: ../src/Keyboard.h ../depend_stubs/string
lua_wrappers_audio.o: ../src/BackgroundLoader.h
lua_wrappers_audio.o: ../src/vect_util.h
lua_wrappers_audio.o: ../depend_stubs/algorithm
lua_wrappers_audio.o: ../src/CentralisedLog.h
lua_wrappers_audio.o: ../depend_stubs/iostream
lua_wrappers_audio.o: ../depend_stubs/sstream
lua_wrappers_audio.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_audio.o: ../../dependencies/util/console.h
lua_wrappers_audio.o: ../../dependencies/util/exception.h
lua_wrappers_audio.o: ../depend_stubs/cstdlib
lua_wrappers_audio.o: ../../dependencies/util/intrinsics.h
lua_wrappers_audio.o: ../depend_stubs/list
lua_wrappers_audio.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_audio.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_audio.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_audio.o: ../src/BulletDebugDrawer.h
lua_wrappers_audio.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_audio.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_audio.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_audio.o: ../src/physics/PhysicsWorld.h
lua_wrappers_audio.o: ../depend_stubs/map
lua_wrappers_audio.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_audio.o: ../depend_stubs/functional
lua_wrappers_audio.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_audio.o: ../depend_stubs/lauxlib.h
lua_wrappers_audio.o: ../depend_stubs/lualib.h
lua_wrappers_audio.o: ../../dependencies/util/math_util.h
lua_wrappers_audio.o: ../depend_stubs/cmath
lua_wrappers_audio.o: ../depend_stubs/cfloat
lua_wrappers_audio.o: ../src/physics/TColParser.h
lua_wrappers_audio.o: ../src/physics/TColLexer
lua_wrappers_audio.o: ../depend_stubs/istream
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_audio.o: ../src/physics/TColLexer-token_ids
lua_wrappers_audio.o: ../depend_stubs/cstdio
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_audio.o: ../src/physics/CollisionMesh.h
lua_wrappers_audio.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_audio.o: ../../dependencies/util/sleep.h
lua_wrappers_audio.o: ../src/DiskResource.h
lua_wrappers_audio.o: ../src/physics/BColParser.h
lua_wrappers_audio.o: ../depend_stubs/stdint.h
lua_wrappers_audio.o: ../src/physics/LooseEnd.h
lua_wrappers_audio.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_audio.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_audio.o: ../src/Streamer.h ../src/GritObject.h
lua_wrappers_audio.o: ../src/ExternalTable.h ../src/LuaPtr.h
lua_wrappers_audio.o: ../../dependencies/util/SplineTable.h
lua_wrappers_audio.o: ../src/LuaPtr.h ../src/ExternalTable.h
lua_wrappers_audio.o: ../src/path_util.h ../src/audio/audio.h
lua_wrappers_audio.o: ../depend_stubs/AL/al.h
lua_wrappers_audio.o: ../src/BackgroundLoader.h
lua_wrappers_audio.o: ../src/audio/AudioDiskResource.h
lua_wrappers_audio.o: ../depend_stubs/OgreResourceGroupManager.h
lua_wrappers_audio.o: ../src/audio/lua_wrappers_audio.h
lua_wrappers_audio.o: ../src/grit_lua_util.h
lua_wrappers_audio.o: ../depend_stubs/OgreException.h
lua_wrappers_audio.o: ../../dependencies/util/lua_util.h
lua_wrappers_audio.o: ../depend_stubs/limits
lua_wrappers_audio.o: ../../dependencies/util/lua_wrappers_common.h
NetAddress.o: ../depend_stubs/cstdio ../src/CentralisedLog.h
NetAddress.o: ../src/net/net.h ../src/SharedPtr.h
NetAddress.o: ../depend_stubs/cstdlib ../depend_stubs/functional
NetAddress.o: ../depend_stubs/string ../depend_stubs/stdint.h
NetAddress.o: ../depend_stubs/sys/socket.h
NetAddress.o: ../depend_stubs/arpa/inet.h
NetAddress.o: ../depend_stubs/sys/param.h
NetAddress.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
NetAddress.o: ../depend_stubs/sys/ioctl.h
NetManager.o: ../depend_stubs/cstdlib
NetManager.o: ../../dependencies/util/sleep.h
NetManager.o: ../src/CentralisedLog.h ../src/grit_lua_util.h
NetManager.o: ../depend_stubs/string ../depend_stubs/lua.h
NetManager.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
NetManager.o: ../depend_stubs/OgreException.h
NetManager.o: ../src/CentralisedLog.h ../depend_stubs/iostream
NetManager.o: ../depend_stubs/sstream
NetManager.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
NetManager.o: ../../dependencies/util/console.h
NetManager.o: ../../dependencies/util/exception.h
NetManager.o: ../../dependencies/util/intrinsics.h
NetManager.o: ../../dependencies/util/lua_util.h
NetManager.o: ../depend_stubs/limits ../depend_stubs/vector
NetManager.o: ../../dependencies/util/math_util.h
NetManager.o: ../depend_stubs/cmath ../depend_stubs/cfloat
NetManager.o: ../../dependencies/util/lua_wrappers_common.h
NetManager.o: ../src/net/net.h ../src/SharedPtr.h
NetManager.o: ../depend_stubs/functional ../depend_stubs/stdint.h
NetManager.o: ../depend_stubs/sys/socket.h
NetManager.o: ../depend_stubs/arpa/inet.h
NetManager.o: ../depend_stubs/sys/param.h
NetManager.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
NetManager.o: ../depend_stubs/sys/ioctl.h ../src/net/NetManager.h
NetManager.o: ../depend_stubs/queue ../src/ExternalTable.h
NetManager.o: ../src/net/lua_wrappers_net.h
NetMessage.o: ../src/net/net.h ../src/SharedPtr.h
NetMessage.o: ../depend_stubs/cstdlib ../depend_stubs/functional
NetMessage.o: ../depend_stubs/string ../depend_stubs/stdint.h
NetMessage.o: ../depend_stubs/sys/socket.h
NetMessage.o: ../depend_stubs/arpa/inet.h
NetMessage.o: ../depend_stubs/sys/param.h
NetMessage.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
NetMessage.o: ../depend_stubs/sys/ioctl.h ../src/CentralisedLog.h
NetMessage.o: ../depend_stubs/stdio.h
lua_wrappers_net.o: ../src/main.h ../depend_stubs/lua.h
lua_wrappers_net.o: ../src/Mouse.h ../depend_stubs/vector
lua_wrappers_net.o: ../src/Keyboard.h ../depend_stubs/string
lua_wrappers_net.o: ../src/BackgroundLoader.h ../src/vect_util.h
lua_wrappers_net.o: ../depend_stubs/algorithm
lua_wrappers_net.o: ../src/CentralisedLog.h
lua_wrappers_net.o: ../depend_stubs/iostream
lua_wrappers_net.o: ../depend_stubs/sstream
lua_wrappers_net.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_net.o: ../../dependencies/util/console.h
lua_wrappers_net.o: ../../dependencies/util/exception.h
lua_wrappers_net.o: ../depend_stubs/cstdlib
lua_wrappers_net.o: ../../dependencies/util/intrinsics.h
lua_wrappers_net.o: ../depend_stubs/list
lua_wrappers_net.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_net.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_net.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_net.o: ../src/BulletDebugDrawer.h
lua_wrappers_net.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_net.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_net.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_net.o: ../src/physics/PhysicsWorld.h
lua_wrappers_net.o: ../depend_stubs/map ../src/CentralisedLog.h
lua_wrappers_net.o: ../src/SharedPtr.h ../depend_stubs/functional
lua_wrappers_net.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_net.o: ../depend_stubs/lauxlib.h
lua_wrappers_net.o: ../depend_stubs/lualib.h
lua_wrappers_net.o: ../../dependencies/util/math_util.h
lua_wrappers_net.o: ../depend_stubs/cmath ../depend_stubs/cfloat
lua_wrappers_net.o: ../src/physics/TColParser.h
lua_wrappers_net.o: ../src/physics/TColLexer
lua_wrappers_net.o: ../depend_stubs/istream
lua_wrappers_net.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_net.o: ../src/physics/TColLexer-token_ids
lua_wrappers_net.o: ../depend_stubs/cstdio
lua_wrappers_net.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_net.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_net.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_net.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_net.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_net.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_net.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_net.o: ../src/physics/CollisionMesh.h
lua_wrappers_net.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_net.o: ../../dependencies/util/sleep.h
lua_wrappers_net.o: ../src/DiskResource.h
lua_wrappers_net.o: ../src/physics/BColParser.h
lua_wrappers_net.o: ../depend_stubs/stdint.h
lua_wrappers_net.o: ../src/physics/LooseEnd.h
lua_wrappers_net.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_net.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_net.o: ../src/Streamer.h ../src/GritObject.h
lua_wrappers_net.o: ../src/ExternalTable.h ../src/LuaPtr.h
lua_wrappers_net.o: ../../dependencies/util/SplineTable.h
lua_wrappers_net.o: ../src/LuaPtr.h ../src/ExternalTable.h
lua_wrappers_net.o: ../src/path_util.h ../src/net/net.h
lua_wrappers_net.o: ../depend_stubs/sys/socket.h
lua_wrappers_net.o: ../depend_stubs/arpa/inet.h
lua_wrappers_net.o: ../depend_stubs/sys/param.h
lua_wrappers_net.o: ../depend_stubs/sys/types.h
lua_wrappers_net.o: ../depend_stubs/netdb.h
lua_wrappers_net.o: ../depend_stubs/sys/ioctl.h
lua_wrappers_net.o: ../src/net/lua_wrappers_net.h
lua_wrappers_net.o: ../src/grit_lua_util.h
lua_wrappers_net.o: ../depend_stubs/OgreException.h
lua_wrappers_net.o: ../../dependencies/util/lua_util.h
lua_wrappers_net.o: ../depend_stubs/limits
lua_wrappers_net.o: ../../dependencies/util/lua_wrappers_common.h
net.o: ../../dependencies/util/sleep.h ../src/CentralisedLog.h
net.o: ../src/net/net.h ../src/SharedPtr.h ../depend_stubs/cstdlib
net.o: ../depend_stubs/functional ../depend_stubs/string
net.o: ../depend_stubs/stdint.h ../depend_stubs/sys/socket.h
net.o: ../depend_stubs/arpa/inet.h ../depend_stubs/sys/param.h
net.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
net.o: ../depend_stubs/sys/ioctl.h ../src/net/NetManager.h
net.o: ../depend_stubs/queue ../src/ExternalTable.h
