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
	makedepend -Y -f Makefile ../src/*.cpp ../src/linux/*.cpp ../src/win32/*.cpp ../src/gfx/*.cpp ../src/physics/*.cpp ../src/audio/*.cpp -I ../depend_stubs
	sed -i 's_^[.][.]/src/linux/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/win32/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/gfx/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/physics/\([^.]*[.]o:\)_\1_g' Makefile
	sed -i 's_^[.][.]/src/audio/\([^.]*[.]o:\)_\1_g' Makefile
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
        Clutter.o \
        CollisionMesh.o \
        CentralisedLog.o \
        DiskResource.o \
        DenseIndexMap.o \
        ExternalTable.o \
        GfxFont.o \
        gfx.o \
        gfx_option.o \
        GfxBody.o \
        gfx_disk_resource.o \
        GfxTextBuffer.o \
        gfx_hud.o \
        GfxInstances.o \
        GfxLight.o \
        GfxMaterial.o \
        GfxNode.o \
        GfxParticleSystem.o \
        GfxPipeline.o \
        GfxRangedInstances.o \
        GfxSkyBody.o \
        GfxSkyMaterial.o \
        GritClass.o \
        GritEntity.o \
        GritObject.o \
        Streamer.o \
        HUD.o \
        ldbglue.o \
        lua_util.o \
        lua_wrappers_audio.o \
        lua_wrappers_core.o \
        lua_wrappers_disk_resource.o \
        lua_wrappers_gfx.o \
        lua_wrappers_gpuprog.o \
        lua_wrappers_gritobj.o \
        lua_wrappers_hud.o \
        lua_wrappers_material.o \
        lua_wrappers_mesh.o \
        lua_wrappers_mobj.o \
        lua_wrappers_physics.o \
        lua_wrappers_primitives.o \
        lua_wrappers_render.o \
        lua_wrappers_scnmgr.o \
        lua_wrappers_tex.o \
        lua_utf8.o \
        main.o \
        path_util.o \
        PhysicsWorld.o \
        PhysicalMaterial.o \
        ray.o \
        TColLexer.o \
        TColLexer-core-engine.o \
        TColParser.o \
        TextListOverlayElement.o \
        unicode_util.o \
        MouseX11.o \
        KeyboardX11.o \
        posix_sleep.o \
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
               TColLexer-core-engine.o \
               CentralisedLog.o

%/grit_col_conv.$(GRIT_EXEC_SUFFIX): grit_col_conv.$(GRIT_EXEC_SUFFIX)
	strip -s $< -o $@

grit_col_conv.$(GRIT_EXEC_SUFFIX): $(COL_CONV_OBJ)
	@$(LINKING)
	@$(CXX) $(COL_CONV_OBJ) -o $@ $(LDFLAGS) $(LDLIBS)



clean:
	rm -fv $(TARGETS) *.o




# DO NOT DELETE

BackgroundLoader.o: ../src/sleep.h ../src/BackgroundLoader.h
BackgroundLoader.o: ../depend_stubs/vector ../src/vect_util.h
BackgroundLoader.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
BackgroundLoader.o: ../depend_stubs/iostream ../depend_stubs/sstream
BackgroundLoader.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
BackgroundLoader.o: ../src/console_colour.h ../depend_stubs/list
BackgroundLoader.o: ../depend_stubs/boost/thread/thread.hpp
BackgroundLoader.o: ../depend_stubs/boost/thread/condition.hpp
BackgroundLoader.o: ../src/DiskResource.h ../depend_stubs/set
BackgroundLoader.o: ../depend_stubs/string ../src/main.h
BackgroundLoader.o: ../depend_stubs/lua.h ../src/Mouse.h
BackgroundLoader.o: ../src/Keyboard.h ../src/BulletDebugDrawer.h
BackgroundLoader.o: ../depend_stubs/OgreFrameListener.h
BackgroundLoader.o: ../depend_stubs/OgreManualObject.h
BackgroundLoader.o: ../depend_stubs/btBulletCollisionCommon.h
BackgroundLoader.o: ../src/gfx/lua_userdata_dependency_tracker.h
BackgroundLoader.o: ../depend_stubs/OgreStaticGeometry.h
BackgroundLoader.o: ../depend_stubs/OgreInstancedGeometry.h
BackgroundLoader.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
BackgroundLoader.o: ../depend_stubs/OgreMovableObject.h
BackgroundLoader.o: ../depend_stubs/OgreVertexIndexData.h
BackgroundLoader.o: ../depend_stubs/OgreMaterial.h
BackgroundLoader.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
BackgroundLoader.o: ../depend_stubs/cmath
BackgroundLoader.o: ../src/CacheFriendlyRangeSpaceSIMD.h
BackgroundLoader.o: ../depend_stubs/map ../src/SSEAllocator.h
BackgroundLoader.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
BackgroundLoader.o: ../depend_stubs/lualib.h ../src/GritObject.h
BackgroundLoader.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
BackgroundLoader.o: ../depend_stubs/functional ../src/Streamer.h
BackgroundLoader.o: ../src/ExternalTable.h ../src/math_util.h
BackgroundLoader.o: ../src/LuaPtr.h ../src/SplineTable.h
BackgroundLoader.o: ../src/gfx/HUD.h ../depend_stubs/climits
BackgroundLoader.o: ../depend_stubs/OgrePanelOverlayElement.h
BackgroundLoader.o: ../depend_stubs/OgreOverlayElementFactory.h
BackgroundLoader.o: ../depend_stubs/OgreOverlay.h
BackgroundLoader.o: ../depend_stubs/OgreOverlayManager.h
BackgroundLoader.o: ../src/gfx/TextListOverlayElement.h
BackgroundLoader.o: ../depend_stubs/OgrePrerequisites.h
BackgroundLoader.o: ../depend_stubs/OgreOverlayElement.h
BackgroundLoader.o: ../depend_stubs/OgreFont.h
BackgroundLoader.o: ../src/physics/PhysicsWorld.h
BackgroundLoader.o: ../src/CentralisedLog.h ../src/SharedPtr.h
BackgroundLoader.o: ../depend_stubs/btBulletDynamicsCommon.h
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
BackgroundLoader.o: ../depend_stubs/quex/code_base/TokenQueue
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/IncludeStack
BackgroundLoader.o: ../depend_stubs/quex/code_base/temporary_macros_on
BackgroundLoader.o: ../depend_stubs/quex/code_base/temporary_macros_off
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/constructor.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Counter.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Accumulator.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/mode_handling.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/misc.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/buffer_access.i
BackgroundLoader.o: ../depend_stubs/quex/code_base/template/Analyser.i
BackgroundLoader.o: ../src/physics/CollisionMesh.h
BackgroundLoader.o: ../depend_stubs/OgreDataStream.h
BackgroundLoader.o: ../src/DiskResource.h ../src/sleep.h
BackgroundLoader.o: ../src/physics/BColParser.h
BackgroundLoader.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
BackgroundLoader.o: ../src/physics/PhysicalMaterial.h
BackgroundLoader.o: ../src/GritObject.h ../src/LuaPtr.h
BackgroundLoader.o: ../src/gfx/gfx_disk_resource.h
BackgroundLoader.o: ../depend_stubs/OgreResource.h
BackgroundLoader.o: ../depend_stubs/OgreTexture.h
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
BulletDebugDrawer.o: ../src/console_colour.h ../src/gfx/gfx.h
BulletDebugDrawer.o: ../src/SharedPtr.h ../src/vect_util.h
BulletDebugDrawer.o: ../depend_stubs/vector ../depend_stubs/algorithm
BulletDebugDrawer.o: ../depend_stubs/string ../src/math_util.h
BulletDebugDrawer.o: ../depend_stubs/cmath ../depend_stubs/cfloat
BulletDebugDrawer.o: ../src/gfx/gfx_disk_resource.h
BulletDebugDrawer.o: ../depend_stubs/OgreResource.h
BulletDebugDrawer.o: ../depend_stubs/OgreTexture.h
BulletDebugDrawer.o: ../depend_stubs/OgreMesh.h
BulletDebugDrawer.o: ../src/CentralisedLog.h ../src/BackgroundLoader.h
BulletDebugDrawer.o: ../src/vect_util.h ../src/gfx/GfxBody.h
BulletDebugDrawer.o: ../src/gfx/gfx.h ../src/gfx/GfxMaterial.h
BulletDebugDrawer.o: ../depend_stubs/OgreMaterial.h
BulletDebugDrawer.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
BulletDebugDrawer.o: ../depend_stubs/OgreFontManager.h
BulletDebugDrawer.o: ../depend_stubs/OgreMeshManager.h
BulletDebugDrawer.o: ../depend_stubs/OgreOverlayElementFactory.h
BulletDebugDrawer.o: ../depend_stubs/OgreOctreeSceneManager.h
BulletDebugDrawer.o: ../depend_stubs/OgreVector3.h
BulletDebugDrawer.o: ../depend_stubs/OgreQuaternion.h
BulletDebugDrawer.o: ../depend_stubs/OgreCustomCompositionPass.h
BulletDebugDrawer.o: ../depend_stubs/OgreCompositor.h
BulletDebugDrawer.o: ../depend_stubs/OgreOctreePlugin.h
BulletDebugDrawer.o: ../depend_stubs/OgreCgPlugin.h
BulletDebugDrawer.o: ../depend_stubs/OgreRenderSystem.h
BulletDebugDrawer.o: ../depend_stubs/OgreGLRenderSystem.h
BulletDebugDrawer.o: ../src/gfx/gfx_option.h
BulletDebugDrawer.o: ../src/gfx/GfxParticleSystem.h
BulletDebugDrawer.o: ../depend_stubs/utility ../src/gfx/HUD.h
BulletDebugDrawer.o: ../depend_stubs/climits
BulletDebugDrawer.o: ../depend_stubs/OgrePanelOverlayElement.h
BulletDebugDrawer.o: ../depend_stubs/OgreOverlay.h
BulletDebugDrawer.o: ../depend_stubs/OgreOverlayManager.h
BulletDebugDrawer.o: ../src/gfx/TextListOverlayElement.h
BulletDebugDrawer.o: ../depend_stubs/OgrePrerequisites.h
BulletDebugDrawer.o: ../depend_stubs/OgreOverlayElement.h
BulletDebugDrawer.o: ../depend_stubs/OgreFont.h
BulletDebugDrawer.o: ../depend_stubs/OgreRenderWindow.h
BulletDebugDrawer.o: ../src/physics/PhysicsWorld.h ../depend_stubs/map
BulletDebugDrawer.o: ../depend_stubs/btBulletDynamicsCommon.h
BulletDebugDrawer.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
BulletDebugDrawer.o: ../depend_stubs/lualib.h
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
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/TokenQueue
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/IncludeStack
BulletDebugDrawer.o: ../depend_stubs/cstdlib
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/temporary_macros_on
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/temporary_macros_off
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/constructor.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Counter.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/misc.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
BulletDebugDrawer.o: ../depend_stubs/quex/code_base/template/Analyser.i
BulletDebugDrawer.o: ../src/physics/CollisionMesh.h
BulletDebugDrawer.o: ../depend_stubs/OgreDataStream.h
BulletDebugDrawer.o: ../src/DiskResource.h ../depend_stubs/set
BulletDebugDrawer.o: ../src/sleep.h ../src/physics/BColParser.h
BulletDebugDrawer.o: ../depend_stubs/stdint.h
BulletDebugDrawer.o: ../src/physics/LooseEnd.h
BulletDebugDrawer.o: ../src/physics/PhysicalMaterial.h
BulletDebugDrawer.o: ../src/GritObject.h ../src/SharedPtr.h
BulletDebugDrawer.o: ../depend_stubs/functional ../src/LuaPtr.h
CentralisedLog.o: ../src/CentralisedLog.h ../depend_stubs/iostream
CentralisedLog.o: ../depend_stubs/sstream
CentralisedLog.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
CentralisedLog.o: ../src/console_colour.h
DenseIndexMap.o: ../depend_stubs/sstream ../src/DenseIndexMap.h
DenseIndexMap.o: ../depend_stubs/vector ../src/CentralisedLog.h
DenseIndexMap.o: ../depend_stubs/iostream
DenseIndexMap.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
DenseIndexMap.o: ../src/console_colour.h
DiskResource.o: ../src/gfx/gfx_disk_resource.h
DiskResource.o: ../depend_stubs/OgreResource.h
DiskResource.o: ../depend_stubs/OgreTexture.h
DiskResource.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
DiskResource.o: ../depend_stubs/cmath ../depend_stubs/cfloat
DiskResource.o: ../src/CentralisedLog.h ../depend_stubs/iostream
DiskResource.o: ../depend_stubs/sstream
DiskResource.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
DiskResource.o: ../src/console_colour.h ../src/CentralisedLog.h
DiskResource.o: ../src/BackgroundLoader.h ../depend_stubs/vector
DiskResource.o: ../src/vect_util.h ../depend_stubs/algorithm
DiskResource.o: ../src/audio/AudioDiskResource.h
DiskResource.o: ../depend_stubs/AL/al.h
DiskResource.o: ../depend_stubs/OgreResourceGroupManager.h
DiskResource.o: ../src/main.h ../depend_stubs/lua.h ../src/Mouse.h
DiskResource.o: ../src/Keyboard.h ../depend_stubs/string
DiskResource.o: ../src/BackgroundLoader.h ../depend_stubs/list
DiskResource.o: ../depend_stubs/boost/thread/thread.hpp
DiskResource.o: ../depend_stubs/boost/thread/condition.hpp
DiskResource.o: ../src/DiskResource.h ../depend_stubs/set
DiskResource.o: ../src/BulletDebugDrawer.h
DiskResource.o: ../depend_stubs/OgreFrameListener.h
DiskResource.o: ../depend_stubs/OgreManualObject.h
DiskResource.o: ../depend_stubs/btBulletCollisionCommon.h
DiskResource.o: ../src/gfx/lua_userdata_dependency_tracker.h
DiskResource.o: ../depend_stubs/OgreStaticGeometry.h
DiskResource.o: ../depend_stubs/OgreInstancedGeometry.h
DiskResource.o: ../src/gfx/Clutter.h
DiskResource.o: ../depend_stubs/OgreMovableObject.h
DiskResource.o: ../depend_stubs/OgreVertexIndexData.h
DiskResource.o: ../depend_stubs/OgreMaterial.h
DiskResource.o: ../src/CacheFriendlyRangeSpaceSIMD.h
DiskResource.o: ../depend_stubs/map ../src/SSEAllocator.h
DiskResource.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
DiskResource.o: ../depend_stubs/lualib.h ../src/GritObject.h
DiskResource.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
DiskResource.o: ../depend_stubs/functional ../src/Streamer.h
DiskResource.o: ../src/ExternalTable.h ../src/math_util.h
DiskResource.o: ../src/LuaPtr.h ../src/SplineTable.h ../src/gfx/HUD.h
DiskResource.o: ../depend_stubs/climits
DiskResource.o: ../depend_stubs/OgrePanelOverlayElement.h
DiskResource.o: ../depend_stubs/OgreOverlayElementFactory.h
DiskResource.o: ../depend_stubs/OgreOverlay.h
DiskResource.o: ../depend_stubs/OgreOverlayManager.h
DiskResource.o: ../src/gfx/TextListOverlayElement.h
DiskResource.o: ../depend_stubs/OgrePrerequisites.h
DiskResource.o: ../depend_stubs/OgreOverlayElement.h
DiskResource.o: ../depend_stubs/OgreFont.h
DiskResource.o: ../src/physics/PhysicsWorld.h ../src/SharedPtr.h
DiskResource.o: ../depend_stubs/btBulletDynamicsCommon.h
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
DiskResource.o: ../depend_stubs/quex/code_base/TokenQueue
DiskResource.o: ../depend_stubs/quex/code_base/template/IncludeStack
DiskResource.o: ../depend_stubs/quex/code_base/temporary_macros_on
DiskResource.o: ../depend_stubs/quex/code_base/temporary_macros_off
DiskResource.o: ../depend_stubs/quex/code_base/template/constructor.i
DiskResource.o: ../depend_stubs/quex/code_base/template/Counter.i
DiskResource.o: ../depend_stubs/quex/code_base/template/Accumulator.i
DiskResource.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
DiskResource.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
DiskResource.o: ../depend_stubs/quex/code_base/template/mode_handling.i
DiskResource.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
DiskResource.o: ../depend_stubs/quex/code_base/template/misc.i
DiskResource.o: ../depend_stubs/quex/code_base/template/buffer_access.i
DiskResource.o: ../depend_stubs/quex/code_base/template/Analyser.i
DiskResource.o: ../src/physics/CollisionMesh.h
DiskResource.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
DiskResource.o: ../src/sleep.h ../src/physics/BColParser.h
DiskResource.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
DiskResource.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
DiskResource.o: ../src/LuaPtr.h
ExternalTable.o: ../src/ExternalTable.h ../depend_stubs/map
ExternalTable.o: ../depend_stubs/string ../depend_stubs/lua.h
ExternalTable.o: ../depend_stubs/lauxlib.h ../src/math_util.h
ExternalTable.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
ExternalTable.o: ../depend_stubs/functional ../src/LuaPtr.h
ExternalTable.o: ../src/SplineTable.h ../src/lua_util.h
ExternalTable.o: ../depend_stubs/limits ../depend_stubs/vector
ExternalTable.o: ../depend_stubs/lualib.h ../src/CentralisedLog.h
ExternalTable.o: ../depend_stubs/iostream ../depend_stubs/sstream
ExternalTable.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ExternalTable.o: ../src/console_colour.h
ExternalTable.o: ../src/lua_wrappers_primitives.h
ExternalTable.o: ../src/lua_wrappers_common.h
ExternalTable.o: ../depend_stubs/OgreException.h
GritClass.o: ../src/GritClass.h ../depend_stubs/string
GritClass.o: ../depend_stubs/map ../depend_stubs/lua.h
GritClass.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
GritClass.o: ../src/ExternalTable.h ../src/math_util.h
GritClass.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
GritClass.o: ../depend_stubs/functional ../src/LuaPtr.h
GritClass.o: ../src/SplineTable.h ../src/lua_util.h
GritClass.o: ../depend_stubs/limits ../depend_stubs/vector
GritClass.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GritClass.o: ../depend_stubs/sstream
GritClass.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GritClass.o: ../src/console_colour.h ../src/path_util.h
GritObject.o: ../depend_stubs/cmath ../src/main.h
GritObject.o: ../depend_stubs/lua.h ../src/Mouse.h
GritObject.o: ../depend_stubs/vector ../src/Keyboard.h
GritObject.o: ../depend_stubs/string ../src/BackgroundLoader.h
GritObject.o: ../src/vect_util.h ../depend_stubs/algorithm
GritObject.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GritObject.o: ../depend_stubs/sstream
GritObject.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GritObject.o: ../src/console_colour.h ../depend_stubs/list
GritObject.o: ../depend_stubs/boost/thread/thread.hpp
GritObject.o: ../depend_stubs/boost/thread/condition.hpp
GritObject.o: ../src/DiskResource.h ../depend_stubs/set
GritObject.o: ../src/BulletDebugDrawer.h
GritObject.o: ../depend_stubs/OgreFrameListener.h
GritObject.o: ../depend_stubs/OgreManualObject.h
GritObject.o: ../depend_stubs/btBulletCollisionCommon.h
GritObject.o: ../src/gfx/lua_userdata_dependency_tracker.h
GritObject.o: ../depend_stubs/OgreStaticGeometry.h
GritObject.o: ../depend_stubs/OgreInstancedGeometry.h
GritObject.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
GritObject.o: ../depend_stubs/OgreMovableObject.h
GritObject.o: ../depend_stubs/OgreVertexIndexData.h
GritObject.o: ../depend_stubs/OgreMaterial.h
GritObject.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
GritObject.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../depend_stubs/map
GritObject.o: ../src/SSEAllocator.h ../src/Streamer.h
GritObject.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
GritObject.o: ../src/GritObject.h ../src/SharedPtr.h
GritObject.o: ../depend_stubs/cstdlib ../depend_stubs/functional
GritObject.o: ../src/Streamer.h ../src/ExternalTable.h
GritObject.o: ../src/math_util.h ../src/LuaPtr.h ../src/SplineTable.h
GritObject.o: ../src/gfx/HUD.h ../depend_stubs/climits
GritObject.o: ../depend_stubs/OgrePanelOverlayElement.h
GritObject.o: ../depend_stubs/OgreOverlayElementFactory.h
GritObject.o: ../depend_stubs/OgreOverlay.h
GritObject.o: ../depend_stubs/OgreOverlayManager.h
GritObject.o: ../src/gfx/TextListOverlayElement.h
GritObject.o: ../depend_stubs/OgrePrerequisites.h
GritObject.o: ../depend_stubs/OgreOverlayElement.h
GritObject.o: ../depend_stubs/OgreFont.h ../src/physics/PhysicsWorld.h
GritObject.o: ../src/CentralisedLog.h ../src/SharedPtr.h
GritObject.o: ../depend_stubs/btBulletDynamicsCommon.h
GritObject.o: ../src/physics/TColParser.h ../src/physics/TColLexer
GritObject.o: ../depend_stubs/istream
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
GritObject.o: ../depend_stubs/quex/code_base/TokenQueue
GritObject.o: ../depend_stubs/quex/code_base/template/IncludeStack
GritObject.o: ../depend_stubs/quex/code_base/temporary_macros_on
GritObject.o: ../depend_stubs/quex/code_base/temporary_macros_off
GritObject.o: ../depend_stubs/quex/code_base/template/constructor.i
GritObject.o: ../depend_stubs/quex/code_base/template/Counter.i
GritObject.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GritObject.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
GritObject.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
GritObject.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GritObject.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
GritObject.o: ../depend_stubs/quex/code_base/template/misc.i
GritObject.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GritObject.o: ../depend_stubs/quex/code_base/template/Analyser.i
GritObject.o: ../src/physics/CollisionMesh.h
GritObject.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
GritObject.o: ../src/sleep.h ../src/physics/BColParser.h
GritObject.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
GritObject.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
GritObject.o: ../src/LuaPtr.h ../src/GritClass.h ../src/lua_util.h
GritObject.o: ../depend_stubs/limits ../src/path_util.h
GritObject.o: ../src/lua_wrappers_gritobj.h
GritObject.o: ../src/lua_wrappers_common.h
GritObject.o: ../depend_stubs/OgreException.h
Streamer.o: ../src/Streamer.h ../src/main.h ../depend_stubs/lua.h
Streamer.o: ../src/Mouse.h ../depend_stubs/vector ../src/Keyboard.h
Streamer.o: ../depend_stubs/string ../src/BackgroundLoader.h
Streamer.o: ../src/vect_util.h ../depend_stubs/algorithm
Streamer.o: ../src/CentralisedLog.h ../depend_stubs/iostream
Streamer.o: ../depend_stubs/sstream
Streamer.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
Streamer.o: ../src/console_colour.h ../depend_stubs/list
Streamer.o: ../depend_stubs/boost/thread/thread.hpp
Streamer.o: ../depend_stubs/boost/thread/condition.hpp
Streamer.o: ../src/DiskResource.h ../depend_stubs/set
Streamer.o: ../src/BulletDebugDrawer.h
Streamer.o: ../depend_stubs/OgreFrameListener.h
Streamer.o: ../depend_stubs/OgreManualObject.h
Streamer.o: ../depend_stubs/btBulletCollisionCommon.h
Streamer.o: ../src/gfx/lua_userdata_dependency_tracker.h
Streamer.o: ../depend_stubs/OgreStaticGeometry.h
Streamer.o: ../depend_stubs/OgreInstancedGeometry.h
Streamer.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
Streamer.o: ../depend_stubs/OgreMovableObject.h
Streamer.o: ../depend_stubs/OgreVertexIndexData.h
Streamer.o: ../depend_stubs/OgreMaterial.h ../depend_stubs/OgreMesh.h
Streamer.o: ../src/math_util.h ../depend_stubs/cmath
Streamer.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../depend_stubs/map
Streamer.o: ../src/SSEAllocator.h ../src/Streamer.h
Streamer.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
Streamer.o: ../src/GritObject.h ../src/SharedPtr.h
Streamer.o: ../depend_stubs/cstdlib ../depend_stubs/functional
Streamer.o: ../src/ExternalTable.h ../src/math_util.h ../src/LuaPtr.h
Streamer.o: ../src/SplineTable.h ../src/gfx/HUD.h
Streamer.o: ../depend_stubs/climits
Streamer.o: ../depend_stubs/OgrePanelOverlayElement.h
Streamer.o: ../depend_stubs/OgreOverlayElementFactory.h
Streamer.o: ../depend_stubs/OgreOverlay.h
Streamer.o: ../depend_stubs/OgreOverlayManager.h
Streamer.o: ../src/gfx/TextListOverlayElement.h
Streamer.o: ../depend_stubs/OgrePrerequisites.h
Streamer.o: ../depend_stubs/OgreOverlayElement.h
Streamer.o: ../depend_stubs/OgreFont.h ../src/physics/PhysicsWorld.h
Streamer.o: ../src/CentralisedLog.h ../src/SharedPtr.h
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
Streamer.o: ../depend_stubs/quex/code_base/TokenQueue
Streamer.o: ../depend_stubs/quex/code_base/template/IncludeStack
Streamer.o: ../depend_stubs/quex/code_base/temporary_macros_on
Streamer.o: ../depend_stubs/quex/code_base/temporary_macros_off
Streamer.o: ../depend_stubs/quex/code_base/template/constructor.i
Streamer.o: ../depend_stubs/quex/code_base/template/Counter.i
Streamer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
Streamer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
Streamer.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
Streamer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
Streamer.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
Streamer.o: ../depend_stubs/quex/code_base/template/misc.i
Streamer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
Streamer.o: ../depend_stubs/quex/code_base/template/Analyser.i
Streamer.o: ../src/physics/CollisionMesh.h
Streamer.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
Streamer.o: ../src/sleep.h ../src/physics/BColParser.h
Streamer.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
Streamer.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
Streamer.o: ../src/LuaPtr.h ../src/option.h ../depend_stubs/ostream
Streamer.o: ../src/GritClass.h ../src/lua_util.h
Streamer.o: ../depend_stubs/limits ../src/path_util.h
ldbglue.o: ../depend_stubs/lua.h
lua_utf8.o: ../depend_stubs/cstdlib ../depend_stubs/cstdio
lua_utf8.o: ../depend_stubs/cassert ../depend_stubs/iostream
lua_utf8.o: ../depend_stubs/string ../depend_stubs/lua.h
lua_utf8.o: ../depend_stubs/unicode/unistr.h
lua_utf8.o: ../depend_stubs/unicode/regex.h ../src/lua_utf8.h
lua_utf8.o: ../src/lua_wrappers_common.h
lua_utf8.o: ../depend_stubs/OgreException.h ../depend_stubs/lauxlib.h
lua_utf8.o: ../depend_stubs/lualib.h ../src/lua_util.h
lua_utf8.o: ../depend_stubs/limits ../depend_stubs/vector
lua_utf8.o: ../src/CentralisedLog.h ../depend_stubs/sstream
lua_utf8.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_utf8.o: ../src/console_colour.h ../src/math_util.h
lua_util.o: ../depend_stubs/cstdlib ../depend_stubs/cmath
lua_util.o: ../depend_stubs/string ../depend_stubs/map
lua_util.o: ../depend_stubs/sstream ../src/lua_util.h
lua_util.o: ../depend_stubs/limits ../depend_stubs/vector
lua_util.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
lua_util.o: ../depend_stubs/lualib.h ../src/CentralisedLog.h
lua_util.o: ../depend_stubs/iostream
lua_util.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_util.o: ../src/console_colour.h ../src/math_util.h ../src/LuaPtr.h
lua_util.o: ../src/path_util.h
lua_wrappers_core.o: ../depend_stubs/limits.h
lua_wrappers_core.o: ../depend_stubs/sys/mman.h ../src/Keyboard.h
lua_wrappers_core.o: ../depend_stubs/string ../depend_stubs/vector
lua_wrappers_core.o: ../src/Mouse.h ../src/BackgroundLoader.h
lua_wrappers_core.o: ../src/vect_util.h ../depend_stubs/algorithm
lua_wrappers_core.o: ../src/CentralisedLog.h ../depend_stubs/iostream
lua_wrappers_core.o: ../depend_stubs/sstream
lua_wrappers_core.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_core.o: ../src/console_colour.h ../depend_stubs/list
lua_wrappers_core.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_core.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_core.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_core.o: ../src/sleep.h ../src/clipboard.h ../src/main.h
lua_wrappers_core.o: ../depend_stubs/lua.h ../src/BulletDebugDrawer.h
lua_wrappers_core.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_core.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_core.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_core.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_core.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_core.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_core.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
lua_wrappers_core.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_core.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_core.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_core.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
lua_wrappers_core.o: ../depend_stubs/cmath
lua_wrappers_core.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_core.o: ../depend_stubs/map ../src/SSEAllocator.h
lua_wrappers_core.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
lua_wrappers_core.o: ../depend_stubs/lualib.h ../src/GritObject.h
lua_wrappers_core.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
lua_wrappers_core.o: ../depend_stubs/functional ../src/Streamer.h
lua_wrappers_core.o: ../src/ExternalTable.h ../src/math_util.h
lua_wrappers_core.o: ../src/LuaPtr.h ../src/SplineTable.h
lua_wrappers_core.o: ../src/gfx/HUD.h ../depend_stubs/climits
lua_wrappers_core.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_core.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_core.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_core.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_core.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_core.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_core.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_core.o: ../depend_stubs/OgreFont.h
lua_wrappers_core.o: ../src/physics/PhysicsWorld.h
lua_wrappers_core.o: ../src/CentralisedLog.h ../src/SharedPtr.h
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
lua_wrappers_core.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_core.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_core.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_core.o: ../src/physics/CollisionMesh.h
lua_wrappers_core.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_core.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_core.o: ../src/physics/BColParser.h
lua_wrappers_core.o: ../depend_stubs/stdint.h
lua_wrappers_core.o: ../src/physics/LooseEnd.h
lua_wrappers_core.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_core.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_core.o: ../src/lua_utf8.h ../src/gfx/gfx_disk_resource.h
lua_wrappers_core.o: ../depend_stubs/OgreResource.h
lua_wrappers_core.o: ../depend_stubs/OgreTexture.h
lua_wrappers_core.o: ../src/BackgroundLoader.h
lua_wrappers_core.o: ../src/lua_wrappers_primitives.h
lua_wrappers_core.o: ../src/lua_wrappers_common.h
lua_wrappers_core.o: ../depend_stubs/OgreException.h ../src/lua_util.h
lua_wrappers_core.o: ../depend_stubs/limits ../src/path_util.h
lua_wrappers_core.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_core.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_core.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_core.o: ../src/lua_wrappers_common.h ../src/gfx/gfx.h
lua_wrappers_core.o: ../src/vect_util.h ../src/gfx/GfxBody.h
lua_wrappers_core.o: ../src/gfx/GfxMaterial.h
lua_wrappers_core.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
lua_wrappers_core.o: ../depend_stubs/OgreFontManager.h
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
lua_wrappers_core.o: ../src/gfx/gfx_option.h ../src/gfx/GfxSkyBody.h
lua_wrappers_core.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_core.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_core.o: ../src/gfx/GfxSkyMaterial.h ../src/gfx/GfxLight.h
lua_wrappers_core.o: ../src/gfx/GfxParticleSystem.h
lua_wrappers_core.o: ../depend_stubs/utility ../src/gfx/GfxInstances.h
lua_wrappers_core.o: ../src/DenseIndexMap.h
lua_wrappers_core.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_core.o: ../src/gfx/gfx_hud.h ../src/lua_util.h
lua_wrappers_core.o: ../src/path_util.h ../src/gfx/GfxPipeline.h
lua_wrappers_core.o: ../src/gfx/GfxFont.h ../src/main.h
lua_wrappers_core.o: ../src/gfx/GfxTextBuffer.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_core.o: ../src/audio/lua_wrappers_audio.h
lua_wrappers_core.o: ../src/audio/audio.h ../depend_stubs/AL/al.h
lua_wrappers_core.o: ../src/audio/AudioDiskResource.h
lua_wrappers_core.o: ../depend_stubs/OgreResourceGroupManager.h
lua_wrappers_core.o: ../src/physics/lua_wrappers_physics.h
lua_wrappers_core.o: ../src/physics/PhysicsWorld.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreResource.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreTexture.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMesh.h
lua_wrappers_disk_resource.o: ../src/math_util.h ../depend_stubs/cmath
lua_wrappers_disk_resource.o: ../depend_stubs/cfloat
lua_wrappers_disk_resource.o: ../src/CentralisedLog.h
lua_wrappers_disk_resource.o: ../depend_stubs/iostream
lua_wrappers_disk_resource.o: ../depend_stubs/sstream
lua_wrappers_disk_resource.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_disk_resource.o: ../src/console_colour.h
lua_wrappers_disk_resource.o: ../src/CentralisedLog.h
lua_wrappers_disk_resource.o: ../src/BackgroundLoader.h
lua_wrappers_disk_resource.o: ../depend_stubs/vector
lua_wrappers_disk_resource.o: ../src/vect_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/algorithm
lua_wrappers_disk_resource.o: ../src/physics/CollisionMesh.h
lua_wrappers_disk_resource.o: ../depend_stubs/string
lua_wrappers_disk_resource.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_disk_resource.o: ../src/DiskResource.h
lua_wrappers_disk_resource.o: ../depend_stubs/set ../src/sleep.h
lua_wrappers_disk_resource.o: ../src/SharedPtr.h
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
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_disk_resource.o: ../depend_stubs/cstdlib
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_disk_resource.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_disk_resource.o: ../src/physics/BColParser.h
lua_wrappers_disk_resource.o: ../depend_stubs/stdint.h
lua_wrappers_disk_resource.o: ../src/physics/LooseEnd.h
lua_wrappers_disk_resource.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_disk_resource.o: ../src/lua_wrappers_common.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreException.h
lua_wrappers_disk_resource.o: ../depend_stubs/lua.h
lua_wrappers_disk_resource.o: ../depend_stubs/lauxlib.h
lua_wrappers_disk_resource.o: ../depend_stubs/lualib.h
lua_wrappers_disk_resource.o: ../src/lua_util.h ../depend_stubs/limits
lua_wrappers_disk_resource.o: ../src/math_util.h
lua_wrappers_disk_resource.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_disk_resource.o: ../src/DiskResource.h
lua_wrappers_disk_resource.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_disk_resource.o: ../src/lua_wrappers_common.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx.h ../src/vect_util.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxBody.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxMaterial.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_internal.h
lua_wrappers_disk_resource.o: ../depend_stubs/Ogre.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreFontManager.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMeshManager.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreOverlayElementFactory.h
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
lua_wrappers_disk_resource.o: ../src/gfx/GfxSkyBody.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxSkyMaterial.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxLight.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxParticleSystem.h
lua_wrappers_disk_resource.o: ../depend_stubs/utility
lua_wrappers_disk_resource.o: ../src/gfx/GfxInstances.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_disk_resource.o: ../src/DenseIndexMap.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_disk_resource.o: ../src/Streamer.h ../src/GritObject.h
lua_wrappers_disk_resource.o: ../src/SharedPtr.h
lua_wrappers_disk_resource.o: ../depend_stubs/functional
lua_wrappers_disk_resource.o: ../src/Streamer.h ../src/ExternalTable.h
lua_wrappers_disk_resource.o: ../src/LuaPtr.h ../src/SplineTable.h
lua_wrappers_disk_resource.o: ../src/BackgroundLoader.h
lua_wrappers_disk_resource.o: ../depend_stubs/list
lua_wrappers_disk_resource.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_disk_resource.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_disk_resource.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_disk_resource.o: ../src/SSEAllocator.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_hud.h ../src/LuaPtr.h
lua_wrappers_disk_resource.o: ../src/lua_util.h ../src/path_util.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxPipeline.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxFont.h ../src/main.h
lua_wrappers_disk_resource.o: ../src/Mouse.h ../src/Keyboard.h
lua_wrappers_disk_resource.o: ../src/BulletDebugDrawer.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_disk_resource.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_disk_resource.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_disk_resource.o: ../src/gfx/Clutter.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_disk_resource.o: ../src/gfx/HUD.h ../depend_stubs/climits
lua_wrappers_disk_resource.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_disk_resource.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreFont.h
lua_wrappers_disk_resource.o: ../src/physics/PhysicsWorld.h
lua_wrappers_disk_resource.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_disk_resource.o: ../src/physics/CollisionMesh.h
lua_wrappers_disk_resource.o: ../src/GritObject.h
lua_wrappers_disk_resource.o: ../src/gfx/GfxTextBuffer.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_disk_resource.o: ../src/physics/lua_wrappers_physics.h
lua_wrappers_disk_resource.o: ../src/physics/PhysicsWorld.h
lua_wrappers_gritobj.o: ../src/GritObject.h ../depend_stubs/map
lua_wrappers_gritobj.o: ../depend_stubs/vector ../depend_stubs/set
lua_wrappers_gritobj.o: ../depend_stubs/string ../src/SharedPtr.h
lua_wrappers_gritobj.o: ../depend_stubs/cstdlib
lua_wrappers_gritobj.o: ../depend_stubs/functional ../src/Streamer.h
lua_wrappers_gritobj.o: ../src/ExternalTable.h ../depend_stubs/lua.h
lua_wrappers_gritobj.o: ../depend_stubs/lauxlib.h ../src/math_util.h
lua_wrappers_gritobj.o: ../src/LuaPtr.h ../src/SplineTable.h
lua_wrappers_gritobj.o: ../src/BackgroundLoader.h ../src/vect_util.h
lua_wrappers_gritobj.o: ../depend_stubs/algorithm
lua_wrappers_gritobj.o: ../src/CentralisedLog.h
lua_wrappers_gritobj.o: ../depend_stubs/iostream
lua_wrappers_gritobj.o: ../depend_stubs/sstream
lua_wrappers_gritobj.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_gritobj.o: ../src/console_colour.h ../depend_stubs/list
lua_wrappers_gritobj.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_gritobj.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_gritobj.o: ../src/DiskResource.h ../src/GritClass.h
lua_wrappers_gritobj.o: ../depend_stubs/lualib.h ../src/lua_util.h
lua_wrappers_gritobj.o: ../depend_stubs/limits ../src/path_util.h
lua_wrappers_gritobj.o: ../src/main.h ../src/Mouse.h ../src/Keyboard.h
lua_wrappers_gritobj.o: ../src/BulletDebugDrawer.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_gritobj.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_gritobj.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_gritobj.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
lua_wrappers_gritobj.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
lua_wrappers_gritobj.o: ../depend_stubs/cmath
lua_wrappers_gritobj.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_gritobj.o: ../src/SSEAllocator.h ../src/Streamer.h
lua_wrappers_gritobj.o: ../src/gfx/HUD.h ../depend_stubs/climits
lua_wrappers_gritobj.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_gritobj.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_gritobj.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreFont.h
lua_wrappers_gritobj.o: ../src/physics/PhysicsWorld.h
lua_wrappers_gritobj.o: ../src/CentralisedLog.h ../src/SharedPtr.h
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
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_gritobj.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_gritobj.o: ../src/physics/CollisionMesh.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_gritobj.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_gritobj.o: ../src/physics/BColParser.h
lua_wrappers_gritobj.o: ../depend_stubs/stdint.h
lua_wrappers_gritobj.o: ../src/physics/LooseEnd.h
lua_wrappers_gritobj.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_gritobj.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_common.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreException.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_primitives.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_core.h
lua_wrappers_primitives.o: ../depend_stubs/algorithm
lua_wrappers_primitives.o: ../src/lua_wrappers_primitives.h
lua_wrappers_primitives.o: ../src/lua_wrappers_common.h
lua_wrappers_primitives.o: ../depend_stubs/string
lua_wrappers_primitives.o: ../depend_stubs/OgreException.h
lua_wrappers_primitives.o: ../depend_stubs/lua.h
lua_wrappers_primitives.o: ../depend_stubs/lauxlib.h
lua_wrappers_primitives.o: ../depend_stubs/lualib.h ../src/lua_util.h
lua_wrappers_primitives.o: ../depend_stubs/limits
lua_wrappers_primitives.o: ../depend_stubs/vector
lua_wrappers_primitives.o: ../src/CentralisedLog.h
lua_wrappers_primitives.o: ../depend_stubs/iostream
lua_wrappers_primitives.o: ../depend_stubs/sstream
lua_wrappers_primitives.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_primitives.o: ../src/console_colour.h ../src/math_util.h
lua_wrappers_primitives.o: ../src/SplineTable.h
lua_wrappers_primitives.o: ../depend_stubs/cstdlib ../depend_stubs/map
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
main.o: ../src/console_colour.h ../depend_stubs/list
main.o: ../depend_stubs/boost/thread/thread.hpp
main.o: ../depend_stubs/boost/thread/condition.hpp
main.o: ../src/DiskResource.h ../src/BulletDebugDrawer.h
main.o: ../depend_stubs/OgreFrameListener.h
main.o: ../depend_stubs/OgreManualObject.h
main.o: ../depend_stubs/btBulletCollisionCommon.h
main.o: ../src/gfx/lua_userdata_dependency_tracker.h
main.o: ../depend_stubs/OgreStaticGeometry.h
main.o: ../depend_stubs/OgreInstancedGeometry.h ../src/gfx/Clutter.h
main.o: ../depend_stubs/cfloat ../depend_stubs/OgreMovableObject.h
main.o: ../depend_stubs/OgreVertexIndexData.h
main.o: ../depend_stubs/OgreMaterial.h ../depend_stubs/OgreMesh.h
main.o: ../src/math_util.h ../depend_stubs/cmath
main.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../src/SSEAllocator.h
main.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
main.o: ../depend_stubs/lualib.h ../src/GritObject.h
main.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
main.o: ../depend_stubs/functional ../src/Streamer.h
main.o: ../src/ExternalTable.h ../src/math_util.h ../src/LuaPtr.h
main.o: ../src/SplineTable.h ../src/gfx/HUD.h ../depend_stubs/climits
main.o: ../depend_stubs/OgrePanelOverlayElement.h
main.o: ../depend_stubs/OgreOverlayElementFactory.h
main.o: ../depend_stubs/OgreOverlay.h
main.o: ../depend_stubs/OgreOverlayManager.h
main.o: ../src/gfx/TextListOverlayElement.h
main.o: ../depend_stubs/OgrePrerequisites.h
main.o: ../depend_stubs/OgreOverlayElement.h
main.o: ../depend_stubs/OgreFont.h ../src/physics/PhysicsWorld.h
main.o: ../src/CentralisedLog.h ../src/SharedPtr.h
main.o: ../depend_stubs/btBulletDynamicsCommon.h
main.o: ../src/physics/TColParser.h ../src/physics/TColLexer
main.o: ../depend_stubs/istream
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
main.o: ../depend_stubs/quex/code_base/TokenQueue
main.o: ../depend_stubs/quex/code_base/template/IncludeStack
main.o: ../depend_stubs/quex/code_base/temporary_macros_on
main.o: ../depend_stubs/quex/code_base/temporary_macros_off
main.o: ../depend_stubs/quex/code_base/template/constructor.i
main.o: ../depend_stubs/quex/code_base/template/Counter.i
main.o: ../depend_stubs/quex/code_base/template/Accumulator.i
main.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
main.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
main.o: ../depend_stubs/quex/code_base/template/mode_handling.i
main.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
main.o: ../depend_stubs/quex/code_base/template/misc.i
main.o: ../depend_stubs/quex/code_base/template/buffer_access.i
main.o: ../depend_stubs/quex/code_base/template/Analyser.i
main.o: ../src/physics/CollisionMesh.h
main.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
main.o: ../src/sleep.h ../src/physics/BColParser.h
main.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
main.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
main.o: ../src/LuaPtr.h ../src/lua_util.h ../depend_stubs/limits
main.o: ../src/lua_wrappers_core.h ../src/gfx/gfx.h ../src/vect_util.h
main.o: ../src/gfx/gfx_disk_resource.h ../depend_stubs/OgreResource.h
main.o: ../depend_stubs/OgreTexture.h ../src/BackgroundLoader.h
main.o: ../src/gfx/GfxBody.h ../src/gfx/gfx.h ../src/gfx/GfxMaterial.h
main.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
main.o: ../depend_stubs/OgreFontManager.h
main.o: ../depend_stubs/OgreMeshManager.h
main.o: ../depend_stubs/OgreOctreeSceneManager.h
main.o: ../depend_stubs/OgreVector3.h ../depend_stubs/OgreQuaternion.h
main.o: ../depend_stubs/OgreCustomCompositionPass.h
main.o: ../depend_stubs/OgreCompositor.h
main.o: ../depend_stubs/OgreOctreePlugin.h
main.o: ../depend_stubs/OgreCgPlugin.h
main.o: ../depend_stubs/OgreRenderSystem.h
main.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx_option.h
main.o: ../src/gfx/GfxParticleSystem.h ../depend_stubs/utility
main.o: ../src/gfx/HUD.h ../depend_stubs/OgreRoot.h
main.o: ../depend_stubs/OgreRenderWindow.h ../src/audio/audio.h
main.o: ../depend_stubs/AL/al.h ../src/audio/AudioDiskResource.h
main.o: ../depend_stubs/OgreResourceGroupManager.h
path_util.o: ../src/CentralisedLog.h ../depend_stubs/iostream
path_util.o: ../depend_stubs/sstream
path_util.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
path_util.o: ../src/console_colour.h ../src/path_util.h
path_util.o: ../depend_stubs/vector ../depend_stubs/string
path_util.o: ../src/lua_util.h ../depend_stubs/limits
path_util.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
path_util.o: ../depend_stubs/lualib.h ../src/math_util.h
unicode_util.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
unicode_util.o: ../depend_stubs/cstdio ../src/unicode_util.h
unicode_util.o: ../depend_stubs/string
KeyboardX11.o: ../depend_stubs/iostream ../depend_stubs/sstream
KeyboardX11.o: ../src/linux/KeyboardX11.h ../depend_stubs/set
KeyboardX11.o: ../depend_stubs/map ../depend_stubs/X11/Xlib.h
KeyboardX11.o: ../depend_stubs/X11/Xutil.h
KeyboardX11.o: ../depend_stubs/X11/keysym.h ../src/Keyboard.h
KeyboardX11.o: ../depend_stubs/string ../depend_stubs/vector
KeyboardX11.o: ../src/CentralisedLog.h ../src/unicode_util.h
KeyboardX11.o: ../depend_stubs/cstdlib
MouseX11.o: ../depend_stubs/sstream ../src/linux/MouseX11.h
MouseX11.o: ../depend_stubs/X11/Xlib.h ../src/Mouse.h
MouseX11.o: ../depend_stubs/vector ../src/CentralisedLog.h
posix_sleep.o: ../depend_stubs/time.h ../depend_stubs/assert.h
posix_sleep.o: ../depend_stubs/stdio.h
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
KeyboardWinAPI.o: ../src/win32/KeyboardWinAPI.h
KeyboardWinAPI.o: ../depend_stubs/map ../depend_stubs/set
KeyboardWinAPI.o: ../depend_stubs/windows.h ../src/Keyboard.h
KeyboardWinAPI.o: ../depend_stubs/string ../depend_stubs/vector
KeyboardWinAPI.o: ../src/CentralisedLog.h ../src/unicode_util.h
KeyboardWinAPI.o: ../depend_stubs/cstdlib
MouseDirectInput8.o: ../depend_stubs/assert.h
MouseDirectInput8.o: ../depend_stubs/iostream
MouseDirectInput8.o: ../src/win32/MouseDirectInput8.h
MouseDirectInput8.o: ../depend_stubs/dinput.h ../src/Mouse.h
MouseDirectInput8.o: ../depend_stubs/vector
MouseDirectInput8.o: ../src/CentralisedLog.h
win32_clipboard.o: ../depend_stubs/cstdlib
win32_clipboard.o: ../depend_stubs/string
win32_clipboard.o: ../depend_stubs/windows.h
win32_clipboard.o: ../src/unicode_util.h
win32_sleep.o: ../depend_stubs/windows.h
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
Clutter.o: ../src/console_colour.h ../depend_stubs/list
Clutter.o: ../depend_stubs/boost/thread/thread.hpp
Clutter.o: ../depend_stubs/boost/thread/condition.hpp
Clutter.o: ../src/DiskResource.h ../depend_stubs/set
Clutter.o: ../src/BulletDebugDrawer.h
Clutter.o: ../depend_stubs/OgreFrameListener.h
Clutter.o: ../depend_stubs/OgreManualObject.h
Clutter.o: ../depend_stubs/btBulletCollisionCommon.h
Clutter.o: ../src/gfx/lua_userdata_dependency_tracker.h
Clutter.o: ../depend_stubs/OgreStaticGeometry.h
Clutter.o: ../depend_stubs/OgreInstancedGeometry.h
Clutter.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
Clutter.o: ../depend_stubs/OgreMovableObject.h
Clutter.o: ../depend_stubs/OgreVertexIndexData.h
Clutter.o: ../depend_stubs/OgreMaterial.h ../src/math_util.h
Clutter.o: ../depend_stubs/cmath
Clutter.o: ../src/CacheFriendlyRangeSpaceSIMD.h
Clutter.o: ../depend_stubs/map ../src/SSEAllocator.h
Clutter.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
Clutter.o: ../depend_stubs/lualib.h ../src/GritObject.h
Clutter.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
Clutter.o: ../depend_stubs/functional ../src/Streamer.h
Clutter.o: ../src/ExternalTable.h ../src/math_util.h
Clutter.o: ../src/LuaPtr.h ../src/SplineTable.h ../src/gfx/HUD.h
Clutter.o: ../depend_stubs/climits
Clutter.o: ../depend_stubs/OgrePanelOverlayElement.h
Clutter.o: ../depend_stubs/OgreOverlayElementFactory.h
Clutter.o: ../depend_stubs/OgreOverlay.h
Clutter.o: ../depend_stubs/OgreOverlayManager.h
Clutter.o: ../src/gfx/TextListOverlayElement.h
Clutter.o: ../depend_stubs/OgrePrerequisites.h
Clutter.o: ../depend_stubs/OgreOverlayElement.h
Clutter.o: ../depend_stubs/OgreFont.h
Clutter.o: ../src/physics/PhysicsWorld.h ../src/CentralisedLog.h
Clutter.o: ../src/SharedPtr.h
Clutter.o: ../depend_stubs/btBulletDynamicsCommon.h
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
Clutter.o: ../depend_stubs/quex/code_base/TokenQueue
Clutter.o: ../depend_stubs/quex/code_base/template/IncludeStack
Clutter.o: ../depend_stubs/quex/code_base/temporary_macros_on
Clutter.o: ../depend_stubs/quex/code_base/temporary_macros_off
Clutter.o: ../depend_stubs/quex/code_base/template/constructor.i
Clutter.o: ../depend_stubs/quex/code_base/template/Counter.i
Clutter.o: ../depend_stubs/quex/code_base/template/Accumulator.i
Clutter.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
Clutter.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
Clutter.o: ../depend_stubs/quex/code_base/template/mode_handling.i
Clutter.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
Clutter.o: ../depend_stubs/quex/code_base/template/misc.i
Clutter.o: ../depend_stubs/quex/code_base/template/buffer_access.i
Clutter.o: ../depend_stubs/quex/code_base/template/Analyser.i
Clutter.o: ../src/physics/CollisionMesh.h
Clutter.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
Clutter.o: ../src/sleep.h ../src/physics/BColParser.h
Clutter.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
Clutter.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
Clutter.o: ../src/LuaPtr.h ../src/gfx/gfx.h ../src/vect_util.h
GfxBody.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
GfxBody.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxBody.o: ../depend_stubs/OgreFontManager.h
GfxBody.o: ../depend_stubs/OgreMeshManager.h
GfxBody.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxBody.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxBody.o: ../depend_stubs/OgreVector3.h
GfxBody.o: ../depend_stubs/OgreQuaternion.h
GfxBody.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxBody.o: ../depend_stubs/OgreCompositor.h
GfxBody.o: ../depend_stubs/OgreOctreePlugin.h
GfxBody.o: ../depend_stubs/OgreCgPlugin.h
GfxBody.o: ../depend_stubs/OgreRenderSystem.h
GfxBody.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
GfxBody.o: ../src/SharedPtr.h ../src/vect_util.h
GfxBody.o: ../depend_stubs/vector ../depend_stubs/algorithm
GfxBody.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxBody.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxBody.o: ../src/console_colour.h ../src/gfx/gfx_option.h
GfxBody.o: ../src/gfx/GfxBody.h ../depend_stubs/OgreMesh.h
GfxBody.o: ../src/gfx/GfxMaterial.h ../depend_stubs/OgreMaterial.h
GfxBody.o: ../src/gfx/GritEntity.h
GfxBody.o: ../depend_stubs/OgreMovableObject.h
GfxFont.o: ../src/gfx/GfxFont.h ../depend_stubs/vector
GfxFont.o: ../depend_stubs/map ../src/BackgroundLoader.h
GfxFont.o: ../src/vect_util.h ../depend_stubs/algorithm
GfxFont.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxFont.o: ../depend_stubs/sstream
GfxFont.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxFont.o: ../src/console_colour.h ../src/main.h
GfxFont.o: ../depend_stubs/lua.h ../src/Mouse.h ../src/Keyboard.h
GfxFont.o: ../depend_stubs/string ../src/BackgroundLoader.h
GfxFont.o: ../depend_stubs/list
GfxFont.o: ../depend_stubs/boost/thread/thread.hpp
GfxFont.o: ../depend_stubs/boost/thread/condition.hpp
GfxFont.o: ../src/DiskResource.h ../depend_stubs/set
GfxFont.o: ../src/BulletDebugDrawer.h
GfxFont.o: ../depend_stubs/OgreFrameListener.h
GfxFont.o: ../depend_stubs/OgreManualObject.h
GfxFont.o: ../depend_stubs/btBulletCollisionCommon.h
GfxFont.o: ../src/gfx/lua_userdata_dependency_tracker.h
GfxFont.o: ../depend_stubs/OgreStaticGeometry.h
GfxFont.o: ../depend_stubs/OgreInstancedGeometry.h
GfxFont.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
GfxFont.o: ../depend_stubs/OgreMovableObject.h
GfxFont.o: ../depend_stubs/OgreVertexIndexData.h
GfxFont.o: ../depend_stubs/OgreMaterial.h
GfxFont.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
GfxFont.o: ../depend_stubs/cmath
GfxFont.o: ../src/CacheFriendlyRangeSpaceSIMD.h
GfxFont.o: ../src/SSEAllocator.h ../src/Streamer.h
GfxFont.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
GfxFont.o: ../src/GritObject.h ../src/SharedPtr.h
GfxFont.o: ../depend_stubs/cstdlib ../depend_stubs/functional
GfxFont.o: ../src/Streamer.h ../src/ExternalTable.h
GfxFont.o: ../src/math_util.h ../src/LuaPtr.h ../src/SplineTable.h
GfxFont.o: ../src/gfx/HUD.h ../depend_stubs/climits
GfxFont.o: ../depend_stubs/OgrePanelOverlayElement.h
GfxFont.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxFont.o: ../depend_stubs/OgreOverlay.h
GfxFont.o: ../depend_stubs/OgreOverlayManager.h
GfxFont.o: ../src/gfx/TextListOverlayElement.h
GfxFont.o: ../depend_stubs/OgrePrerequisites.h
GfxFont.o: ../depend_stubs/OgreOverlayElement.h
GfxFont.o: ../depend_stubs/OgreFont.h
GfxFont.o: ../src/physics/PhysicsWorld.h ../src/CentralisedLog.h
GfxFont.o: ../src/SharedPtr.h
GfxFont.o: ../depend_stubs/btBulletDynamicsCommon.h
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
GfxFont.o: ../depend_stubs/quex/code_base/TokenQueue
GfxFont.o: ../depend_stubs/quex/code_base/template/IncludeStack
GfxFont.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxFont.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxFont.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxFont.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxFont.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxFont.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
GfxFont.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
GfxFont.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxFont.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
GfxFont.o: ../depend_stubs/quex/code_base/template/misc.i
GfxFont.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxFont.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxFont.o: ../src/physics/CollisionMesh.h
GfxFont.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
GfxFont.o: ../src/sleep.h ../src/physics/BColParser.h
GfxFont.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
GfxFont.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
GfxFont.o: ../src/LuaPtr.h ../src/gfx/gfx_disk_resource.h
GfxFont.o: ../depend_stubs/OgreResource.h
GfxFont.o: ../depend_stubs/OgreTexture.h
GfxInstances.o: ../depend_stubs/algorithm ../src/main.h
GfxInstances.o: ../depend_stubs/lua.h ../src/Mouse.h
GfxInstances.o: ../depend_stubs/vector ../src/Keyboard.h
GfxInstances.o: ../depend_stubs/string ../src/BackgroundLoader.h
GfxInstances.o: ../src/vect_util.h ../src/CentralisedLog.h
GfxInstances.o: ../depend_stubs/iostream ../depend_stubs/sstream
GfxInstances.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxInstances.o: ../src/console_colour.h ../depend_stubs/list
GfxInstances.o: ../depend_stubs/boost/thread/thread.hpp
GfxInstances.o: ../depend_stubs/boost/thread/condition.hpp
GfxInstances.o: ../src/DiskResource.h ../depend_stubs/set
GfxInstances.o: ../src/BulletDebugDrawer.h
GfxInstances.o: ../depend_stubs/OgreFrameListener.h
GfxInstances.o: ../depend_stubs/OgreManualObject.h
GfxInstances.o: ../depend_stubs/btBulletCollisionCommon.h
GfxInstances.o: ../src/gfx/lua_userdata_dependency_tracker.h
GfxInstances.o: ../depend_stubs/OgreStaticGeometry.h
GfxInstances.o: ../depend_stubs/OgreInstancedGeometry.h
GfxInstances.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
GfxInstances.o: ../depend_stubs/OgreMovableObject.h
GfxInstances.o: ../depend_stubs/OgreVertexIndexData.h
GfxInstances.o: ../depend_stubs/OgreMaterial.h
GfxInstances.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
GfxInstances.o: ../depend_stubs/cmath
GfxInstances.o: ../src/CacheFriendlyRangeSpaceSIMD.h
GfxInstances.o: ../depend_stubs/map ../src/SSEAllocator.h
GfxInstances.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
GfxInstances.o: ../depend_stubs/lualib.h ../src/GritObject.h
GfxInstances.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
GfxInstances.o: ../depend_stubs/functional ../src/Streamer.h
GfxInstances.o: ../src/ExternalTable.h ../src/math_util.h
GfxInstances.o: ../src/LuaPtr.h ../src/SplineTable.h
GfxInstances.o: ../src/gfx/HUD.h ../depend_stubs/climits
GfxInstances.o: ../depend_stubs/OgrePanelOverlayElement.h
GfxInstances.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxInstances.o: ../depend_stubs/OgreOverlay.h
GfxInstances.o: ../depend_stubs/OgreOverlayManager.h
GfxInstances.o: ../src/gfx/TextListOverlayElement.h
GfxInstances.o: ../depend_stubs/OgrePrerequisites.h
GfxInstances.o: ../depend_stubs/OgreOverlayElement.h
GfxInstances.o: ../depend_stubs/OgreFont.h
GfxInstances.o: ../src/physics/PhysicsWorld.h
GfxInstances.o: ../src/CentralisedLog.h ../src/SharedPtr.h
GfxInstances.o: ../depend_stubs/btBulletDynamicsCommon.h
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
GfxInstances.o: ../depend_stubs/quex/code_base/TokenQueue
GfxInstances.o: ../depend_stubs/quex/code_base/template/IncludeStack
GfxInstances.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxInstances.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxInstances.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/misc.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxInstances.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxInstances.o: ../src/physics/CollisionMesh.h
GfxInstances.o: ../depend_stubs/OgreDataStream.h
GfxInstances.o: ../src/DiskResource.h ../src/sleep.h
GfxInstances.o: ../src/physics/BColParser.h
GfxInstances.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
GfxInstances.o: ../src/physics/PhysicalMaterial.h
GfxInstances.o: ../src/GritObject.h ../src/LuaPtr.h
GfxInstances.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
GfxInstances.o: ../depend_stubs/OgreFontManager.h
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
GfxInstances.o: ../src/gfx/gfx_option.h ../src/gfx/GfxInstances.h
GfxInstances.o: ../src/DenseIndexMap.h ../src/gfx/GfxBody.h
GfxInstances.o: ../src/gfx/GfxMaterial.h
GfxLight.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
GfxLight.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxLight.o: ../depend_stubs/OgreFontManager.h
GfxLight.o: ../depend_stubs/OgreMeshManager.h
GfxLight.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxLight.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxLight.o: ../depend_stubs/OgreVector3.h
GfxLight.o: ../depend_stubs/OgreQuaternion.h
GfxLight.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxLight.o: ../depend_stubs/OgreCompositor.h
GfxLight.o: ../depend_stubs/OgreOctreePlugin.h
GfxLight.o: ../depend_stubs/OgreCgPlugin.h
GfxLight.o: ../depend_stubs/OgreRenderSystem.h
GfxLight.o: ../depend_stubs/OgreGLRenderSystem.h ../src/gfx/gfx.h
GfxLight.o: ../src/SharedPtr.h ../src/vect_util.h
GfxLight.o: ../depend_stubs/vector ../depend_stubs/algorithm
GfxLight.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxLight.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxLight.o: ../src/console_colour.h ../src/gfx/gfx_option.h
GfxLight.o: ../src/gfx/GfxLight.h ../src/gfx/GfxBody.h
GfxLight.o: ../depend_stubs/OgreMesh.h ../src/gfx/GfxMaterial.h
GfxLight.o: ../depend_stubs/OgreMaterial.h
GfxLight.o: ../src/gfx/GfxParticleSystem.h ../depend_stubs/utility
GfxLight.o: ../src/math_util.h ../depend_stubs/cmath
GfxLight.o: ../depend_stubs/cfloat
GfxMaterial.o: ../depend_stubs/set ../src/gfx/gfx_internal.h
GfxMaterial.o: ../depend_stubs/string ../depend_stubs/sstream
GfxMaterial.o: ../depend_stubs/Ogre.h
GfxMaterial.o: ../depend_stubs/OgreFontManager.h
GfxMaterial.o: ../depend_stubs/OgreMeshManager.h
GfxMaterial.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxMaterial.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxMaterial.o: ../depend_stubs/OgreVector3.h
GfxMaterial.o: ../depend_stubs/OgreQuaternion.h
GfxMaterial.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxMaterial.o: ../depend_stubs/OgreCompositor.h
GfxMaterial.o: ../depend_stubs/OgreOctreePlugin.h
GfxMaterial.o: ../depend_stubs/OgreCgPlugin.h
GfxMaterial.o: ../depend_stubs/OgreRenderSystem.h
GfxMaterial.o: ../depend_stubs/OgreGLRenderSystem.h
GfxMaterial.o: ../src/gfx/gfx.h ../src/SharedPtr.h
GfxMaterial.o: ../src/vect_util.h ../depend_stubs/vector
GfxMaterial.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxMaterial.o: ../depend_stubs/iostream
GfxMaterial.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxMaterial.o: ../src/console_colour.h ../src/gfx/gfx_option.h
GfxMaterial.o: ../src/gfx/GfxMaterial.h
GfxMaterial.o: ../depend_stubs/OgreMaterial.h ../src/gfx/GfxBody.h
GfxMaterial.o: ../depend_stubs/OgreMesh.h
GfxParticleSystem.o: ../depend_stubs/string
GfxParticleSystem.o: ../depend_stubs/algorithm ../depend_stubs/map
GfxParticleSystem.o: ../src/vect_util.h ../depend_stubs/vector
GfxParticleSystem.o: ../src/CentralisedLog.h
GfxParticleSystem.o: ../depend_stubs/iostream
GfxParticleSystem.o: ../depend_stubs/sstream
GfxParticleSystem.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxParticleSystem.o: ../src/console_colour.h ../src/math_util.h
GfxParticleSystem.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxParticleSystem.o: ../src/gfx/gfx.h ../src/SharedPtr.h
GfxParticleSystem.o: ../src/gfx/gfx_internal.h
GfxParticleSystem.o: ../depend_stubs/Ogre.h
GfxParticleSystem.o: ../depend_stubs/OgreFontManager.h
GfxParticleSystem.o: ../depend_stubs/OgreMeshManager.h
GfxParticleSystem.o: ../depend_stubs/OgreOverlayElementFactory.h
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
GfxPipeline.o: ../depend_stubs/OgreString.h ../src/math_util.h
GfxPipeline.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxPipeline.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxPipeline.o: ../depend_stubs/sstream
GfxPipeline.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxPipeline.o: ../src/console_colour.h ../src/sleep.h
GfxPipeline.o: ../src/gfx/GfxPipeline.h ../src/SharedPtr.h
GfxPipeline.o: ../src/vect_util.h ../depend_stubs/vector
GfxPipeline.o: ../depend_stubs/algorithm ../src/gfx/gfx_internal.h
GfxPipeline.o: ../depend_stubs/string ../depend_stubs/Ogre.h
GfxPipeline.o: ../depend_stubs/OgreFontManager.h
GfxPipeline.o: ../depend_stubs/OgreMeshManager.h
GfxPipeline.o: ../depend_stubs/OgreOverlayElementFactory.h
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
GfxRangedInstances.o: ../depend_stubs/algorithm ../src/main.h
GfxRangedInstances.o: ../depend_stubs/lua.h ../src/Mouse.h
GfxRangedInstances.o: ../depend_stubs/vector ../src/Keyboard.h
GfxRangedInstances.o: ../depend_stubs/string
GfxRangedInstances.o: ../src/BackgroundLoader.h ../src/vect_util.h
GfxRangedInstances.o: ../src/CentralisedLog.h
GfxRangedInstances.o: ../depend_stubs/iostream
GfxRangedInstances.o: ../depend_stubs/sstream
GfxRangedInstances.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxRangedInstances.o: ../src/console_colour.h ../depend_stubs/list
GfxRangedInstances.o: ../depend_stubs/boost/thread/thread.hpp
GfxRangedInstances.o: ../depend_stubs/boost/thread/condition.hpp
GfxRangedInstances.o: ../src/DiskResource.h ../depend_stubs/set
GfxRangedInstances.o: ../src/BulletDebugDrawer.h
GfxRangedInstances.o: ../depend_stubs/OgreFrameListener.h
GfxRangedInstances.o: ../depend_stubs/OgreManualObject.h
GfxRangedInstances.o: ../depend_stubs/btBulletCollisionCommon.h
GfxRangedInstances.o: ../src/gfx/lua_userdata_dependency_tracker.h
GfxRangedInstances.o: ../depend_stubs/OgreStaticGeometry.h
GfxRangedInstances.o: ../depend_stubs/OgreInstancedGeometry.h
GfxRangedInstances.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
GfxRangedInstances.o: ../depend_stubs/OgreMovableObject.h
GfxRangedInstances.o: ../depend_stubs/OgreVertexIndexData.h
GfxRangedInstances.o: ../depend_stubs/OgreMaterial.h
GfxRangedInstances.o: ../depend_stubs/OgreMesh.h
GfxRangedInstances.o: ../src/math_util.h ../depend_stubs/cmath
GfxRangedInstances.o: ../src/CacheFriendlyRangeSpaceSIMD.h
GfxRangedInstances.o: ../depend_stubs/map ../src/SSEAllocator.h
GfxRangedInstances.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
GfxRangedInstances.o: ../depend_stubs/lualib.h ../src/GritObject.h
GfxRangedInstances.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
GfxRangedInstances.o: ../depend_stubs/functional ../src/Streamer.h
GfxRangedInstances.o: ../src/ExternalTable.h ../src/math_util.h
GfxRangedInstances.o: ../src/LuaPtr.h ../src/SplineTable.h
GfxRangedInstances.o: ../src/gfx/HUD.h ../depend_stubs/climits
GfxRangedInstances.o: ../depend_stubs/OgrePanelOverlayElement.h
GfxRangedInstances.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxRangedInstances.o: ../depend_stubs/OgreOverlay.h
GfxRangedInstances.o: ../depend_stubs/OgreOverlayManager.h
GfxRangedInstances.o: ../src/gfx/TextListOverlayElement.h
GfxRangedInstances.o: ../depend_stubs/OgrePrerequisites.h
GfxRangedInstances.o: ../depend_stubs/OgreOverlayElement.h
GfxRangedInstances.o: ../depend_stubs/OgreFont.h
GfxRangedInstances.o: ../src/physics/PhysicsWorld.h
GfxRangedInstances.o: ../src/CentralisedLog.h ../src/SharedPtr.h
GfxRangedInstances.o: ../depend_stubs/btBulletDynamicsCommon.h
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
GfxRangedInstances.o: ../depend_stubs/quex/code_base/TokenQueue
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/IncludeStack
GfxRangedInstances.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxRangedInstances.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/misc.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxRangedInstances.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxRangedInstances.o: ../src/physics/CollisionMesh.h
GfxRangedInstances.o: ../depend_stubs/OgreDataStream.h
GfxRangedInstances.o: ../src/DiskResource.h ../src/sleep.h
GfxRangedInstances.o: ../src/physics/BColParser.h
GfxRangedInstances.o: ../depend_stubs/stdint.h
GfxRangedInstances.o: ../src/physics/LooseEnd.h
GfxRangedInstances.o: ../src/physics/PhysicalMaterial.h
GfxRangedInstances.o: ../src/GritObject.h ../src/LuaPtr.h
GfxRangedInstances.o: ../src/gfx/gfx_internal.h
GfxRangedInstances.o: ../depend_stubs/Ogre.h
GfxRangedInstances.o: ../depend_stubs/OgreFontManager.h
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
GfxRangedInstances.o: ../src/gfx/GfxInstances.h
GfxRangedInstances.o: ../src/DenseIndexMap.h ../src/gfx/GfxBody.h
GfxRangedInstances.o: ../src/gfx/GfxMaterial.h
GfxSkyBody.o: ../depend_stubs/OgreMeshManager.h
GfxSkyBody.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
GfxSkyBody.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxSkyBody.o: ../depend_stubs/OgreFontManager.h
GfxSkyBody.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxSkyBody.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxSkyBody.o: ../depend_stubs/OgreVector3.h
GfxSkyBody.o: ../depend_stubs/OgreQuaternion.h
GfxSkyBody.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxSkyBody.o: ../depend_stubs/OgreCompositor.h
GfxSkyBody.o: ../depend_stubs/OgreOctreePlugin.h
GfxSkyBody.o: ../depend_stubs/OgreCgPlugin.h
GfxSkyBody.o: ../depend_stubs/OgreRenderSystem.h
GfxSkyBody.o: ../depend_stubs/OgreGLRenderSystem.h
GfxSkyBody.o: ../src/gfx/gfx.h ../src/SharedPtr.h
GfxSkyBody.o: ../src/vect_util.h ../depend_stubs/vector
GfxSkyBody.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
GfxSkyBody.o: ../depend_stubs/iostream
GfxSkyBody.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxSkyBody.o: ../src/console_colour.h ../src/gfx/gfx_option.h
GfxSkyBody.o: ../src/gfx/GfxSkyBody.h ../depend_stubs/OgreMesh.h
GfxSkyBody.o: ../depend_stubs/OgreSceneNode.h ../src/math_util.h
GfxSkyBody.o: ../depend_stubs/cmath ../depend_stubs/cfloat
GfxSkyBody.o: ../src/gfx/gfx_disk_resource.h
GfxSkyBody.o: ../depend_stubs/OgreResource.h
GfxSkyBody.o: ../depend_stubs/OgreTexture.h
GfxSkyBody.o: ../src/CentralisedLog.h ../src/BackgroundLoader.h
GfxSkyBody.o: ../src/vect_util.h ../src/gfx/GfxSkyMaterial.h
GfxSkyBody.o: ../depend_stubs/OgreMaterial.h
GfxSkyBody.o: ../src/gfx/GfxPipeline.h
GfxSkyMaterial.o: ../depend_stubs/string ../depend_stubs/map
GfxSkyMaterial.o: ../depend_stubs/OgreCgProgram.h
GfxSkyMaterial.o: ../src/CentralisedLog.h
GfxSkyMaterial.o: ../src/gfx/gfx_internal.h
GfxSkyMaterial.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
GfxSkyMaterial.o: ../depend_stubs/OgreFontManager.h
GfxSkyMaterial.o: ../depend_stubs/OgreMeshManager.h
GfxSkyMaterial.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxSkyMaterial.o: ../depend_stubs/OgreOctreeSceneManager.h
GfxSkyMaterial.o: ../depend_stubs/OgreVector3.h
GfxSkyMaterial.o: ../depend_stubs/OgreQuaternion.h
GfxSkyMaterial.o: ../depend_stubs/OgreCustomCompositionPass.h
GfxSkyMaterial.o: ../depend_stubs/OgreCompositor.h
GfxSkyMaterial.o: ../depend_stubs/OgreOctreePlugin.h
GfxSkyMaterial.o: ../depend_stubs/OgreCgPlugin.h
GfxSkyMaterial.o: ../depend_stubs/OgreRenderSystem.h
GfxSkyMaterial.o: ../depend_stubs/OgreGLRenderSystem.h
GfxSkyMaterial.o: ../src/gfx/gfx.h ../src/SharedPtr.h
GfxSkyMaterial.o: ../src/vect_util.h ../depend_stubs/vector
GfxSkyMaterial.o: ../depend_stubs/algorithm
GfxSkyMaterial.o: ../src/CentralisedLog.h ../depend_stubs/iostream
GfxSkyMaterial.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxSkyMaterial.o: ../src/console_colour.h ../src/gfx/gfx_option.h
GfxSkyMaterial.o: ../src/gfx/GfxSkyMaterial.h
GfxSkyMaterial.o: ../depend_stubs/OgreMaterial.h
GfxSkyMaterial.o: ../src/gfx/GfxSkyBody.h
GfxSkyMaterial.o: ../depend_stubs/OgreMesh.h
GfxSkyMaterial.o: ../depend_stubs/OgreSceneNode.h
GfxSkyMaterial.o: ../src/math_util.h ../depend_stubs/cmath
GfxSkyMaterial.o: ../depend_stubs/cfloat
GfxSkyMaterial.o: ../src/gfx/gfx_disk_resource.h
GfxSkyMaterial.o: ../depend_stubs/OgreResource.h
GfxSkyMaterial.o: ../depend_stubs/OgreTexture.h
GfxSkyMaterial.o: ../src/BackgroundLoader.h ../src/vect_util.h
GfxTextBuffer.o: ../src/gfx/GfxTextBuffer.h
GfxTextBuffer.o: ../depend_stubs/OgreHardwareBufferManager.h
GfxTextBuffer.o: ../depend_stubs/OgreHardwareVertexBuffer.h
GfxTextBuffer.o: ../depend_stubs/OgreHardwareIndexBuffer.h
GfxTextBuffer.o: ../src/math_util.h ../depend_stubs/cmath
GfxTextBuffer.o: ../depend_stubs/cfloat ../src/CentralisedLog.h
GfxTextBuffer.o: ../depend_stubs/iostream ../depend_stubs/sstream
GfxTextBuffer.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
GfxTextBuffer.o: ../src/console_colour.h ../src/gfx/GfxFont.h
GfxTextBuffer.o: ../depend_stubs/vector ../depend_stubs/map
GfxTextBuffer.o: ../src/BackgroundLoader.h ../src/vect_util.h
GfxTextBuffer.o: ../depend_stubs/algorithm ../src/main.h
GfxTextBuffer.o: ../depend_stubs/lua.h ../src/Mouse.h
GfxTextBuffer.o: ../src/Keyboard.h ../depend_stubs/string
GfxTextBuffer.o: ../src/BackgroundLoader.h ../depend_stubs/list
GfxTextBuffer.o: ../depend_stubs/boost/thread/thread.hpp
GfxTextBuffer.o: ../depend_stubs/boost/thread/condition.hpp
GfxTextBuffer.o: ../src/DiskResource.h ../depend_stubs/set
GfxTextBuffer.o: ../src/BulletDebugDrawer.h
GfxTextBuffer.o: ../depend_stubs/OgreFrameListener.h
GfxTextBuffer.o: ../depend_stubs/OgreManualObject.h
GfxTextBuffer.o: ../depend_stubs/btBulletCollisionCommon.h
GfxTextBuffer.o: ../src/gfx/lua_userdata_dependency_tracker.h
GfxTextBuffer.o: ../depend_stubs/OgreStaticGeometry.h
GfxTextBuffer.o: ../depend_stubs/OgreInstancedGeometry.h
GfxTextBuffer.o: ../src/gfx/Clutter.h
GfxTextBuffer.o: ../depend_stubs/OgreMovableObject.h
GfxTextBuffer.o: ../depend_stubs/OgreVertexIndexData.h
GfxTextBuffer.o: ../depend_stubs/OgreMaterial.h
GfxTextBuffer.o: ../depend_stubs/OgreMesh.h
GfxTextBuffer.o: ../src/CacheFriendlyRangeSpaceSIMD.h
GfxTextBuffer.o: ../src/SSEAllocator.h ../src/Streamer.h
GfxTextBuffer.o: ../depend_stubs/lauxlib.h
GfxTextBuffer.o: ../depend_stubs/lualib.h ../src/GritObject.h
GfxTextBuffer.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
GfxTextBuffer.o: ../depend_stubs/functional ../src/Streamer.h
GfxTextBuffer.o: ../src/ExternalTable.h ../src/math_util.h
GfxTextBuffer.o: ../src/LuaPtr.h ../src/SplineTable.h
GfxTextBuffer.o: ../src/gfx/HUD.h ../depend_stubs/climits
GfxTextBuffer.o: ../depend_stubs/OgrePanelOverlayElement.h
GfxTextBuffer.o: ../depend_stubs/OgreOverlayElementFactory.h
GfxTextBuffer.o: ../depend_stubs/OgreOverlay.h
GfxTextBuffer.o: ../depend_stubs/OgreOverlayManager.h
GfxTextBuffer.o: ../src/gfx/TextListOverlayElement.h
GfxTextBuffer.o: ../depend_stubs/OgrePrerequisites.h
GfxTextBuffer.o: ../depend_stubs/OgreOverlayElement.h
GfxTextBuffer.o: ../depend_stubs/OgreFont.h
GfxTextBuffer.o: ../src/physics/PhysicsWorld.h
GfxTextBuffer.o: ../src/CentralisedLog.h ../src/SharedPtr.h
GfxTextBuffer.o: ../depend_stubs/btBulletDynamicsCommon.h
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
GfxTextBuffer.o: ../depend_stubs/quex/code_base/TokenQueue
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/IncludeStack
GfxTextBuffer.o: ../depend_stubs/quex/code_base/temporary_macros_on
GfxTextBuffer.o: ../depend_stubs/quex/code_base/temporary_macros_off
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/constructor.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Counter.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/misc.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
GfxTextBuffer.o: ../depend_stubs/quex/code_base/template/Analyser.i
GfxTextBuffer.o: ../src/physics/CollisionMesh.h
GfxTextBuffer.o: ../depend_stubs/OgreDataStream.h
GfxTextBuffer.o: ../src/DiskResource.h ../src/sleep.h
GfxTextBuffer.o: ../src/physics/BColParser.h
GfxTextBuffer.o: ../depend_stubs/stdint.h
GfxTextBuffer.o: ../src/physics/LooseEnd.h
GfxTextBuffer.o: ../src/physics/PhysicalMaterial.h
GfxTextBuffer.o: ../src/GritObject.h ../src/LuaPtr.h
GfxTextBuffer.o: ../src/gfx/gfx_disk_resource.h
GfxTextBuffer.o: ../depend_stubs/OgreResource.h
GfxTextBuffer.o: ../depend_stubs/OgreTexture.h
GfxTextBuffer.o: ../src/unicode_util.h
GritEntity.o: ../depend_stubs/OgreSceneManager.h
GritEntity.o: ../depend_stubs/OgreVector4.h
GritEntity.o: ../depend_stubs/OgreSubMesh.h
GritEntity.o: ../src/CentralisedLog.h ../src/gfx/GritEntity.h
GritEntity.o: ../depend_stubs/OgreMovableObject.h
GritEntity.o: ../depend_stubs/OgreMesh.h
HUD.o: ../depend_stubs/math.h ../depend_stubs/OgreFontManager.h
HUD.o: ../src/gfx/HUD.h ../depend_stubs/vector
HUD.o: ../depend_stubs/climits
HUD.o: ../depend_stubs/OgrePanelOverlayElement.h
HUD.o: ../depend_stubs/OgreOverlayElementFactory.h
HUD.o: ../depend_stubs/OgreOverlay.h
HUD.o: ../depend_stubs/OgreOverlayManager.h
HUD.o: ../src/gfx/TextListOverlayElement.h
HUD.o: ../depend_stubs/OgrePrerequisites.h
HUD.o: ../depend_stubs/OgreOverlayElement.h
HUD.o: ../depend_stubs/OgreFont.h ../src/CentralisedLog.h
TextListOverlayElement.o: ../depend_stubs/OgreStableHeaders.h
TextListOverlayElement.o: ../depend_stubs/OgreRoot.h
TextListOverlayElement.o: ../depend_stubs/OgreLogManager.h
TextListOverlayElement.o: ../depend_stubs/OgreOverlayManager.h
TextListOverlayElement.o: ../depend_stubs/OgreHardwareBufferManager.h
TextListOverlayElement.o: ../depend_stubs/OgreHardwareVertexBuffer.h
TextListOverlayElement.o: ../depend_stubs/OgreException.h
TextListOverlayElement.o: ../depend_stubs/OgreStringConverter.h
TextListOverlayElement.o: ../depend_stubs/OgreFont.h
TextListOverlayElement.o: ../depend_stubs/OgreFontManager.h
TextListOverlayElement.o: ../src/gfx/TextListOverlayElement.h
TextListOverlayElement.o: ../depend_stubs/OgrePrerequisites.h
TextListOverlayElement.o: ../depend_stubs/OgreOverlayElement.h
TextListOverlayElement.o: ../src/gfx/gfx.h ../src/SharedPtr.h
TextListOverlayElement.o: ../src/vect_util.h
TextListOverlayElement.o: ../depend_stubs/vector
TextListOverlayElement.o: ../depend_stubs/algorithm
TextListOverlayElement.o: ../src/CentralisedLog.h
TextListOverlayElement.o: ../depend_stubs/iostream
TextListOverlayElement.o: ../depend_stubs/sstream
TextListOverlayElement.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
TextListOverlayElement.o: ../src/console_colour.h ../src/main.h
TextListOverlayElement.o: ../depend_stubs/lua.h ../src/Mouse.h
TextListOverlayElement.o: ../src/Keyboard.h ../depend_stubs/string
TextListOverlayElement.o: ../src/BackgroundLoader.h
TextListOverlayElement.o: ../src/vect_util.h ../depend_stubs/list
TextListOverlayElement.o: ../depend_stubs/boost/thread/thread.hpp
TextListOverlayElement.o: ../depend_stubs/boost/thread/condition.hpp
TextListOverlayElement.o: ../src/DiskResource.h
TextListOverlayElement.o: ../depend_stubs/set
TextListOverlayElement.o: ../src/BulletDebugDrawer.h
TextListOverlayElement.o: ../depend_stubs/OgreFrameListener.h
TextListOverlayElement.o: ../depend_stubs/OgreManualObject.h
TextListOverlayElement.o: ../depend_stubs/btBulletCollisionCommon.h
TextListOverlayElement.o: ../src/gfx/lua_userdata_dependency_tracker.h
TextListOverlayElement.o: ../depend_stubs/OgreStaticGeometry.h
TextListOverlayElement.o: ../depend_stubs/OgreInstancedGeometry.h
TextListOverlayElement.o: ../src/gfx/Clutter.h
TextListOverlayElement.o: ../depend_stubs/cfloat
TextListOverlayElement.o: ../depend_stubs/OgreMovableObject.h
TextListOverlayElement.o: ../depend_stubs/OgreVertexIndexData.h
TextListOverlayElement.o: ../depend_stubs/OgreMaterial.h
TextListOverlayElement.o: ../depend_stubs/OgreMesh.h
TextListOverlayElement.o: ../src/math_util.h ../depend_stubs/cmath
TextListOverlayElement.o: ../src/CacheFriendlyRangeSpaceSIMD.h
TextListOverlayElement.o: ../depend_stubs/map
TextListOverlayElement.o: ../src/SSEAllocator.h ../src/Streamer.h
TextListOverlayElement.o: ../depend_stubs/lauxlib.h
TextListOverlayElement.o: ../depend_stubs/lualib.h
TextListOverlayElement.o: ../src/GritObject.h ../src/SharedPtr.h
TextListOverlayElement.o: ../depend_stubs/cstdlib
TextListOverlayElement.o: ../depend_stubs/functional
TextListOverlayElement.o: ../src/Streamer.h ../src/ExternalTable.h
TextListOverlayElement.o: ../src/math_util.h ../src/LuaPtr.h
TextListOverlayElement.o: ../src/SplineTable.h ../src/gfx/HUD.h
TextListOverlayElement.o: ../depend_stubs/climits
TextListOverlayElement.o: ../depend_stubs/OgrePanelOverlayElement.h
TextListOverlayElement.o: ../depend_stubs/OgreOverlayElementFactory.h
TextListOverlayElement.o: ../depend_stubs/OgreOverlay.h
TextListOverlayElement.o: ../src/physics/PhysicsWorld.h
TextListOverlayElement.o: ../src/CentralisedLog.h
TextListOverlayElement.o: ../depend_stubs/btBulletDynamicsCommon.h
TextListOverlayElement.o: ../src/physics/TColParser.h
TextListOverlayElement.o: ../src/physics/TColLexer
TextListOverlayElement.o: ../depend_stubs/istream
TextListOverlayElement.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
TextListOverlayElement.o: ../src/physics/TColLexer-token_ids
TextListOverlayElement.o: ../depend_stubs/cstdio
TextListOverlayElement.o: ../depend_stubs/quex/code_base/Token
TextListOverlayElement.o: ../depend_stubs/quex/code_base/definitions
TextListOverlayElement.o: ../depend_stubs/quex/code_base/buffer/Buffer
TextListOverlayElement.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/QuexMode
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/Analyser
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/Counter
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/Accumulator
TextListOverlayElement.o: ../depend_stubs/quex/code_base/TokenQueue
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/IncludeStack
TextListOverlayElement.o: ../depend_stubs/quex/code_base/temporary_macros_on
TextListOverlayElement.o: ../depend_stubs/quex/code_base/temporary_macros_off
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/constructor.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/Counter.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/Accumulator.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/mode_handling.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/misc.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/buffer_access.i
TextListOverlayElement.o: ../depend_stubs/quex/code_base/template/Analyser.i
TextListOverlayElement.o: ../src/physics/CollisionMesh.h
TextListOverlayElement.o: ../depend_stubs/OgreDataStream.h
TextListOverlayElement.o: ../src/DiskResource.h ../src/sleep.h
TextListOverlayElement.o: ../src/physics/BColParser.h
TextListOverlayElement.o: ../depend_stubs/stdint.h
TextListOverlayElement.o: ../src/physics/LooseEnd.h
TextListOverlayElement.o: ../src/physics/PhysicalMaterial.h
TextListOverlayElement.o: ../src/GritObject.h ../src/LuaPtr.h
gfx.o: ../depend_stubs/sstream
gfx.o: ../depend_stubs/OgreAutoParamDataSource.h
gfx.o: ../depend_stubs/OgreOverlaySystem.h ../src/path_util.h
gfx.o: ../depend_stubs/vector ../depend_stubs/string
gfx.o: ../src/sleep.h ../src/main.h ../depend_stubs/lua.h
gfx.o: ../src/Mouse.h ../src/Keyboard.h ../src/BackgroundLoader.h
gfx.o: ../src/vect_util.h ../depend_stubs/algorithm
gfx.o: ../src/CentralisedLog.h ../depend_stubs/iostream
gfx.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx.o: ../src/console_colour.h ../depend_stubs/list
gfx.o: ../depend_stubs/boost/thread/thread.hpp
gfx.o: ../depend_stubs/boost/thread/condition.hpp
gfx.o: ../src/DiskResource.h ../depend_stubs/set
gfx.o: ../src/BulletDebugDrawer.h
gfx.o: ../depend_stubs/OgreFrameListener.h
gfx.o: ../depend_stubs/OgreManualObject.h
gfx.o: ../depend_stubs/btBulletCollisionCommon.h
gfx.o: ../src/gfx/lua_userdata_dependency_tracker.h
gfx.o: ../depend_stubs/OgreStaticGeometry.h
gfx.o: ../depend_stubs/OgreInstancedGeometry.h
gfx.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
gfx.o: ../depend_stubs/OgreMovableObject.h
gfx.o: ../depend_stubs/OgreVertexIndexData.h
gfx.o: ../depend_stubs/OgreMaterial.h ../depend_stubs/OgreMesh.h
gfx.o: ../src/math_util.h ../depend_stubs/cmath
gfx.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../depend_stubs/map
gfx.o: ../src/SSEAllocator.h ../src/Streamer.h
gfx.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
gfx.o: ../src/GritObject.h ../src/SharedPtr.h
gfx.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx.o: ../src/Streamer.h ../src/ExternalTable.h ../src/math_util.h
gfx.o: ../src/LuaPtr.h ../src/SplineTable.h ../src/gfx/HUD.h
gfx.o: ../depend_stubs/climits
gfx.o: ../depend_stubs/OgrePanelOverlayElement.h
gfx.o: ../depend_stubs/OgreOverlayElementFactory.h
gfx.o: ../depend_stubs/OgreOverlay.h
gfx.o: ../depend_stubs/OgreOverlayManager.h
gfx.o: ../src/gfx/TextListOverlayElement.h
gfx.o: ../depend_stubs/OgrePrerequisites.h
gfx.o: ../depend_stubs/OgreOverlayElement.h
gfx.o: ../depend_stubs/OgreFont.h ../src/physics/PhysicsWorld.h
gfx.o: ../src/CentralisedLog.h ../src/SharedPtr.h
gfx.o: ../depend_stubs/btBulletDynamicsCommon.h
gfx.o: ../src/physics/TColParser.h ../src/physics/TColLexer
gfx.o: ../depend_stubs/istream
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
gfx.o: ../depend_stubs/quex/code_base/TokenQueue
gfx.o: ../depend_stubs/quex/code_base/template/IncludeStack
gfx.o: ../depend_stubs/quex/code_base/temporary_macros_on
gfx.o: ../depend_stubs/quex/code_base/temporary_macros_off
gfx.o: ../depend_stubs/quex/code_base/template/constructor.i
gfx.o: ../depend_stubs/quex/code_base/template/Counter.i
gfx.o: ../depend_stubs/quex/code_base/template/Accumulator.i
gfx.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
gfx.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
gfx.o: ../depend_stubs/quex/code_base/template/mode_handling.i
gfx.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
gfx.o: ../depend_stubs/quex/code_base/template/misc.i
gfx.o: ../depend_stubs/quex/code_base/template/buffer_access.i
gfx.o: ../depend_stubs/quex/code_base/template/Analyser.i
gfx.o: ../src/physics/CollisionMesh.h
gfx.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
gfx.o: ../src/physics/BColParser.h ../depend_stubs/stdint.h
gfx.o: ../src/physics/LooseEnd.h ../src/physics/PhysicalMaterial.h
gfx.o: ../src/GritObject.h ../src/LuaPtr.h
gfx.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
gfx.o: ../depend_stubs/OgreFontManager.h
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
gfx.o: ../src/gfx/GfxPipeline.h ../src/gfx/GfxMaterial.h
gfx.o: ../src/gfx/GfxBody.h ../src/gfx/GfxSkyMaterial.h
gfx.o: ../src/gfx/GfxSkyBody.h ../depend_stubs/OgreSceneNode.h
gfx.o: ../src/gfx/gfx_disk_resource.h
gfx.o: ../depend_stubs/OgreResource.h
gfx.o: ../depend_stubs/OgreTexture.h ../src/BackgroundLoader.h
gfx.o: ../src/gfx/gfx_hud.h ../src/lua_util.h ../src/gfx/GfxFont.h
gfx.o: ../src/gfx/GfxTextBuffer.h
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
gfx_disk_resource.o: ../src/math_util.h ../depend_stubs/cmath
gfx_disk_resource.o: ../depend_stubs/cfloat
gfx_disk_resource.o: ../src/CentralisedLog.h
gfx_disk_resource.o: ../depend_stubs/iostream
gfx_disk_resource.o: ../depend_stubs/sstream
gfx_disk_resource.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx_disk_resource.o: ../src/console_colour.h
gfx_disk_resource.o: ../src/CentralisedLog.h
gfx_disk_resource.o: ../src/BackgroundLoader.h
gfx_disk_resource.o: ../depend_stubs/vector ../src/vect_util.h
gfx_disk_resource.o: ../depend_stubs/algorithm
gfx_disk_resource.o: ../src/gfx/gfx_internal.h
gfx_disk_resource.o: ../depend_stubs/string ../depend_stubs/Ogre.h
gfx_disk_resource.o: ../depend_stubs/OgreFontManager.h
gfx_disk_resource.o: ../depend_stubs/OgreOverlayElementFactory.h
gfx_disk_resource.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_disk_resource.o: ../depend_stubs/OgreVector3.h
gfx_disk_resource.o: ../depend_stubs/OgreQuaternion.h
gfx_disk_resource.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_disk_resource.o: ../depend_stubs/OgreCompositor.h
gfx_disk_resource.o: ../depend_stubs/OgreOctreePlugin.h
gfx_disk_resource.o: ../depend_stubs/OgreCgPlugin.h
gfx_disk_resource.o: ../depend_stubs/OgreRenderSystem.h
gfx_disk_resource.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_disk_resource.o: ../src/gfx/gfx.h ../src/SharedPtr.h
gfx_disk_resource.o: ../src/vect_util.h ../src/gfx/gfx_option.h
gfx_disk_resource.o: ../src/gfx/GfxMaterial.h
gfx_disk_resource.o: ../depend_stubs/OgreMaterial.h
gfx_disk_resource.o: ../src/gfx/GfxBody.h ../src/gfx/GfxSkyBody.h
gfx_disk_resource.o: ../depend_stubs/OgreSceneNode.h
gfx_disk_resource.o: ../src/gfx/GfxSkyMaterial.h
gfx_hud.o: ../depend_stubs/OgreHighLevelGpuProgramManager.h
gfx_hud.o: ../depend_stubs/OgreCgProgram.h ../src/CentralisedLog.h
gfx_hud.o: ../src/path_util.h ../depend_stubs/vector
gfx_hud.o: ../depend_stubs/string ../src/gfx/lua_wrappers_gfx.h
gfx_hud.o: ../src/lua_wrappers_common.h ../src/gfx/gfx.h
gfx_hud.o: ../src/SharedPtr.h ../src/vect_util.h
gfx_hud.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
gfx_hud.o: ../depend_stubs/iostream ../depend_stubs/sstream
gfx_hud.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx_hud.o: ../src/console_colour.h ../src/gfx/GfxBody.h
gfx_hud.o: ../depend_stubs/OgreMesh.h ../src/gfx/GfxMaterial.h
gfx_hud.o: ../depend_stubs/OgreMaterial.h
gfx_hud.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
gfx_hud.o: ../depend_stubs/OgreFontManager.h
gfx_hud.o: ../depend_stubs/OgreMeshManager.h
gfx_hud.o: ../depend_stubs/OgreOverlayElementFactory.h
gfx_hud.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_hud.o: ../depend_stubs/OgreVector3.h
gfx_hud.o: ../depend_stubs/OgreQuaternion.h
gfx_hud.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_hud.o: ../depend_stubs/OgreCompositor.h
gfx_hud.o: ../depend_stubs/OgreOctreePlugin.h
gfx_hud.o: ../depend_stubs/OgreCgPlugin.h
gfx_hud.o: ../depend_stubs/OgreRenderSystem.h
gfx_hud.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_hud.o: ../src/gfx/gfx_option.h ../src/gfx/GfxSkyBody.h
gfx_hud.o: ../depend_stubs/OgreSceneNode.h ../src/math_util.h
gfx_hud.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_hud.o: ../src/gfx/gfx_disk_resource.h
gfx_hud.o: ../depend_stubs/OgreResource.h
gfx_hud.o: ../depend_stubs/OgreTexture.h ../src/BackgroundLoader.h
gfx_hud.o: ../src/vect_util.h ../src/gfx/GfxSkyMaterial.h
gfx_hud.o: ../src/gfx/GfxLight.h ../src/gfx/GfxParticleSystem.h
gfx_hud.o: ../depend_stubs/utility ../src/gfx/GfxInstances.h
gfx_hud.o: ../depend_stubs/OgreMovableObject.h
gfx_hud.o: ../src/DenseIndexMap.h ../src/gfx/GfxRangedInstances.h
gfx_hud.o: ../src/Streamer.h ../depend_stubs/map
gfx_hud.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
gfx_hud.o: ../depend_stubs/lualib.h ../src/GritObject.h
gfx_hud.o: ../depend_stubs/set ../src/SharedPtr.h
gfx_hud.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx_hud.o: ../src/Streamer.h ../src/ExternalTable.h
gfx_hud.o: ../src/math_util.h ../src/LuaPtr.h ../src/SplineTable.h
gfx_hud.o: ../src/BackgroundLoader.h ../depend_stubs/list
gfx_hud.o: ../depend_stubs/boost/thread/thread.hpp
gfx_hud.o: ../depend_stubs/boost/thread/condition.hpp
gfx_hud.o: ../src/DiskResource.h
gfx_hud.o: ../src/CacheFriendlyRangeSpaceSIMD.h
gfx_hud.o: ../src/SSEAllocator.h ../src/gfx/gfx_hud.h
gfx_hud.o: ../src/LuaPtr.h ../src/lua_util.h
gfx_hud.o: ../src/gfx/GfxPipeline.h ../src/gfx/GfxFont.h
gfx_hud.o: ../src/main.h ../src/Mouse.h ../src/Keyboard.h
gfx_hud.o: ../src/BulletDebugDrawer.h
gfx_hud.o: ../depend_stubs/OgreFrameListener.h
gfx_hud.o: ../depend_stubs/OgreManualObject.h
gfx_hud.o: ../depend_stubs/btBulletCollisionCommon.h
gfx_hud.o: ../src/gfx/lua_userdata_dependency_tracker.h
gfx_hud.o: ../depend_stubs/OgreStaticGeometry.h
gfx_hud.o: ../depend_stubs/OgreInstancedGeometry.h
gfx_hud.o: ../src/gfx/Clutter.h
gfx_hud.o: ../depend_stubs/OgreVertexIndexData.h ../src/gfx/HUD.h
gfx_hud.o: ../depend_stubs/climits
gfx_hud.o: ../depend_stubs/OgrePanelOverlayElement.h
gfx_hud.o: ../depend_stubs/OgreOverlay.h
gfx_hud.o: ../depend_stubs/OgreOverlayManager.h
gfx_hud.o: ../src/gfx/TextListOverlayElement.h
gfx_hud.o: ../depend_stubs/OgrePrerequisites.h
gfx_hud.o: ../depend_stubs/OgreOverlayElement.h
gfx_hud.o: ../depend_stubs/OgreFont.h
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
gfx_hud.o: ../depend_stubs/quex/code_base/TokenQueue
gfx_hud.o: ../depend_stubs/quex/code_base/template/IncludeStack
gfx_hud.o: ../depend_stubs/quex/code_base/temporary_macros_on
gfx_hud.o: ../depend_stubs/quex/code_base/temporary_macros_off
gfx_hud.o: ../depend_stubs/quex/code_base/template/constructor.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/Counter.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/Accumulator.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/mode_handling.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/misc.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/buffer_access.i
gfx_hud.o: ../depend_stubs/quex/code_base/template/Analyser.i
gfx_hud.o: ../src/physics/CollisionMesh.h
gfx_hud.o: ../depend_stubs/OgreDataStream.h ../src/DiskResource.h
gfx_hud.o: ../src/sleep.h ../src/physics/BColParser.h
gfx_hud.o: ../depend_stubs/stdint.h ../src/physics/LooseEnd.h
gfx_hud.o: ../src/physics/PhysicalMaterial.h ../src/GritObject.h
gfx_hud.o: ../src/gfx/GfxTextBuffer.h
gfx_hud.o: ../depend_stubs/OgreHardwareBufferManager.h
gfx_hud.o: ../depend_stubs/OgreHardwareVertexBuffer.h
gfx_hud.o: ../depend_stubs/OgreHardwareIndexBuffer.h
gfx_option.o: ../depend_stubs/string ../depend_stubs/map
gfx_option.o: ../src/gfx/gfx_internal.h ../depend_stubs/sstream
gfx_option.o: ../depend_stubs/Ogre.h
gfx_option.o: ../depend_stubs/OgreFontManager.h
gfx_option.o: ../depend_stubs/OgreMeshManager.h
gfx_option.o: ../depend_stubs/OgreOverlayElementFactory.h
gfx_option.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_option.o: ../depend_stubs/OgreVector3.h
gfx_option.o: ../depend_stubs/OgreQuaternion.h
gfx_option.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_option.o: ../depend_stubs/OgreCompositor.h
gfx_option.o: ../depend_stubs/OgreOctreePlugin.h
gfx_option.o: ../depend_stubs/OgreCgPlugin.h
gfx_option.o: ../depend_stubs/OgreRenderSystem.h
gfx_option.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_option.o: ../src/gfx/gfx.h ../src/SharedPtr.h
gfx_option.o: ../src/vect_util.h ../depend_stubs/vector
gfx_option.o: ../depend_stubs/algorithm ../src/CentralisedLog.h
gfx_option.o: ../depend_stubs/iostream
gfx_option.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
gfx_option.o: ../src/console_colour.h ../src/gfx/gfx_option.h
gfx_option.o: ../src/gfx/GfxPipeline.h ../src/option.h
gfx_option.o: ../depend_stubs/ostream ../src/CentralisedLog.h
lua_wrappers_gfx.o: ../src/main.h ../depend_stubs/lua.h
lua_wrappers_gfx.o: ../src/Mouse.h ../depend_stubs/vector
lua_wrappers_gfx.o: ../src/Keyboard.h ../depend_stubs/string
lua_wrappers_gfx.o: ../src/BackgroundLoader.h ../src/vect_util.h
lua_wrappers_gfx.o: ../depend_stubs/algorithm
lua_wrappers_gfx.o: ../src/CentralisedLog.h
lua_wrappers_gfx.o: ../depend_stubs/iostream
lua_wrappers_gfx.o: ../depend_stubs/sstream
lua_wrappers_gfx.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_gfx.o: ../src/console_colour.h ../depend_stubs/list
lua_wrappers_gfx.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_gfx.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_gfx.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_gfx.o: ../src/BulletDebugDrawer.h
lua_wrappers_gfx.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_gfx.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_gfx.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_gfx.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_gfx.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_gfx.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_gfx.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
lua_wrappers_gfx.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_gfx.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
lua_wrappers_gfx.o: ../depend_stubs/cmath
lua_wrappers_gfx.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_gfx.o: ../depend_stubs/map ../src/SSEAllocator.h
lua_wrappers_gfx.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
lua_wrappers_gfx.o: ../depend_stubs/lualib.h ../src/GritObject.h
lua_wrappers_gfx.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
lua_wrappers_gfx.o: ../depend_stubs/functional ../src/Streamer.h
lua_wrappers_gfx.o: ../src/ExternalTable.h ../src/math_util.h
lua_wrappers_gfx.o: ../src/LuaPtr.h ../src/SplineTable.h
lua_wrappers_gfx.o: ../src/gfx/HUD.h ../depend_stubs/climits
lua_wrappers_gfx.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_gfx.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_gfx.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_gfx.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_gfx.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_gfx.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_gfx.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_gfx.o: ../depend_stubs/OgreFont.h
lua_wrappers_gfx.o: ../src/physics/PhysicsWorld.h
lua_wrappers_gfx.o: ../src/CentralisedLog.h ../src/SharedPtr.h
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
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_gfx.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_gfx.o: ../src/physics/CollisionMesh.h
lua_wrappers_gfx.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_gfx.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_gfx.o: ../src/physics/BColParser.h
lua_wrappers_gfx.o: ../depend_stubs/stdint.h
lua_wrappers_gfx.o: ../src/physics/LooseEnd.h
lua_wrappers_gfx.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_gfx.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_gfx.o: ../src/ExternalTable.h ../src/path_util.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_gfx.o: ../src/lua_wrappers_common.h ../src/gfx/gfx.h
lua_wrappers_gfx.o: ../src/vect_util.h ../src/gfx/GfxBody.h
lua_wrappers_gfx.o: ../src/gfx/GfxMaterial.h
lua_wrappers_gfx.o: ../src/gfx/gfx_internal.h
lua_wrappers_gfx.o: ../depend_stubs/Ogre.h
lua_wrappers_gfx.o: ../depend_stubs/OgreFontManager.h
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
lua_wrappers_gfx.o: ../src/gfx/gfx_option.h
lua_wrappers_gfx.o: ../src/gfx/GfxSkyBody.h
lua_wrappers_gfx.o: ../depend_stubs/OgreSceneNode.h
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
lua_wrappers_gfx.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_gfx.o: ../src/gfx/gfx_hud.h ../src/lua_util.h
lua_wrappers_gfx.o: ../src/gfx/GfxPipeline.h ../src/gfx/GfxFont.h
lua_wrappers_gfx.o: ../src/gfx/GfxTextBuffer.h
lua_wrappers_gfx.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_gfx.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_gfx.o: ../src/lua_wrappers_primitives.h
lua_wrappers_gfx.o: ../src/lua_wrappers_common.h
lua_wrappers_gfx.o: ../depend_stubs/OgreException.h
lua_wrappers_gfx.o: ../src/lua_util.h ../depend_stubs/limits
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_scnmgr.h
lua_wrappers_gfx.o: ../depend_stubs/OgrePixelFormat.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_mobj.h
lua_wrappers_gfx.o: ../depend_stubs/OgreLight.h
lua_wrappers_gfx.o: ../depend_stubs/OgreHighLevelGpuProgramManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreTextureManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreSkeletonManager.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGpuProgramManager.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_gfx.o: ../depend_stubs/OgreCommon.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_mesh.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_gpuprog.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGpuProgram.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_tex.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_render.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_hud.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreVector3.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreVector4.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreCgProgram.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreGLSLProgram.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreGLSLGpuProgram.h
lua_wrappers_gpuprog.o: ../src/gfx/lua_wrappers_gpuprog.h
lua_wrappers_gpuprog.o: ../depend_stubs/OgreGpuProgram.h
lua_wrappers_gpuprog.o: ../src/lua_wrappers_common.h
lua_wrappers_hud.o: ../src/gfx/HUD.h ../depend_stubs/vector
lua_wrappers_hud.o: ../depend_stubs/climits
lua_wrappers_hud.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_hud.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_hud.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_hud.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_hud.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_hud.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_hud.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_hud.o: ../depend_stubs/OgreFont.h
lua_wrappers_hud.o: ../src/gfx/lua_wrappers_hud.h
lua_wrappers_hud.o: ../src/lua_wrappers_common.h
lua_wrappers_hud.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_hud.o: ../depend_stubs/OgreCommon.h
lua_wrappers_material.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_material.o: ../depend_stubs/OgreTechnique.h
lua_wrappers_material.o: ../depend_stubs/OgrePass.h
lua_wrappers_material.o: ../depend_stubs/OgreTextureUnitState.h
lua_wrappers_material.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_material.o: ../depend_stubs/OgreCommon.h
lua_wrappers_material.o: ../src/lua_wrappers_common.h
lua_wrappers_mesh.o: ../depend_stubs/OgreSubMesh.h
lua_wrappers_mesh.o: ../depend_stubs/OgreMaterialManager.h
lua_wrappers_mesh.o: ../src/gfx/lua_wrappers_mesh.h
lua_wrappers_mesh.o: ../depend_stubs/OgreMesh.h
lua_wrappers_mesh.o: ../src/lua_wrappers_common.h
lua_wrappers_mesh.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_mesh.o: ../depend_stubs/OgreCommon.h
lua_wrappers_mobj.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_mobj.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_mobj.o: ../depend_stubs/OgreEntity.h
lua_wrappers_mobj.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_mobj.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_mobj.o: ../depend_stubs/OgreCamera.h
lua_wrappers_mobj.o: ../depend_stubs/OgreMaterialManager.h
lua_wrappers_mobj.o: ../depend_stubs/OgreSubEntity.h
lua_wrappers_mobj.o: ../depend_stubs/OgreMeshManager.h
lua_wrappers_mobj.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
lua_wrappers_mobj.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_mobj.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_mobj.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
lua_wrappers_mobj.o: ../depend_stubs/cmath ../src/CentralisedLog.h
lua_wrappers_mobj.o: ../depend_stubs/iostream
lua_wrappers_mobj.o: ../depend_stubs/sstream
lua_wrappers_mobj.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_mobj.o: ../src/console_colour.h
lua_wrappers_mobj.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_mobj.o: ../depend_stubs/map ../depend_stubs/vector
lua_wrappers_mobj.o: ../depend_stubs/algorithm
lua_wrappers_mobj.o: ../src/SSEAllocator.h ../src/Streamer.h
lua_wrappers_mobj.o: ../depend_stubs/lua.h
lua_wrappers_mobj.o: ../depend_stubs/lauxlib.h
lua_wrappers_mobj.o: ../depend_stubs/lualib.h ../src/GritObject.h
lua_wrappers_mobj.o: ../depend_stubs/set ../depend_stubs/string
lua_wrappers_mobj.o: ../src/SharedPtr.h ../depend_stubs/cstdlib
lua_wrappers_mobj.o: ../depend_stubs/functional ../src/Streamer.h
lua_wrappers_mobj.o: ../src/ExternalTable.h ../src/math_util.h
lua_wrappers_mobj.o: ../src/LuaPtr.h ../src/SplineTable.h
lua_wrappers_mobj.o: ../src/BackgroundLoader.h ../src/vect_util.h
lua_wrappers_mobj.o: ../depend_stubs/list
lua_wrappers_mobj.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_mobj.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_mobj.o: ../src/DiskResource.h ../src/main.h
lua_wrappers_mobj.o: ../src/Mouse.h ../src/Keyboard.h
lua_wrappers_mobj.o: ../src/BulletDebugDrawer.h
lua_wrappers_mobj.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_mobj.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_mobj.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_mobj.o: ../src/gfx/HUD.h ../depend_stubs/climits
lua_wrappers_mobj.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_mobj.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_mobj.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_mobj.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_mobj.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_mobj.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_mobj.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_mobj.o: ../depend_stubs/OgreFont.h
lua_wrappers_mobj.o: ../src/physics/PhysicsWorld.h
lua_wrappers_mobj.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_mobj.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_mobj.o: ../src/physics/TColParser.h
lua_wrappers_mobj.o: ../src/physics/TColLexer
lua_wrappers_mobj.o: ../depend_stubs/istream
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_mobj.o: ../src/physics/TColLexer-token_ids
lua_wrappers_mobj.o: ../depend_stubs/cstdio
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_mobj.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_mobj.o: ../src/physics/CollisionMesh.h
lua_wrappers_mobj.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_mobj.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_mobj.o: ../src/physics/BColParser.h
lua_wrappers_mobj.o: ../depend_stubs/stdint.h
lua_wrappers_mobj.o: ../src/physics/LooseEnd.h
lua_wrappers_mobj.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_mobj.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_mobj.o: ../src/gfx/gfx.h ../src/vect_util.h
lua_wrappers_mobj.o: ../src/gfx/lua_wrappers_mobj.h
lua_wrappers_mobj.o: ../src/lua_wrappers_common.h
lua_wrappers_mobj.o: ../depend_stubs/OgreLight.h
lua_wrappers_mobj.o: ../src/gfx/lua_wrappers_scnmgr.h
lua_wrappers_mobj.o: ../depend_stubs/OgrePixelFormat.h
lua_wrappers_mobj.o: ../src/lua_wrappers_primitives.h
lua_wrappers_mobj.o: ../src/lua_wrappers_common.h
lua_wrappers_mobj.o: ../depend_stubs/OgreException.h
lua_wrappers_mobj.o: ../src/lua_util.h ../depend_stubs/limits
lua_wrappers_mobj.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_mobj.o: ../depend_stubs/OgreCommon.h
lua_wrappers_mobj.o: ../src/gfx/lua_wrappers_mesh.h
lua_wrappers_mobj.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_mobj.o: ../src/gfx/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_render.o: ../depend_stubs/OgreViewport.h
lua_wrappers_render.o: ../depend_stubs/OgreRenderTarget.h
lua_wrappers_render.o: ../depend_stubs/OgreRenderWindow.h
lua_wrappers_render.o: ../depend_stubs/OgreRenderTexture.h
lua_wrappers_render.o: ../depend_stubs/OgreCompositorManager.h
lua_wrappers_render.o: ../src/main.h ../depend_stubs/lua.h
lua_wrappers_render.o: ../src/Mouse.h ../depend_stubs/vector
lua_wrappers_render.o: ../src/Keyboard.h ../depend_stubs/string
lua_wrappers_render.o: ../src/BackgroundLoader.h
lua_wrappers_render.o: ../src/vect_util.h
lua_wrappers_render.o: ../depend_stubs/algorithm
lua_wrappers_render.o: ../src/CentralisedLog.h
lua_wrappers_render.o: ../depend_stubs/iostream
lua_wrappers_render.o: ../depend_stubs/sstream
lua_wrappers_render.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_render.o: ../src/console_colour.h
lua_wrappers_render.o: ../depend_stubs/list
lua_wrappers_render.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_render.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_render.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_render.o: ../src/BulletDebugDrawer.h
lua_wrappers_render.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_render.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_render.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_render.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_render.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_render.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_render.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
lua_wrappers_render.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_render.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_render.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_render.o: ../depend_stubs/OgreMesh.h
lua_wrappers_render.o: ../src/math_util.h ../depend_stubs/cmath
lua_wrappers_render.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_render.o: ../depend_stubs/map ../src/SSEAllocator.h
lua_wrappers_render.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
lua_wrappers_render.o: ../depend_stubs/lualib.h
lua_wrappers_render.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_render.o: ../depend_stubs/cstdlib
lua_wrappers_render.o: ../depend_stubs/functional
lua_wrappers_render.o: ../src/Streamer.h ../src/ExternalTable.h
lua_wrappers_render.o: ../src/math_util.h ../src/LuaPtr.h
lua_wrappers_render.o: ../src/SplineTable.h ../src/gfx/HUD.h
lua_wrappers_render.o: ../depend_stubs/climits
lua_wrappers_render.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_render.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_render.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_render.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_render.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_render.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_render.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_render.o: ../depend_stubs/OgreFont.h
lua_wrappers_render.o: ../src/physics/PhysicsWorld.h
lua_wrappers_render.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_render.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_render.o: ../src/physics/TColParser.h
lua_wrappers_render.o: ../src/physics/TColLexer
lua_wrappers_render.o: ../depend_stubs/istream
lua_wrappers_render.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_render.o: ../src/physics/TColLexer-token_ids
lua_wrappers_render.o: ../depend_stubs/cstdio
lua_wrappers_render.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_render.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_render.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_render.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_render.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_render.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_render.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_render.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_render.o: ../src/physics/CollisionMesh.h
lua_wrappers_render.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_render.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_render.o: ../src/physics/BColParser.h
lua_wrappers_render.o: ../depend_stubs/stdint.h
lua_wrappers_render.o: ../src/physics/LooseEnd.h
lua_wrappers_render.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_render.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_render.o: ../src/gfx/gfx.h ../src/vect_util.h
lua_wrappers_render.o: ../src/gfx/lua_wrappers_render.h
lua_wrappers_render.o: ../src/lua_wrappers_common.h
lua_wrappers_render.o: ../src/gfx/lua_wrappers_tex.h
lua_wrappers_render.o: ../depend_stubs/OgreTexture.h
lua_wrappers_render.o: ../src/gfx/lua_wrappers_mobj.h
lua_wrappers_render.o: ../depend_stubs/OgreLight.h
lua_wrappers_render.o: ../src/gfx/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreSceneManager.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreEntity.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreShadowCameraSetup.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreShadowCameraSetupFocused.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreShadowCameraSetupLiSPSM.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreShadowCameraSetupPSSM.h
lua_wrappers_scnmgr.o: ../src/main.h ../depend_stubs/lua.h
lua_wrappers_scnmgr.o: ../src/Mouse.h ../depend_stubs/vector
lua_wrappers_scnmgr.o: ../src/Keyboard.h ../depend_stubs/string
lua_wrappers_scnmgr.o: ../src/BackgroundLoader.h
lua_wrappers_scnmgr.o: ../src/vect_util.h
lua_wrappers_scnmgr.o: ../depend_stubs/algorithm
lua_wrappers_scnmgr.o: ../src/CentralisedLog.h
lua_wrappers_scnmgr.o: ../depend_stubs/iostream
lua_wrappers_scnmgr.o: ../depend_stubs/sstream
lua_wrappers_scnmgr.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_scnmgr.o: ../src/console_colour.h
lua_wrappers_scnmgr.o: ../depend_stubs/list
lua_wrappers_scnmgr.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_scnmgr.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_scnmgr.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_scnmgr.o: ../src/BulletDebugDrawer.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_scnmgr.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_scnmgr.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_scnmgr.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
lua_wrappers_scnmgr.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreMesh.h
lua_wrappers_scnmgr.o: ../src/math_util.h ../depend_stubs/cmath
lua_wrappers_scnmgr.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_scnmgr.o: ../depend_stubs/map ../src/SSEAllocator.h
lua_wrappers_scnmgr.o: ../src/Streamer.h ../depend_stubs/lauxlib.h
lua_wrappers_scnmgr.o: ../depend_stubs/lualib.h
lua_wrappers_scnmgr.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_scnmgr.o: ../depend_stubs/cstdlib
lua_wrappers_scnmgr.o: ../depend_stubs/functional
lua_wrappers_scnmgr.o: ../src/Streamer.h ../src/ExternalTable.h
lua_wrappers_scnmgr.o: ../src/math_util.h ../src/LuaPtr.h
lua_wrappers_scnmgr.o: ../src/SplineTable.h ../src/gfx/HUD.h
lua_wrappers_scnmgr.o: ../depend_stubs/climits
lua_wrappers_scnmgr.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_scnmgr.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreFont.h
lua_wrappers_scnmgr.o: ../src/physics/PhysicsWorld.h
lua_wrappers_scnmgr.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_scnmgr.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_scnmgr.o: ../src/physics/TColParser.h
lua_wrappers_scnmgr.o: ../src/physics/TColLexer
lua_wrappers_scnmgr.o: ../depend_stubs/istream
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_scnmgr.o: ../src/physics/TColLexer-token_ids
lua_wrappers_scnmgr.o: ../depend_stubs/cstdio
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_scnmgr.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_scnmgr.o: ../src/physics/CollisionMesh.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_scnmgr.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_scnmgr.o: ../src/physics/BColParser.h
lua_wrappers_scnmgr.o: ../depend_stubs/stdint.h
lua_wrappers_scnmgr.o: ../src/physics/LooseEnd.h
lua_wrappers_scnmgr.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_scnmgr.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_scnmgr.o: ../src/gfx/gfx.h ../src/vect_util.h
lua_wrappers_scnmgr.o: ../src/gfx/lua_wrappers_scnmgr.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgrePixelFormat.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_common.h
lua_wrappers_scnmgr.o: ../src/gfx/lua_wrappers_mobj.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreLight.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_primitives.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_common.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreException.h
lua_wrappers_scnmgr.o: ../src/lua_util.h ../depend_stubs/limits
lua_wrappers_scnmgr.o: ../src/gfx/lua_wrappers_material.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreCommon.h
lua_wrappers_scnmgr.o: ../src/gfx/lua_wrappers_tex.h
lua_wrappers_scnmgr.o: ../depend_stubs/OgreTexture.h
lua_wrappers_scnmgr.o: ../src/gfx/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_tex.o: ../depend_stubs/OgreTexture.h
lua_wrappers_tex.o: ../depend_stubs/OgreHardwarePixelBuffer.h
lua_wrappers_tex.o: ../src/gfx/lua_wrappers_tex.h
lua_wrappers_tex.o: ../src/lua_wrappers_common.h
lua_wrappers_tex.o: ../src/gfx/lua_wrappers_render.h
lua_wrappers_tex.o: ../src/gfx/lua_wrappers_scnmgr.h
lua_wrappers_tex.o: ../depend_stubs/OgrePixelFormat.h
ray.o: ../depend_stubs/float.h ../src/gfx/ray.h
ray.o: ../depend_stubs/Ogre.h ../depend_stubs/OgreSceneQuery.h
BColParser.o: ../depend_stubs/cmath ../depend_stubs/sstream
BColParser.o: ../src/portable_io.h ../depend_stubs/cstdlib
BColParser.o: ../depend_stubs/cstdio ../depend_stubs/cassert
BColParser.o: ../depend_stubs/cstring ../depend_stubs/string
BColParser.o: ../depend_stubs/iostream ../depend_stubs/fstream
BColParser.o: ../depend_stubs/stdint.h ../src/CentralisedLog.h
BColParser.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
BColParser.o: ../src/console_colour.h
BColParser.o: ../src/physics/col_defaults.h
BColParser.o: ../src/physics/BColParser.h
BColParser.o: ../depend_stubs/vector
BColParser.o: ../src/physics/TColParser.h ../src/math_util.h
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
BColParser.o: ../depend_stubs/quex/code_base/TokenQueue
BColParser.o: ../depend_stubs/quex/code_base/template/IncludeStack
BColParser.o: ../depend_stubs/quex/code_base/temporary_macros_on
BColParser.o: ../depend_stubs/quex/code_base/temporary_macros_off
BColParser.o: ../depend_stubs/quex/code_base/template/constructor.i
BColParser.o: ../depend_stubs/quex/code_base/template/Counter.i
BColParser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
BColParser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
BColParser.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
BColParser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
BColParser.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
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
CollisionMesh.o: ../src/DiskResource.h ../depend_stubs/set
CollisionMesh.o: ../src/sleep.h ../src/SharedPtr.h
CollisionMesh.o: ../src/physics/TColParser.h
CollisionMesh.o: ../src/math_util.h ../depend_stubs/cmath
CollisionMesh.o: ../depend_stubs/cfloat
CollisionMesh.o: ../src/CentralisedLog.h
CollisionMesh.o: ../depend_stubs/sstream
CollisionMesh.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
CollisionMesh.o: ../src/console_colour.h
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
CollisionMesh.o: ../depend_stubs/quex/code_base/TokenQueue
CollisionMesh.o: ../depend_stubs/quex/code_base/template/IncludeStack
CollisionMesh.o: ../depend_stubs/quex/code_base/temporary_macros_on
CollisionMesh.o: ../depend_stubs/quex/code_base/temporary_macros_off
CollisionMesh.o: ../depend_stubs/quex/code_base/template/constructor.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Counter.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/Accumulator.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/mode_handling.i
CollisionMesh.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
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
PhysicsWorld.o: ../depend_stubs/cstdlib
PhysicsWorld.o: ../depend_stubs/functional ../src/main.h
PhysicsWorld.o: ../depend_stubs/lua.h ../src/Mouse.h
PhysicsWorld.o: ../src/Keyboard.h ../src/BackgroundLoader.h
PhysicsWorld.o: ../src/vect_util.h ../depend_stubs/algorithm
PhysicsWorld.o: ../src/CentralisedLog.h
PhysicsWorld.o: ../depend_stubs/iostream
PhysicsWorld.o: ../depend_stubs/sstream
PhysicsWorld.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
PhysicsWorld.o: ../src/console_colour.h ../depend_stubs/list
PhysicsWorld.o: ../depend_stubs/boost/thread/thread.hpp
PhysicsWorld.o: ../depend_stubs/boost/thread/condition.hpp
PhysicsWorld.o: ../src/DiskResource.h
PhysicsWorld.o: ../src/BulletDebugDrawer.h
PhysicsWorld.o: ../depend_stubs/OgreFrameListener.h
PhysicsWorld.o: ../depend_stubs/OgreManualObject.h
PhysicsWorld.o: ../depend_stubs/btBulletCollisionCommon.h
PhysicsWorld.o: ../src/gfx/lua_userdata_dependency_tracker.h
PhysicsWorld.o: ../depend_stubs/OgreStaticGeometry.h
PhysicsWorld.o: ../depend_stubs/OgreInstancedGeometry.h
PhysicsWorld.o: ../src/gfx/Clutter.h ../depend_stubs/cfloat
PhysicsWorld.o: ../depend_stubs/OgreMovableObject.h
PhysicsWorld.o: ../depend_stubs/OgreVertexIndexData.h
PhysicsWorld.o: ../depend_stubs/OgreMaterial.h
PhysicsWorld.o: ../depend_stubs/OgreMesh.h ../src/math_util.h
PhysicsWorld.o: ../depend_stubs/cmath
PhysicsWorld.o: ../src/CacheFriendlyRangeSpaceSIMD.h
PhysicsWorld.o: ../src/SSEAllocator.h ../src/Streamer.h
PhysicsWorld.o: ../depend_stubs/lauxlib.h
PhysicsWorld.o: ../depend_stubs/lualib.h ../src/GritObject.h
PhysicsWorld.o: ../src/Streamer.h ../src/ExternalTable.h
PhysicsWorld.o: ../src/math_util.h ../src/LuaPtr.h
PhysicsWorld.o: ../src/SplineTable.h ../src/gfx/HUD.h
PhysicsWorld.o: ../depend_stubs/climits
PhysicsWorld.o: ../depend_stubs/OgrePanelOverlayElement.h
PhysicsWorld.o: ../depend_stubs/OgreOverlayElementFactory.h
PhysicsWorld.o: ../depend_stubs/OgreOverlay.h
PhysicsWorld.o: ../depend_stubs/OgreOverlayManager.h
PhysicsWorld.o: ../src/gfx/TextListOverlayElement.h
PhysicsWorld.o: ../depend_stubs/OgrePrerequisites.h
PhysicsWorld.o: ../depend_stubs/OgreOverlayElement.h
PhysicsWorld.o: ../depend_stubs/OgreFont.h
PhysicsWorld.o: ../src/physics/PhysicsWorld.h
PhysicsWorld.o: ../src/CentralisedLog.h ../src/SharedPtr.h
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
PhysicsWorld.o: ../depend_stubs/quex/code_base/TokenQueue
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/IncludeStack
PhysicsWorld.o: ../depend_stubs/quex/code_base/temporary_macros_on
PhysicsWorld.o: ../depend_stubs/quex/code_base/temporary_macros_off
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/constructor.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Counter.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Accumulator.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/mode_handling.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/misc.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/buffer_access.i
PhysicsWorld.o: ../depend_stubs/quex/code_base/template/Analyser.i
PhysicsWorld.o: ../src/physics/CollisionMesh.h
PhysicsWorld.o: ../depend_stubs/OgreDataStream.h
PhysicsWorld.o: ../src/DiskResource.h ../src/sleep.h
PhysicsWorld.o: ../src/physics/BColParser.h
PhysicsWorld.o: ../depend_stubs/stdint.h
PhysicsWorld.o: ../src/physics/LooseEnd.h
PhysicsWorld.o: ../src/physics/PhysicalMaterial.h
PhysicsWorld.o: ../src/LuaPtr.h ../src/option.h
PhysicsWorld.o: ../depend_stubs/ostream ../src/lua_util.h
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
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/TokenQueue
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/IncludeStack
TColLexer-core-engine.o: ../depend_stubs/cstdlib
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/temporary_macros_on
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/temporary_macros_off
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/constructor.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Counter.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/Accumulator.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/mode_handling.i
TColLexer-core-engine.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
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
TColLexer.o: ../depend_stubs/quex/code_base/TokenQueue
TColLexer.o: ../depend_stubs/quex/code_base/template/IncludeStack
TColLexer.o: ../depend_stubs/cstdlib
TColLexer.o: ../depend_stubs/quex/code_base/temporary_macros_on
TColLexer.o: ../depend_stubs/quex/code_base/temporary_macros_off
TColLexer.o: ../depend_stubs/quex/code_base/template/constructor.i
TColLexer.o: ../depend_stubs/quex/code_base/template/Counter.i
TColLexer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
TColLexer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
TColLexer.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
TColLexer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
TColLexer.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
TColLexer.o: ../depend_stubs/quex/code_base/template/misc.i
TColLexer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
TColLexer.o: ../depend_stubs/quex/code_base/template/Analyser.i
TColParser.o: ../depend_stubs/iostream ../depend_stubs/cstdlib
TColParser.o: ../src/math_util.h ../depend_stubs/cmath
TColParser.o: ../depend_stubs/cfloat ../src/CentralisedLog.h
TColParser.o: ../depend_stubs/sstream
TColParser.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
TColParser.o: ../src/console_colour.h ../src/path_util.h
TColParser.o: ../depend_stubs/vector ../depend_stubs/string
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
TColParser.o: ../depend_stubs/quex/code_base/TokenQueue
TColParser.o: ../depend_stubs/quex/code_base/template/IncludeStack
TColParser.o: ../depend_stubs/quex/code_base/temporary_macros_on
TColParser.o: ../depend_stubs/quex/code_base/temporary_macros_off
TColParser.o: ../depend_stubs/quex/code_base/template/constructor.i
TColParser.o: ../depend_stubs/quex/code_base/template/Counter.i
TColParser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
TColParser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
TColParser.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
TColParser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
TColParser.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
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
grit_col_conv.o: ../src/console_colour.h
grit_col_conv.o: ../src/physics/TColParser.h
grit_col_conv.o: ../src/math_util.h ../depend_stubs/cmath
grit_col_conv.o: ../depend_stubs/cfloat
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
grit_col_conv.o: ../depend_stubs/quex/code_base/TokenQueue
grit_col_conv.o: ../depend_stubs/quex/code_base/template/IncludeStack
grit_col_conv.o: ../depend_stubs/quex/code_base/temporary_macros_on
grit_col_conv.o: ../depend_stubs/quex/code_base/temporary_macros_off
grit_col_conv.o: ../depend_stubs/quex/code_base/template/constructor.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Counter.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Accumulator.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/mode_handling.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/misc.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/buffer_access.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Analyser.i
grit_col_conv.o: ../src/physics/BColParser.h
lua_wrappers_physics.o: ../depend_stubs/limits
lua_wrappers_physics.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_physics.o: ../src/SharedPtr.h ../src/Streamer.h
lua_wrappers_physics.o: ../depend_stubs/map
lua_wrappers_physics.o: ../depend_stubs/lua.h
lua_wrappers_physics.o: ../depend_stubs/lauxlib.h
lua_wrappers_physics.o: ../depend_stubs/lualib.h
lua_wrappers_physics.o: ../src/GritObject.h
lua_wrappers_physics.o: ../depend_stubs/vector
lua_wrappers_physics.o: ../depend_stubs/set
lua_wrappers_physics.o: ../depend_stubs/string
lua_wrappers_physics.o: ../src/SharedPtr.h
lua_wrappers_physics.o: ../depend_stubs/cstdlib
lua_wrappers_physics.o: ../depend_stubs/functional
lua_wrappers_physics.o: ../src/Streamer.h
lua_wrappers_physics.o: ../src/ExternalTable.h
lua_wrappers_physics.o: ../src/math_util.h ../src/LuaPtr.h
lua_wrappers_physics.o: ../src/SplineTable.h
lua_wrappers_physics.o: ../src/BackgroundLoader.h
lua_wrappers_physics.o: ../src/vect_util.h
lua_wrappers_physics.o: ../depend_stubs/algorithm
lua_wrappers_physics.o: ../src/CentralisedLog.h
lua_wrappers_physics.o: ../depend_stubs/iostream
lua_wrappers_physics.o: ../depend_stubs/sstream
lua_wrappers_physics.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
lua_wrappers_physics.o: ../src/console_colour.h
lua_wrappers_physics.o: ../depend_stubs/list
lua_wrappers_physics.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_physics.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_physics.o: ../src/DiskResource.h
lua_wrappers_physics.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_physics.o: ../depend_stubs/cmath
lua_wrappers_physics.o: ../src/SSEAllocator.h
lua_wrappers_physics.o: ../src/gfx/GfxInstances.h
lua_wrappers_physics.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_physics.o: ../src/DenseIndexMap.h
lua_wrappers_physics.o: ../src/gfx/GfxBody.h
lua_wrappers_physics.o: ../depend_stubs/OgreMesh.h
lua_wrappers_physics.o: ../src/gfx/gfx.h ../src/vect_util.h
lua_wrappers_physics.o: ../src/gfx/GfxMaterial.h
lua_wrappers_physics.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_physics.o: ../src/gfx/gfx_internal.h
lua_wrappers_physics.o: ../depend_stubs/Ogre.h
lua_wrappers_physics.o: ../depend_stubs/OgreFontManager.h
lua_wrappers_physics.o: ../depend_stubs/OgreMeshManager.h
lua_wrappers_physics.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_physics.o: ../depend_stubs/OgreOctreeSceneManager.h
lua_wrappers_physics.o: ../depend_stubs/OgreVector3.h
lua_wrappers_physics.o: ../depend_stubs/OgreQuaternion.h
lua_wrappers_physics.o: ../depend_stubs/OgreCustomCompositionPass.h
lua_wrappers_physics.o: ../depend_stubs/OgreCompositor.h
lua_wrappers_physics.o: ../depend_stubs/OgreOctreePlugin.h
lua_wrappers_physics.o: ../depend_stubs/OgreCgPlugin.h
lua_wrappers_physics.o: ../depend_stubs/OgreRenderSystem.h
lua_wrappers_physics.o: ../depend_stubs/OgreGLRenderSystem.h
lua_wrappers_physics.o: ../src/gfx/gfx_option.h
lua_wrappers_physics.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_physics.o: ../src/lua_wrappers_common.h
lua_wrappers_physics.o: ../depend_stubs/OgreException.h
lua_wrappers_physics.o: ../src/lua_util.h ../src/main.h
lua_wrappers_physics.o: ../src/Mouse.h ../src/Keyboard.h
lua_wrappers_physics.o: ../src/BulletDebugDrawer.h
lua_wrappers_physics.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_physics.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_physics.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_physics.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_physics.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_physics.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_physics.o: ../src/gfx/Clutter.h
lua_wrappers_physics.o: ../depend_stubs/cfloat
lua_wrappers_physics.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_physics.o: ../src/math_util.h ../src/gfx/HUD.h
lua_wrappers_physics.o: ../depend_stubs/climits
lua_wrappers_physics.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_physics.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_physics.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_physics.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_physics.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_physics.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_physics.o: ../depend_stubs/OgreFont.h
lua_wrappers_physics.o: ../src/physics/PhysicsWorld.h
lua_wrappers_physics.o: ../src/CentralisedLog.h
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
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_physics.o: ../src/physics/CollisionMesh.h
lua_wrappers_physics.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_physics.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_physics.o: ../src/physics/BColParser.h
lua_wrappers_physics.o: ../depend_stubs/stdint.h
lua_wrappers_physics.o: ../src/physics/LooseEnd.h
lua_wrappers_physics.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_physics.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_physics.o: ../src/path_util.h
lua_wrappers_physics.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_physics.o: ../src/gfx/gfx.h
lua_wrappers_physics.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_physics.o: ../depend_stubs/OgreResource.h
lua_wrappers_physics.o: ../depend_stubs/OgreTexture.h
lua_wrappers_physics.o: ../src/BackgroundLoader.h
lua_wrappers_physics.o: ../src/gfx/GfxParticleSystem.h
lua_wrappers_physics.o: ../depend_stubs/utility
lua_wrappers_physics.o: ../src/gfx/HUD.h
lua_wrappers_physics.o: ../depend_stubs/OgreRoot.h
lua_wrappers_physics.o: ../depend_stubs/OgreRenderWindow.h
lua_wrappers_physics.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_physics.o: ../src/lua_wrappers_common.h
lua_wrappers_physics.o: ../src/gfx/GfxSkyBody.h
lua_wrappers_physics.o: ../depend_stubs/OgreSceneNode.h
lua_wrappers_physics.o: ../src/gfx/GfxSkyMaterial.h
lua_wrappers_physics.o: ../src/gfx/GfxLight.h
lua_wrappers_physics.o: ../src/gfx/GfxRangedInstances.h
lua_wrappers_physics.o: ../src/gfx/gfx_hud.h ../src/lua_util.h
lua_wrappers_physics.o: ../src/gfx/GfxPipeline.h
lua_wrappers_physics.o: ../src/gfx/GfxFont.h
lua_wrappers_physics.o: ../src/gfx/GfxTextBuffer.h
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
AudioDiskResource.o: ../src/console_colour.h
AudioDiskResource.o: ../depend_stubs/OgreResourceGroupManager.h
AudioDiskResource.o: ../src/audio/audio.h ../src/SharedPtr.h
AudioDiskResource.o: ../src/math_util.h ../depend_stubs/cmath
AudioDiskResource.o: ../depend_stubs/cfloat
AudioDiskResource.o: ../src/DiskResource.h ../depend_stubs/set
AudioDiskResource.o: ../src/portable_io.h
AudioDiskResource.o: ../depend_stubs/cstdlib
AudioDiskResource.o: ../depend_stubs/cstdio
AudioDiskResource.o: ../depend_stubs/cassert
AudioDiskResource.o: ../depend_stubs/cstring
AudioDiskResource.o: ../depend_stubs/string
AudioDiskResource.o: ../depend_stubs/fstream
AudioDiskResource.o: ../depend_stubs/stdint.h
audio.o: ../depend_stubs/AL/al.h ../depend_stubs/AL/alc.h
audio.o: ../src/CentralisedLog.h ../src/vect_util.h
audio.o: ../depend_stubs/vector ../depend_stubs/algorithm
audio.o: ../src/CentralisedLog.h ../depend_stubs/iostream
audio.o: ../depend_stubs/sstream
audio.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
audio.o: ../src/console_colour.h ../src/option.h
audio.o: ../depend_stubs/string ../depend_stubs/ostream
audio.o: ../src/audio/audio.h ../src/SharedPtr.h
audio.o: ../src/math_util.h ../depend_stubs/cmath
audio.o: ../depend_stubs/cfloat ../src/BackgroundLoader.h
audio.o: ../src/vect_util.h ../src/DiskResource.h
audio.o: ../depend_stubs/set ../src/audio/AudioDiskResource.h
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
lua_wrappers_audio.o: ../src/console_colour.h
lua_wrappers_audio.o: ../depend_stubs/list
lua_wrappers_audio.o: ../depend_stubs/boost/thread/thread.hpp
lua_wrappers_audio.o: ../depend_stubs/boost/thread/condition.hpp
lua_wrappers_audio.o: ../src/DiskResource.h ../depend_stubs/set
lua_wrappers_audio.o: ../src/BulletDebugDrawer.h
lua_wrappers_audio.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_audio.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_audio.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_audio.o: ../src/gfx/lua_userdata_dependency_tracker.h
lua_wrappers_audio.o: ../depend_stubs/OgreStaticGeometry.h
lua_wrappers_audio.o: ../depend_stubs/OgreInstancedGeometry.h
lua_wrappers_audio.o: ../src/gfx/Clutter.h
lua_wrappers_audio.o: ../depend_stubs/cfloat
lua_wrappers_audio.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_audio.o: ../depend_stubs/OgreVertexIndexData.h
lua_wrappers_audio.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_audio.o: ../depend_stubs/OgreMesh.h
lua_wrappers_audio.o: ../src/math_util.h ../depend_stubs/cmath
lua_wrappers_audio.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_audio.o: ../depend_stubs/map ../src/SSEAllocator.h
lua_wrappers_audio.o: ../src/Streamer.h
lua_wrappers_audio.o: ../depend_stubs/lauxlib.h
lua_wrappers_audio.o: ../depend_stubs/lualib.h
lua_wrappers_audio.o: ../src/GritObject.h ../src/SharedPtr.h
lua_wrappers_audio.o: ../depend_stubs/cstdlib
lua_wrappers_audio.o: ../depend_stubs/functional
lua_wrappers_audio.o: ../src/Streamer.h ../src/ExternalTable.h
lua_wrappers_audio.o: ../src/math_util.h ../src/LuaPtr.h
lua_wrappers_audio.o: ../src/SplineTable.h ../src/gfx/HUD.h
lua_wrappers_audio.o: ../depend_stubs/climits
lua_wrappers_audio.o: ../depend_stubs/OgrePanelOverlayElement.h
lua_wrappers_audio.o: ../depend_stubs/OgreOverlayElementFactory.h
lua_wrappers_audio.o: ../depend_stubs/OgreOverlay.h
lua_wrappers_audio.o: ../depend_stubs/OgreOverlayManager.h
lua_wrappers_audio.o: ../src/gfx/TextListOverlayElement.h
lua_wrappers_audio.o: ../depend_stubs/OgrePrerequisites.h
lua_wrappers_audio.o: ../depend_stubs/OgreOverlayElement.h
lua_wrappers_audio.o: ../depend_stubs/OgreFont.h
lua_wrappers_audio.o: ../src/physics/PhysicsWorld.h
lua_wrappers_audio.o: ../src/CentralisedLog.h ../src/SharedPtr.h
lua_wrappers_audio.o: ../depend_stubs/btBulletDynamicsCommon.h
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
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/TokenQueue
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_audio.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_audio.o: ../src/physics/CollisionMesh.h
lua_wrappers_audio.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_audio.o: ../src/DiskResource.h ../src/sleep.h
lua_wrappers_audio.o: ../src/physics/BColParser.h
lua_wrappers_audio.o: ../depend_stubs/stdint.h
lua_wrappers_audio.o: ../src/physics/LooseEnd.h
lua_wrappers_audio.o: ../src/physics/PhysicalMaterial.h
lua_wrappers_audio.o: ../src/GritObject.h ../src/LuaPtr.h
lua_wrappers_audio.o: ../src/ExternalTable.h ../src/path_util.h
lua_wrappers_audio.o: ../src/audio/audio.h
lua_wrappers_audio.o: ../depend_stubs/AL/al.h
lua_wrappers_audio.o: ../src/BackgroundLoader.h
lua_wrappers_audio.o: ../src/audio/AudioDiskResource.h
lua_wrappers_audio.o: ../depend_stubs/OgreResourceGroupManager.h
lua_wrappers_audio.o: ../src/audio/lua_wrappers_audio.h
lua_wrappers_audio.o: ../src/lua_wrappers_common.h
