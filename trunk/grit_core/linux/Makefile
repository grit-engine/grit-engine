# (c) David Cunningham and the Grit Game Engine project 2012
# Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

# this Makefile is just used to set up some variables, the muscle is in
# ../src/common.mk which is used by both linux and win32 builds

ROOT=../..

include $(ROOT)/common.mk

TARGETS=$(ROOT)/grit_core/media/Grit.$(GRIT_EXEC_SUFFIX) \
        $(ROOT)/grit_core/media/Tools/gsl.$(GRIT_EXEC_SUFFIX) \
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
	rm Makefile.bak


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
tcol_lexer-core-engine.o: ../src/physics/tcol_lexer-core-engine.cpp
tcol_lexer.o: ../src/physics/tcol_lexer.cpp
	@$(COMPILING)
	@$(CXX) -c $< -o $@ $(CXXFLAGS)


GSL_OBJ = gfx_gasoline.o \
          gfx_gasoline_parser.o \
          gfx_gasoline_type_system.o \
          gfx_gasoline_backend.o \
          gfx_gasoline_backend_gsl.o \
          gfx_gasoline_backend_cg.o \
          gfx_gasoline_backend_glsl.o \


GRIT_OBJ=\
        audio_disk_resource.o \
        audio.o \
        background_loader.o \
        bcol_parser.o \
        bullet_debug_drawer.o \
        collision_mesh.o \
        colour_conversion.o \
        console.o \
		core_option.o \
        dense_index_map.o \
        disk_resource.o \
        external_table.o \
        gfx_body.o \
        gfx_disk_resource.o \
        gfx_fertile_node.o \
        gfx_font.o \
        gfx_hud.o \
        gfx_instances.o \
        gfx_light.o \
        gfx_material.o \
        gfx_node.o \
        gfx.o \
        gfx_option.o \
        gfx_particle_system.o \
        gfx_pipeline.o \
        gfx_ranged_instances.o \
        gfx_shader.o \
        gfx_sky_body.o \
        gfx_sky_material.o \
        gfx_text_buffer.o \
        grit_class.o \
        grit_lua_util.o \
        grit_object.o \
        input_filter.o \
        io_util.o \
        keyboard_x11.o \
        ldbglue.o \
        lua_stack.o \
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
        mouse_x11.o \
        net_address.o \
        net_manager.o \
        net_message.o \
        net.o \
        ogg_vorbis_decoder.o \
        path_util.o \
        physical_material.o \
        physics_world.o \
        posix_sleep.o \
        ray.o \
        streamer.o \
        tcol_lexer-core-engine.o \
        tcol_lexer.o \
        tcol_parser.o \
        unicode_util.o \
        x11_clipboard.o \
        $(GSL_OBJ) \



%/Grit.$(GRIT_EXEC_SUFFIX): Grit.$(GRIT_EXEC_SUFFIX)
	strip -s $< -o $@
	
Grit.$(GRIT_EXEC_SUFFIX): $(GRIT_OBJ) $(OGRE_ARCHIVE) $(BULLET_ARCHIVES) $(LUA_ARCHIVES) $(ICU_ARCHIVES)
	@$(LINKING)
	@$(CXX) $(GRIT_OBJ) -o $@ $(LDFLAGS) $(LDLIBS)



COL_CONV_OBJ = grit_col_conv.o \
               bcol_parser.o \
               tcol_parser.o \
               tcol_lexer.o \
               tcol_lexer-core-engine.o \

%/grit_col_conv.$(GRIT_EXEC_SUFFIX): grit_col_conv.$(GRIT_EXEC_SUFFIX)
	strip -s $< -o $@

grit_col_conv.$(GRIT_EXEC_SUFFIX): $(COL_CONV_OBJ)
	@$(LINKING)
	@$(CXX) $(COL_CONV_OBJ) -o $@ $(LDFLAGS) $(LDLIBS)


%/gsl.$(GRIT_EXEC_SUFFIX): gsl.$(GRIT_EXEC_SUFFIX)
	strip -s $< -o $@

gsl.$(GRIT_EXEC_SUFFIX): $(GSL_OBJ) gfx_gasoline_standalone.o
	@$(LINKING)
	@$(CXX) $(GSL_OBJ) gfx_gasoline_standalone.o -o $@ $(LDFLAGS) $(LDLIBS)



clean:
	rm -fv $(TARGETS) *.o




# DO NOT DELETE

background_loader.o: ../../dependencies/util/sleep.h
background_loader.o: ../src/gfx/gfx_disk_resource.h
background_loader.o: ../depend_stubs/OgreResource.h
background_loader.o: ../depend_stubs/OgreTexture.h
background_loader.o: ../depend_stubs/OgreMesh.h
background_loader.o: ../../dependencies/util/math_util.h
background_loader.o: ../depend_stubs/string ../depend_stubs/iostream
background_loader.o: ../depend_stubs/cmath ../depend_stubs/cfloat
background_loader.o: ../src/centralised_log.h ../depend_stubs/sstream
background_loader.o: ../../dependencies/util/console.h
background_loader.o: ../depend_stubs/map ../depend_stubs/ostream
background_loader.o: ../depend_stubs/set ../depend_stubs/vector
background_loader.o: ../../dependencies/util/exception.h
background_loader.o: ../depend_stubs/cstdlib
background_loader.o: ../../dependencies/util/intrinsics.h
background_loader.o: ../src/background_loader.h ../src/vect_util.h
background_loader.o: ../depend_stubs/algorithm
background_loader.o: ../src/centralised_log.h
background_loader.o: ../depend_stubs/condition_variable
background_loader.o: ../depend_stubs/list ../depend_stubs/mutex
background_loader.o: ../depend_stubs/thread ../src/disk_resource.h
background_loader.o: ../src/background_loader.h ../src/main.h
background_loader.o: ../depend_stubs/lua.h ../src/mouse.h
background_loader.o: ../src/keyboard.h ../src/bullet_debug_drawer.h
background_loader.o: ../depend_stubs/OgreFrameListener.h
background_loader.o: ../depend_stubs/OgreManualObject.h
background_loader.o: ../depend_stubs/btBulletCollisionCommon.h
bullet_debug_drawer.o: ../depend_stubs/OgreSceneManager.h
bullet_debug_drawer.o: ../depend_stubs/OgreMaterialManager.h
bullet_debug_drawer.o: ../depend_stubs/OgreRoot.h
bullet_debug_drawer.o: ../src/bullet_debug_drawer.h
bullet_debug_drawer.o: ../depend_stubs/OgreFrameListener.h
bullet_debug_drawer.o: ../depend_stubs/OgreManualObject.h
bullet_debug_drawer.o: ../depend_stubs/btBulletCollisionCommon.h
bullet_debug_drawer.o: ../src/centralised_log.h ../src/gfx/gfx.h
bullet_debug_drawer.o: ../src/vect_util.h ../depend_stubs/vector
bullet_debug_drawer.o: ../depend_stubs/algorithm ../src/shared_ptr.h
bullet_debug_drawer.o: ../depend_stubs/cstdlib
bullet_debug_drawer.o: ../depend_stubs/functional
bullet_debug_drawer.o: ../depend_stubs/string
bullet_debug_drawer.o: ../../dependencies/util/math_util.h
bullet_debug_drawer.o: ../depend_stubs/iostream ../depend_stubs/cmath
bullet_debug_drawer.o: ../depend_stubs/cfloat
bullet_debug_drawer.o: ../src/gfx/gfx_disk_resource.h
bullet_debug_drawer.o: ../depend_stubs/OgreResource.h
bullet_debug_drawer.o: ../depend_stubs/OgreTexture.h
bullet_debug_drawer.o: ../depend_stubs/OgreMesh.h
bullet_debug_drawer.o: ../src/centralised_log.h
bullet_debug_drawer.o: ../depend_stubs/sstream
bullet_debug_drawer.o: ../../dependencies/util/console.h
bullet_debug_drawer.o: ../depend_stubs/map ../depend_stubs/ostream
bullet_debug_drawer.o: ../depend_stubs/set
bullet_debug_drawer.o: ../../dependencies/util/exception.h
bullet_debug_drawer.o: ../../dependencies/util/intrinsics.h
bullet_debug_drawer.o: ../src/background_loader.h ../src/vect_util.h
bullet_debug_drawer.o: ../depend_stubs/condition_variable
bullet_debug_drawer.o: ../depend_stubs/list ../depend_stubs/mutex
bullet_debug_drawer.o: ../depend_stubs/thread ../src/disk_resource.h
bullet_debug_drawer.o: ../src/gfx/gfx_node.h
bullet_debug_drawer.o: ../depend_stubs/OgreSceneNode.h
bullet_debug_drawer.o: ../src/gfx/gfx_internal.h
bullet_debug_drawer.o: ../depend_stubs/Ogre.h
bullet_debug_drawer.o: ../depend_stubs/OgreMeshManager.h
bullet_debug_drawer.o: ../depend_stubs/OgreOctreeSceneManager.h
bullet_debug_drawer.o: ../depend_stubs/OgreVector3.h
bullet_debug_drawer.o: ../depend_stubs/OgreQuaternion.h
bullet_debug_drawer.o: ../depend_stubs/OgreCustomCompositionPass.h
bullet_debug_drawer.o: ../depend_stubs/OgreCompositor.h
bullet_debug_drawer.o: ../depend_stubs/OgreOctreePlugin.h
bullet_debug_drawer.o: ../depend_stubs/OgreCgPlugin.h
bullet_debug_drawer.o: ../depend_stubs/OgreRenderSystem.h
bullet_debug_drawer.o: ../depend_stubs/OgreGLRenderSystem.h
bullet_debug_drawer.o: ../depend_stubs/OgreGLGpuProgram.h
bullet_debug_drawer.o: ../depend_stubs/OgreGLSLGpuProgram.h
bullet_debug_drawer.o: ../depend_stubs/OgreGLSLProgram.h
bullet_debug_drawer.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
bullet_debug_drawer.o: ../src/gfx/gfx.h ../src/gfx/gfx_option.h
bullet_debug_drawer.o: ../src/gfx/gfx_particle_system.h
bullet_debug_drawer.o: ../depend_stubs/utility ../depend_stubs/lua.h
bullet_debug_drawer.o: ../depend_stubs/lauxlib.h
bullet_debug_drawer.o: ../depend_stubs/lualib.h
bullet_debug_drawer.o: ../depend_stubs/OgreRenderWindow.h
bullet_debug_drawer.o: ../src/gfx/gfx_internal.h
bullet_debug_drawer.o: ../src/physics/physics_world.h
bullet_debug_drawer.o: ../depend_stubs/btBulletDynamicsCommon.h
bullet_debug_drawer.o: ../src/physics/tcol_parser.h
bullet_debug_drawer.o: ../src/physics/tcol_lexer
bullet_debug_drawer.o: ../depend_stubs/istream
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
bullet_debug_drawer.o: ../src/physics/tcol_lexer-token_ids
bullet_debug_drawer.o: ../depend_stubs/cstdio
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/Token
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/definitions
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/buffer/Buffer
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/QuexMode
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/Analyser
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/Counter
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/Accumulator
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/IncludeStack
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/temporary_macros_on
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/temporary_macros_off
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/constructor.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/Counter.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/misc.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
bullet_debug_drawer.o: ../depend_stubs/quex/code_base/template/Analyser.i
bullet_debug_drawer.o: ../src/physics/collision_mesh.h
bullet_debug_drawer.o: ../depend_stubs/OgreDataStream.h
bullet_debug_drawer.o: ../../dependencies/util/sleep.h
bullet_debug_drawer.o: ../src/disk_resource.h
bullet_debug_drawer.o: ../src/physics/bcol_parser.h
bullet_debug_drawer.o: ../depend_stubs/stdint.h
bullet_debug_drawer.o: ../src/physics/loose_end.h
bullet_debug_drawer.o: ../src/physics/physical_material.h
bullet_debug_drawer.o: ../src/grit_object.h ../src/shared_ptr.h
bullet_debug_drawer.o: ../src/streamer.h ../src/grit_object.h
bullet_debug_drawer.o: ../src/external_table.h ../src/lua_ptr.h
bullet_debug_drawer.o: ../../dependencies/util/spline_table.h
bullet_debug_drawer.o: ../src/background_loader.h ../src/lua_ptr.h
core_option.o: ../src/streamer.h ../depend_stubs/map
core_option.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
core_option.o: ../depend_stubs/lualib.h ../src/grit_object.h
core_option.o: ../depend_stubs/vector ../depend_stubs/set
core_option.o: ../depend_stubs/string ../src/shared_ptr.h
core_option.o: ../../dependencies/util/math_util.h
core_option.o: ../depend_stubs/iostream ../depend_stubs/cmath
core_option.o: ../depend_stubs/cfloat ../src/core_option.h
core_option.o: ../src/option.h ../depend_stubs/sstream
core_option.o: ../depend_stubs/ostream
core_option.o: ../../dependencies/util/exception.h
core_option.o: ../depend_stubs/cstdlib
core_option.o: ../../dependencies/util/intrinsics.h
dense_index_map.o: ../depend_stubs/algorithm ../depend_stubs/sstream
dense_index_map.o: ../src/dense_index_map.h ../depend_stubs/vector
dense_index_map.o: ../src/centralised_log.h
disk_resource.o: ../src/core_option.h ../depend_stubs/map
disk_resource.o: ../src/option.h ../depend_stubs/string
disk_resource.o: ../depend_stubs/sstream ../depend_stubs/ostream
disk_resource.o: ../../dependencies/util/exception.h
disk_resource.o: ../depend_stubs/cstdlib ../depend_stubs/iostream
disk_resource.o: ../../dependencies/util/intrinsics.h ../src/main.h
disk_resource.o: ../depend_stubs/lua.h ../src/mouse.h
disk_resource.o: ../depend_stubs/vector ../src/keyboard.h
disk_resource.o: ../src/background_loader.h ../src/vect_util.h
disk_resource.o: ../depend_stubs/algorithm ../src/centralised_log.h
disk_resource.o: ../src/bullet_debug_drawer.h
disk_resource.o: ../depend_stubs/OgreFrameListener.h
disk_resource.o: ../depend_stubs/OgreManualObject.h
disk_resource.o: ../depend_stubs/btBulletCollisionCommon.h
disk_resource.o: ../src/gfx/gfx_disk_resource.h
disk_resource.o: ../depend_stubs/OgreResource.h
disk_resource.o: ../depend_stubs/OgreTexture.h
disk_resource.o: ../depend_stubs/OgreMesh.h
disk_resource.o: ../../dependencies/util/math_util.h
disk_resource.o: ../depend_stubs/cmath ../depend_stubs/cfloat
disk_resource.o: ../src/centralised_log.h
disk_resource.o: ../../dependencies/util/console.h ../depend_stubs/set
disk_resource.o: ../src/background_loader.h
disk_resource.o: ../depend_stubs/condition_variable
disk_resource.o: ../depend_stubs/list ../depend_stubs/mutex
disk_resource.o: ../depend_stubs/thread ../src/disk_resource.h
disk_resource.o: ../src/audio/audio_disk_resource.h
disk_resource.o: ../depend_stubs/AL/al.h
disk_resource.o: ../depend_stubs/OgreResourceGroupManager.h
disk_resource.o: ../src/physics/collision_mesh.h
disk_resource.o: ../depend_stubs/OgreDataStream.h
disk_resource.o: ../../dependencies/util/sleep.h
disk_resource.o: ../src/disk_resource.h ../src/shared_ptr.h
disk_resource.o: ../depend_stubs/functional
disk_resource.o: ../src/physics/tcol_parser.h
disk_resource.o: ../src/physics/tcol_lexer ../depend_stubs/istream
disk_resource.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
disk_resource.o: ../src/physics/tcol_lexer-token_ids
disk_resource.o: ../depend_stubs/cstdio
disk_resource.o: ../depend_stubs/quex/code_base/Token
disk_resource.o: ../depend_stubs/quex/code_base/definitions
disk_resource.o: ../depend_stubs/quex/code_base/buffer/Buffer
disk_resource.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
disk_resource.o: ../depend_stubs/quex/code_base/template/QuexMode
disk_resource.o: ../depend_stubs/quex/code_base/template/Analyser
disk_resource.o: ../depend_stubs/quex/code_base/template/Counter
disk_resource.o: ../depend_stubs/quex/code_base/template/Accumulator
disk_resource.o: ../depend_stubs/quex/code_base/template/IncludeStack
disk_resource.o: ../depend_stubs/quex/code_base/temporary_macros_on
disk_resource.o: ../depend_stubs/quex/code_base/temporary_macros_off
disk_resource.o: ../depend_stubs/quex/code_base/template/constructor.i
disk_resource.o: ../depend_stubs/quex/code_base/template/Counter.i
disk_resource.o: ../depend_stubs/quex/code_base/template/Accumulator.i
disk_resource.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
disk_resource.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
disk_resource.o: ../depend_stubs/quex/code_base/template/mode_handling.i
disk_resource.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
disk_resource.o: ../depend_stubs/quex/code_base/template/misc.i
disk_resource.o: ../depend_stubs/quex/code_base/template/buffer_access.i
disk_resource.o: ../depend_stubs/quex/code_base/template/Analyser.i
disk_resource.o: ../src/physics/bcol_parser.h ../depend_stubs/stdint.h
disk_resource.o: ../src/physics/loose_end.h
disk_resource.o: ../src/physics/physical_material.h
external_table.o: ../src/external_table.h ../depend_stubs/map
external_table.o: ../depend_stubs/string ../depend_stubs/lua.h
external_table.o: ../depend_stubs/lauxlib.h
external_table.o: ../../dependencies/util/math_util.h
external_table.o: ../depend_stubs/iostream ../depend_stubs/cmath
external_table.o: ../depend_stubs/cfloat ../src/shared_ptr.h
external_table.o: ../src/lua_ptr.h ../depend_stubs/lualib.h
external_table.o: ../src/centralised_log.h
external_table.o: ../../dependencies/util/spline_table.h
external_table.o: ../depend_stubs/cstdlib ../src/grit_lua_util.h
external_table.o: ../depend_stubs/OgreException.h
external_table.o: ../../dependencies/util/lua_util.h
external_table.o: ../depend_stubs/sstream ../depend_stubs/limits
external_table.o: ../depend_stubs/vector
external_table.o: ../../dependencies/util/exception.h
external_table.o: ../../dependencies/util/intrinsics.h
external_table.o: ../../dependencies/util/lua_wrappers_common.h
external_table.o: ../src/lua_wrappers_primitives.h
grit_class.o: ../src/grit_class.h ../depend_stubs/string
grit_class.o: ../depend_stubs/map ../depend_stubs/lua.h
grit_class.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
grit_class.o: ../src/external_table.h
grit_class.o: ../../dependencies/util/math_util.h
grit_class.o: ../depend_stubs/iostream ../depend_stubs/cmath
grit_class.o: ../depend_stubs/cfloat ../src/shared_ptr.h
grit_class.o: ../src/lua_ptr.h ../src/centralised_log.h
grit_class.o: ../../dependencies/util/spline_table.h
grit_class.o: ../depend_stubs/cstdlib ../src/grit_lua_util.h
grit_class.o: ../depend_stubs/OgreException.h
grit_class.o: ../../dependencies/util/lua_util.h
grit_class.o: ../depend_stubs/sstream ../depend_stubs/limits
grit_class.o: ../depend_stubs/vector
grit_class.o: ../../dependencies/util/exception.h
grit_class.o: ../../dependencies/util/intrinsics.h
grit_class.o: ../../dependencies/util/lua_wrappers_common.h
grit_class.o: ../src/path_util.h
grit_lua_util.o: ../depend_stubs/cstdlib ../depend_stubs/map
grit_lua_util.o: ../../dependencies/util/io_util.h
grit_lua_util.o: ../depend_stubs/cstdio ../depend_stubs/cstring
grit_lua_util.o: ../depend_stubs/string ../depend_stubs/iostream
grit_lua_util.o: ../depend_stubs/fstream
grit_lua_util.o: ../../dependencies/util/exception.h
grit_lua_util.o: ../depend_stubs/sstream
grit_lua_util.o: ../../dependencies/util/intrinsics.h
grit_lua_util.o: ../../dependencies/util/lua_stack.h
grit_lua_util.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
grit_lua_util.o: ../depend_stubs/lualib.h ../src/lua_ptr.h
grit_lua_util.o: ../src/centralised_log.h ../src/grit_lua_util.h
grit_lua_util.o: ../depend_stubs/OgreException.h
grit_lua_util.o: ../../dependencies/util/lua_util.h
grit_lua_util.o: ../depend_stubs/limits ../depend_stubs/vector
grit_lua_util.o: ../../dependencies/util/math_util.h
grit_lua_util.o: ../depend_stubs/cmath ../depend_stubs/cfloat
grit_lua_util.o: ../../dependencies/util/lua_wrappers_common.h
grit_lua_util.o: ../src/path_util.h
grit_object.o: ../depend_stubs/cmath ../src/main.h
grit_object.o: ../depend_stubs/lua.h ../src/mouse.h
grit_object.o: ../depend_stubs/vector ../src/keyboard.h
grit_object.o: ../depend_stubs/string ../src/background_loader.h
grit_object.o: ../src/vect_util.h ../depend_stubs/algorithm
grit_object.o: ../src/centralised_log.h ../src/bullet_debug_drawer.h
grit_object.o: ../depend_stubs/OgreFrameListener.h
grit_object.o: ../depend_stubs/OgreManualObject.h
grit_object.o: ../depend_stubs/btBulletCollisionCommon.h
grit_object.o: ../src/grit_object.h ../depend_stubs/map
grit_object.o: ../depend_stubs/set ../src/shared_ptr.h
grit_object.o: ../src/grit_class.h ../depend_stubs/lauxlib.h
grit_object.o: ../depend_stubs/lualib.h ../src/external_table.h
grit_object.o: ../../dependencies/util/math_util.h
grit_object.o: ../depend_stubs/iostream ../depend_stubs/cfloat
grit_object.o: ../src/lua_ptr.h ../../dependencies/util/spline_table.h
grit_object.o: ../depend_stubs/cstdlib ../src/grit_lua_util.h
grit_object.o: ../depend_stubs/OgreException.h
grit_object.o: ../../dependencies/util/lua_util.h
grit_object.o: ../depend_stubs/sstream ../depend_stubs/limits
grit_object.o: ../../dependencies/util/exception.h
grit_object.o: ../../dependencies/util/intrinsics.h
grit_object.o: ../../dependencies/util/lua_wrappers_common.h
grit_object.o: ../src/path_util.h ../src/lua_wrappers_gritobj.h
grit_object.o: ../src/streamer.h
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
input_filter.o: ../src/lua_ptr.h ../src/centralised_log.h
input_filter.o: ../src/input_filter.h ../depend_stubs/map
input_filter.o: ../depend_stubs/set ../src/gfx/gfx_hud.h
input_filter.o: ../src/shared_ptr.h ../depend_stubs/functional
input_filter.o: ../src/external_table.h ../src/shared_ptr.h
input_filter.o: ../../dependencies/util/spline_table.h
input_filter.o: ../src/grit_lua_util.h ../depend_stubs/OgreException.h
input_filter.o: ../../dependencies/util/lua_wrappers_common.h
input_filter.o: ../src/lua_ptr.h ../src/path_util.h ../src/vect_util.h
input_filter.o: ../depend_stubs/algorithm ../src/gfx/gfx_pipeline.h
input_filter.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
input_filter.o: ../depend_stubs/OgreMeshManager.h
input_filter.o: ../depend_stubs/OgreOctreeSceneManager.h
input_filter.o: ../depend_stubs/OgreVector3.h
input_filter.o: ../depend_stubs/OgreQuaternion.h
input_filter.o: ../depend_stubs/OgreCustomCompositionPass.h
input_filter.o: ../depend_stubs/OgreCompositor.h
input_filter.o: ../depend_stubs/OgreOctreePlugin.h
input_filter.o: ../depend_stubs/OgreCgPlugin.h
input_filter.o: ../depend_stubs/OgreRenderSystem.h
input_filter.o: ../depend_stubs/OgreGLRenderSystem.h
input_filter.o: ../depend_stubs/OgreGLGpuProgram.h
input_filter.o: ../depend_stubs/OgreGLSLGpuProgram.h
input_filter.o: ../depend_stubs/OgreGLSLProgram.h
input_filter.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
input_filter.o: ../src/gfx/gfx.h ../src/gfx/gfx_option.h
input_filter.o: ../src/gfx/gfx_font.h ../src/background_loader.h
input_filter.o: ../src/vect_util.h ../depend_stubs/condition_variable
input_filter.o: ../depend_stubs/list ../depend_stubs/mutex
input_filter.o: ../depend_stubs/thread ../src/disk_resource.h
input_filter.o: ../src/main.h ../src/mouse.h ../src/keyboard.h
input_filter.o: ../src/background_loader.h
input_filter.o: ../src/bullet_debug_drawer.h
input_filter.o: ../depend_stubs/OgreFrameListener.h
input_filter.o: ../depend_stubs/OgreManualObject.h
input_filter.o: ../depend_stubs/btBulletCollisionCommon.h
input_filter.o: ../src/gfx/gfx_disk_resource.h
input_filter.o: ../depend_stubs/OgreResource.h
input_filter.o: ../depend_stubs/OgreTexture.h
input_filter.o: ../depend_stubs/OgreMesh.h ../src/centralised_log.h
input_filter.o: ../../dependencies/util/console.h
input_filter.o: ../depend_stubs/ostream ../src/gfx/gfx_text_buffer.h
input_filter.o: ../depend_stubs/cstdint
input_filter.o: ../depend_stubs/OgreHardwareBufferManager.h
input_filter.o: ../depend_stubs/OgreHardwareVertexBuffer.h
input_filter.o: ../depend_stubs/OgreHardwareIndexBuffer.h
ldbglue.o: ../depend_stubs/lua.h
lua_wrappers_core.o: ../depend_stubs/limits.h
lua_wrappers_core.o: ../depend_stubs/sys/mman.h
lua_wrappers_core.o: ../../dependencies/util/sleep.h
lua_wrappers_core.o: ../../dependencies/util/lua_stack.h
lua_wrappers_core.o: ../depend_stubs/string ../depend_stubs/lua.h
lua_wrappers_core.o: ../depend_stubs/lauxlib.h
lua_wrappers_core.o: ../depend_stubs/lualib.h
lua_wrappers_core.o: ../../dependencies/util/lua_utf8.h
lua_wrappers_core.o: ../../dependencies/util/io_util.h
lua_wrappers_core.o: ../depend_stubs/cstdlib ../depend_stubs/cstdio
lua_wrappers_core.o: ../depend_stubs/cstring ../depend_stubs/iostream
lua_wrappers_core.o: ../depend_stubs/fstream
lua_wrappers_core.o: ../../dependencies/util/exception.h
lua_wrappers_core.o: ../depend_stubs/sstream
lua_wrappers_core.o: ../../dependencies/util/intrinsics.h
lua_wrappers_core.o: ../src/core_option.h ../depend_stubs/map
lua_wrappers_core.o: ../src/option.h ../depend_stubs/ostream
lua_wrappers_core.o: ../src/grit_lua_util.h
lua_wrappers_core.o: ../depend_stubs/OgreException.h
lua_wrappers_core.o: ../src/centralised_log.h
lua_wrappers_core.o: ../../dependencies/util/lua_util.h
lua_wrappers_core.o: ../depend_stubs/limits ../depend_stubs/vector
lua_wrappers_core.o: ../../dependencies/util/math_util.h
lua_wrappers_core.o: ../depend_stubs/cmath ../depend_stubs/cfloat
lua_wrappers_core.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_core.o: ../src/input_filter.h ../depend_stubs/set
lua_wrappers_core.o: ../src/lua_ptr.h ../src/keyboard.h ../src/mouse.h
lua_wrappers_core.o: ../src/background_loader.h ../src/vect_util.h
lua_wrappers_core.o: ../depend_stubs/algorithm ../src/clipboard.h
lua_wrappers_core.o: ../src/main.h ../src/bullet_debug_drawer.h
lua_wrappers_core.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_core.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_core.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_core.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_core.o: ../depend_stubs/OgreResource.h
lua_wrappers_core.o: ../depend_stubs/OgreTexture.h
lua_wrappers_core.o: ../depend_stubs/OgreMesh.h
lua_wrappers_core.o: ../src/centralised_log.h
lua_wrappers_core.o: ../../dependencies/util/console.h
lua_wrappers_core.o: ../src/background_loader.h
lua_wrappers_core.o: ../depend_stubs/condition_variable
lua_wrappers_core.o: ../depend_stubs/list ../depend_stubs/mutex
lua_wrappers_core.o: ../depend_stubs/thread ../src/disk_resource.h
lua_wrappers_core.o: ../src/lua_wrappers_primitives.h
lua_wrappers_core.o: ../../dependencies/util/spline_table.h
lua_wrappers_core.o: ../src/path_util.h ../src/lua_wrappers_gritobj.h
lua_wrappers_core.o: ../src/streamer.h ../src/grit_object.h
lua_wrappers_core.o: ../src/shared_ptr.h
lua_wrappers_core.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_core.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_core.o: ../src/grit_lua_util.h ../src/gfx/gfx.h
lua_wrappers_core.o: ../src/vect_util.h ../src/shared_ptr.h
lua_wrappers_core.o: ../depend_stubs/functional
lua_wrappers_core.o: ../src/gfx/gfx_fertile_node.h
lua_wrappers_core.o: ../src/gfx/gfx_node.h
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
lua_wrappers_core.o: ../depend_stubs/OgreGLGpuProgram.h
lua_wrappers_core.o: ../depend_stubs/OgreGLSLGpuProgram.h
lua_wrappers_core.o: ../depend_stubs/OgreGLSLProgram.h
lua_wrappers_core.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
lua_wrappers_core.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_body.h
lua_wrappers_core.o: ../src/gfx/gfx_material.h
lua_wrappers_core.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_core.o: ../src/gfx/gfx_shader.h ../depend_stubs/cstdint
lua_wrappers_core.o: ../depend_stubs/array
lua_wrappers_core.o: ../depend_stubs/unordered_map
lua_wrappers_core.o: ../depend_stubs/OgreHighLevelGpuProgram.h
lua_wrappers_core.o: ../src/gfx/gfx_gasoline.h
lua_wrappers_core.o: ../src/gfx/gfx_pipeline.h
lua_wrappers_core.o: ../src/gfx/gfx_sky_body.h
lua_wrappers_core.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_core.o: ../src/gfx/gfx_sky_material.h
lua_wrappers_core.o: ../src/gfx/gfx_light.h
lua_wrappers_core.o: ../src/gfx/gfx_particle_system.h
lua_wrappers_core.o: ../depend_stubs/utility
lua_wrappers_core.o: ../src/gfx/gfx_instances.h
lua_wrappers_core.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_core.o: ../src/dense_index_map.h
lua_wrappers_core.o: ../src/gfx/gfx_ranged_instances.h
lua_wrappers_core.o: ../src/streamer.h
lua_wrappers_core.o: ../src/cache_friendly_range_space_simd.h
lua_wrappers_core.o: ../src/sse_allocator.h ../src/gfx/gfx_hud.h
lua_wrappers_core.o: ../src/external_table.h ../src/lua_ptr.h
lua_wrappers_core.o: ../src/path_util.h ../src/gfx/gfx_font.h
lua_wrappers_core.o: ../src/main.h ../src/gfx/gfx_text_buffer.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_core.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_core.o: ../src/audio/lua_wrappers_audio.h
lua_wrappers_core.o: ../src/audio/audio.h ../depend_stubs/AL/al.h
lua_wrappers_core.o: ../src/disk_resource.h
lua_wrappers_core.o: ../src/audio/audio_disk_resource.h
lua_wrappers_core.o: ../depend_stubs/OgreResourceGroupManager.h
lua_wrappers_core.o: ../src/physics/lua_wrappers_physics.h
lua_wrappers_core.o: ../src/physics/physics_world.h
lua_wrappers_core.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_core.o: ../src/physics/tcol_parser.h
lua_wrappers_core.o: ../src/physics/tcol_lexer ../depend_stubs/istream
lua_wrappers_core.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_core.o: ../src/physics/tcol_lexer-token_ids
lua_wrappers_core.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_core.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_core.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_core.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_core.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_core.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_core.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_core.o: ../src/physics/collision_mesh.h
lua_wrappers_core.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_core.o: ../src/physics/bcol_parser.h
lua_wrappers_core.o: ../depend_stubs/stdint.h
lua_wrappers_core.o: ../src/physics/loose_end.h
lua_wrappers_core.o: ../src/physics/physical_material.h
lua_wrappers_core.o: ../src/grit_object.h ../src/external_table.h
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
lua_wrappers_disk_resource.o: ../src/centralised_log.h
lua_wrappers_disk_resource.o: ../../dependencies/util/lua_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/sstream
lua_wrappers_disk_resource.o: ../depend_stubs/limits
lua_wrappers_disk_resource.o: ../depend_stubs/vector
lua_wrappers_disk_resource.o: ../../dependencies/util/exception.h
lua_wrappers_disk_resource.o: ../depend_stubs/cstdlib
lua_wrappers_disk_resource.o: ../depend_stubs/iostream
lua_wrappers_disk_resource.o: ../../dependencies/util/intrinsics.h
lua_wrappers_disk_resource.o: ../../dependencies/util/math_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/cmath
lua_wrappers_disk_resource.o: ../depend_stubs/cfloat
lua_wrappers_disk_resource.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_disk_resource.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_disk_resource.o: ../src/disk_resource.h
lua_wrappers_disk_resource.o: ../depend_stubs/set ../src/main.h
lua_wrappers_disk_resource.o: ../src/mouse.h ../src/keyboard.h
lua_wrappers_disk_resource.o: ../src/background_loader.h
lua_wrappers_disk_resource.o: ../src/vect_util.h
lua_wrappers_disk_resource.o: ../depend_stubs/algorithm
lua_wrappers_disk_resource.o: ../src/bullet_debug_drawer.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_disk_resource.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_disk_resource.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreResource.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreTexture.h
lua_wrappers_disk_resource.o: ../depend_stubs/OgreMesh.h
lua_wrappers_disk_resource.o: ../src/centralised_log.h
lua_wrappers_disk_resource.o: ../../dependencies/util/console.h
lua_wrappers_disk_resource.o: ../depend_stubs/map
lua_wrappers_disk_resource.o: ../depend_stubs/ostream
lua_wrappers_disk_resource.o: ../src/background_loader.h
lua_wrappers_disk_resource.o: ../depend_stubs/condition_variable
lua_wrappers_disk_resource.o: ../depend_stubs/list
lua_wrappers_disk_resource.o: ../depend_stubs/mutex
lua_wrappers_disk_resource.o: ../depend_stubs/thread
lua_wrappers_gritobj.o: ../src/grit_object.h ../depend_stubs/map
lua_wrappers_gritobj.o: ../depend_stubs/vector ../depend_stubs/set
lua_wrappers_gritobj.o: ../depend_stubs/string ../src/shared_ptr.h
lua_wrappers_gritobj.o: ../src/grit_class.h ../depend_stubs/lua.h
lua_wrappers_gritobj.o: ../depend_stubs/lauxlib.h
lua_wrappers_gritobj.o: ../depend_stubs/lualib.h
lua_wrappers_gritobj.o: ../src/external_table.h
lua_wrappers_gritobj.o: ../../dependencies/util/math_util.h
lua_wrappers_gritobj.o: ../depend_stubs/iostream ../depend_stubs/cmath
lua_wrappers_gritobj.o: ../depend_stubs/cfloat ../src/lua_ptr.h
lua_wrappers_gritobj.o: ../src/centralised_log.h
lua_wrappers_gritobj.o: ../../dependencies/util/spline_table.h
lua_wrappers_gritobj.o: ../depend_stubs/cstdlib ../src/grit_lua_util.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreException.h
lua_wrappers_gritobj.o: ../../dependencies/util/lua_util.h
lua_wrappers_gritobj.o: ../depend_stubs/sstream ../depend_stubs/limits
lua_wrappers_gritobj.o: ../../dependencies/util/exception.h
lua_wrappers_gritobj.o: ../../dependencies/util/intrinsics.h
lua_wrappers_gritobj.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_gritobj.o: ../src/path_util.h ../src/main.h
lua_wrappers_gritobj.o: ../src/mouse.h ../src/keyboard.h
lua_wrappers_gritobj.o: ../src/background_loader.h ../src/vect_util.h
lua_wrappers_gritobj.o: ../depend_stubs/algorithm
lua_wrappers_gritobj.o: ../src/bullet_debug_drawer.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_gritobj.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_gritobj.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_gritobj.o: ../src/streamer.h
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
lua_wrappers_primitives.o: ../src/centralised_log.h
lua_wrappers_primitives.o: ../../dependencies/util/lua_util.h
lua_wrappers_primitives.o: ../depend_stubs/sstream
lua_wrappers_primitives.o: ../depend_stubs/limits
lua_wrappers_primitives.o: ../depend_stubs/vector
lua_wrappers_primitives.o: ../../dependencies/util/exception.h
lua_wrappers_primitives.o: ../depend_stubs/cstdlib
lua_wrappers_primitives.o: ../depend_stubs/iostream
lua_wrappers_primitives.o: ../../dependencies/util/intrinsics.h
lua_wrappers_primitives.o: ../../dependencies/util/math_util.h
lua_wrappers_primitives.o: ../depend_stubs/cmath
lua_wrappers_primitives.o: ../depend_stubs/cfloat
lua_wrappers_primitives.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_primitives.o: ../../dependencies/util/spline_table.h
lua_wrappers_primitives.o: ../depend_stubs/map
main.o: ../depend_stubs/cerrno ../depend_stubs/sstream
main.o: ../src/linux/keyboard_x11.h ../depend_stubs/set
main.o: ../depend_stubs/map ../depend_stubs/X11/Xlib.h
main.o: ../depend_stubs/X11/Xutil.h ../depend_stubs/X11/keysym.h
main.o: ../src/keyboard.h ../depend_stubs/string
main.o: ../depend_stubs/vector ../src/linux/mouse_x11.h ../src/mouse.h
main.o: ../src/clipboard.h ../src/centralised_log.h
main.o: ../src/core_option.h ../src/option.h ../depend_stubs/ostream
main.o: ../../dependencies/util/exception.h ../depend_stubs/cstdlib
main.o: ../depend_stubs/iostream ../../dependencies/util/intrinsics.h
main.o: ../src/grit_lua_util.h ../depend_stubs/lua.h
main.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
main.o: ../depend_stubs/OgreException.h
main.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
main.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
main.o: ../depend_stubs/cfloat
main.o: ../../dependencies/util/lua_wrappers_common.h
main.o: ../src/lua_wrappers_core.h ../src/main.h ../src/mouse.h
main.o: ../src/keyboard.h ../src/background_loader.h
main.o: ../src/vect_util.h ../depend_stubs/algorithm
main.o: ../src/bullet_debug_drawer.h
main.o: ../depend_stubs/OgreFrameListener.h
main.o: ../depend_stubs/OgreManualObject.h
main.o: ../depend_stubs/btBulletCollisionCommon.h ../src/gfx/gfx.h
main.o: ../src/vect_util.h ../src/shared_ptr.h
main.o: ../depend_stubs/functional ../src/gfx/gfx_disk_resource.h
main.o: ../depend_stubs/OgreResource.h ../depend_stubs/OgreTexture.h
main.o: ../depend_stubs/OgreMesh.h ../src/centralised_log.h
main.o: ../../dependencies/util/console.h ../src/background_loader.h
main.o: ../depend_stubs/condition_variable ../depend_stubs/list
main.o: ../depend_stubs/mutex ../depend_stubs/thread
main.o: ../src/disk_resource.h ../src/gfx/gfx_node.h
main.o: ../depend_stubs/OgreSceneNode.h ../src/gfx/gfx_internal.h
main.o: ../depend_stubs/Ogre.h ../depend_stubs/OgreMeshManager.h
main.o: ../depend_stubs/OgreOctreeSceneManager.h
main.o: ../depend_stubs/OgreVector3.h ../depend_stubs/OgreQuaternion.h
main.o: ../depend_stubs/OgreCustomCompositionPass.h
main.o: ../depend_stubs/OgreCompositor.h
main.o: ../depend_stubs/OgreOctreePlugin.h
main.o: ../depend_stubs/OgreCgPlugin.h
main.o: ../depend_stubs/OgreRenderSystem.h
main.o: ../depend_stubs/OgreGLRenderSystem.h
main.o: ../depend_stubs/OgreGLGpuProgram.h
main.o: ../depend_stubs/OgreGLSLGpuProgram.h
main.o: ../depend_stubs/OgreGLSLProgram.h
main.o: ../depend_stubs/OgreGLSLLinkProgramManager.h ../src/gfx/gfx.h
main.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_particle_system.h
main.o: ../depend_stubs/utility ../depend_stubs/OgreRoot.h
main.o: ../depend_stubs/OgreRenderWindow.h
main.o: ../src/physics/physics_world.h
main.o: ../depend_stubs/btBulletDynamicsCommon.h
main.o: ../src/physics/tcol_parser.h ../src/physics/tcol_lexer
main.o: ../depend_stubs/istream
main.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
main.o: ../src/physics/tcol_lexer-token_ids ../depend_stubs/cstdio
main.o: ../depend_stubs/quex/code_base/Token
main.o: ../depend_stubs/quex/code_base/definitions
main.o: ../depend_stubs/quex/code_base/buffer/Buffer
main.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
main.o: ../depend_stubs/quex/code_base/template/QuexMode
main.o: ../depend_stubs/quex/code_base/template/Analyser
main.o: ../depend_stubs/quex/code_base/template/Counter
main.o: ../depend_stubs/quex/code_base/template/Accumulator
main.o: ../depend_stubs/quex/code_base/template/IncludeStack
main.o: ../depend_stubs/quex/code_base/temporary_macros_on
main.o: ../depend_stubs/quex/code_base/temporary_macros_off
main.o: ../depend_stubs/quex/code_base/template/constructor.i
main.o: ../depend_stubs/quex/code_base/template/Counter.i
main.o: ../depend_stubs/quex/code_base/template/Accumulator.i
main.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
main.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
main.o: ../depend_stubs/quex/code_base/template/mode_handling.i
main.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
main.o: ../depend_stubs/quex/code_base/template/misc.i
main.o: ../depend_stubs/quex/code_base/template/buffer_access.i
main.o: ../depend_stubs/quex/code_base/template/Analyser.i
main.o: ../src/physics/collision_mesh.h
main.o: ../depend_stubs/OgreDataStream.h
main.o: ../../dependencies/util/sleep.h ../src/disk_resource.h
main.o: ../src/physics/bcol_parser.h ../depend_stubs/stdint.h
main.o: ../src/physics/loose_end.h ../src/physics/physical_material.h
main.o: ../src/grit_object.h ../src/shared_ptr.h ../src/streamer.h
main.o: ../src/grit_object.h ../src/external_table.h ../src/lua_ptr.h
main.o: ../../dependencies/util/spline_table.h ../src/lua_ptr.h
main.o: ../src/audio/audio.h ../depend_stubs/AL/al.h
main.o: ../src/audio/audio_disk_resource.h
main.o: ../depend_stubs/OgreResourceGroupManager.h ../src/net/net.h
main.o: ../depend_stubs/sys/socket.h ../depend_stubs/arpa/inet.h
main.o: ../depend_stubs/sys/param.h ../depend_stubs/sys/types.h
main.o: ../depend_stubs/netdb.h ../depend_stubs/sys/ioctl.h
path_util.o: ../src/centralised_log.h ../src/path_util.h
path_util.o: ../depend_stubs/vector ../depend_stubs/string
path_util.o: ../src/grit_lua_util.h ../depend_stubs/lua.h
path_util.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
path_util.o: ../depend_stubs/OgreException.h
path_util.o: ../../dependencies/util/lua_util.h
path_util.o: ../depend_stubs/sstream ../depend_stubs/limits
path_util.o: ../../dependencies/util/exception.h
path_util.o: ../depend_stubs/cstdlib ../depend_stubs/iostream
path_util.o: ../../dependencies/util/intrinsics.h
path_util.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
path_util.o: ../depend_stubs/cfloat
path_util.o: ../../dependencies/util/lua_wrappers_common.h
streamer.o: ../src/cache_friendly_range_space_simd.h
streamer.o: ../depend_stubs/map ../depend_stubs/vector
streamer.o: ../depend_stubs/iostream ../depend_stubs/algorithm
streamer.o: ../depend_stubs/cmath ../src/sse_allocator.h
streamer.o: ../src/centralised_log.h ../src/core_option.h
streamer.o: ../src/option.h ../depend_stubs/string
streamer.o: ../depend_stubs/sstream ../depend_stubs/ostream
streamer.o: ../../dependencies/util/exception.h
streamer.o: ../depend_stubs/cstdlib
streamer.o: ../../dependencies/util/intrinsics.h ../src/grit_class.h
streamer.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
streamer.o: ../depend_stubs/lualib.h ../src/external_table.h
streamer.o: ../../dependencies/util/math_util.h ../depend_stubs/cfloat
streamer.o: ../src/shared_ptr.h ../src/lua_ptr.h
streamer.o: ../../dependencies/util/spline_table.h
streamer.o: ../src/grit_lua_util.h ../depend_stubs/OgreException.h
streamer.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
streamer.o: ../../dependencies/util/lua_wrappers_common.h
streamer.o: ../src/path_util.h ../src/main.h ../src/mouse.h
streamer.o: ../src/keyboard.h ../src/background_loader.h
streamer.o: ../src/vect_util.h ../src/bullet_debug_drawer.h
streamer.o: ../depend_stubs/OgreFrameListener.h
streamer.o: ../depend_stubs/OgreManualObject.h
streamer.o: ../depend_stubs/btBulletCollisionCommon.h
streamer.o: ../src/streamer.h ../src/grit_object.h ../depend_stubs/set
keyboard_x11.o: ../depend_stubs/iostream ../depend_stubs/sstream
keyboard_x11.o: ../../dependencies/util/unicode_util.h
keyboard_x11.o: ../depend_stubs/cstdlib ../depend_stubs/string
keyboard_x11.o: ../src/linux/keyboard_x11.h ../depend_stubs/set
keyboard_x11.o: ../depend_stubs/map ../depend_stubs/X11/Xlib.h
keyboard_x11.o: ../depend_stubs/X11/Xutil.h
keyboard_x11.o: ../depend_stubs/X11/keysym.h ../src/keyboard.h
keyboard_x11.o: ../depend_stubs/vector ../src/centralised_log.h
keyboard_x11.o: ../../dependencies/util/console.h
keyboard_x11.o: ../depend_stubs/ostream
keyboard_x11.o: ../../dependencies/util/exception.h
keyboard_x11.o: ../../dependencies/util/intrinsics.h
mouse_x11.o: ../depend_stubs/sstream ../src/linux/mouse_x11.h
mouse_x11.o: ../depend_stubs/X11/Xlib.h ../src/mouse.h
mouse_x11.o: ../depend_stubs/vector ../src/centralised_log.h
mouse_x11.o: ../depend_stubs/iostream
mouse_x11.o: ../../dependencies/util/console.h
mouse_x11.o: ../depend_stubs/map ../depend_stubs/ostream
mouse_x11.o: ../depend_stubs/set ../depend_stubs/string
mouse_x11.o: ../../dependencies/util/exception.h
mouse_x11.o: ../depend_stubs/cstdlib
mouse_x11.o: ../../dependencies/util/intrinsics.h
x11_clipboard.o: ../depend_stubs/cstdlib ../depend_stubs/string
x11_clipboard.o: ../depend_stubs/X11/Xlib.h
x11_clipboard.o: ../depend_stubs/X11/Xatom.h ../src/clipboard.h
x11_clipboard.o: ../src/centralised_log.h
x11_clipboard.o: ../depend_stubs/iostream
x11_clipboard.o: ../depend_stubs/sstream
x11_clipboard.o: ../../dependencies/util/console.h
x11_clipboard.o: ../depend_stubs/map ../depend_stubs/ostream
x11_clipboard.o: ../depend_stubs/set ../depend_stubs/vector
x11_clipboard.o: ../../dependencies/util/exception.h
x11_clipboard.o: ../../dependencies/util/intrinsics.h
keyboard_direct_input8.o: ../depend_stubs/assert.h
keyboard_direct_input8.o: ../depend_stubs/iostream
keyboard_direct_input8.o: ../src/win32/keyboard_direct_input8.h
keyboard_direct_input8.o: ../depend_stubs/map
keyboard_direct_input8.o: ../depend_stubs/set
keyboard_direct_input8.o: ../depend_stubs/dinput.h
keyboard_direct_input8.o: ../src/keyboard.h
keyboard_direct_input8.o: ../depend_stubs/string
keyboard_direct_input8.o: ../depend_stubs/vector
keyboard_direct_input8.o: ../src/centralised_log.h
keyboard_direct_input8.o: ../depend_stubs/sstream
keyboard_direct_input8.o: ../../dependencies/util/console.h
keyboard_direct_input8.o: ../depend_stubs/ostream
keyboard_direct_input8.o: ../../dependencies/util/exception.h
keyboard_direct_input8.o: ../depend_stubs/cstdlib
keyboard_direct_input8.o: ../../dependencies/util/intrinsics.h
keyboard_win_api.o: ../depend_stubs/assert.h
keyboard_win_api.o: ../depend_stubs/iostream
keyboard_win_api.o: ../../dependencies/util/unicode_util.h
keyboard_win_api.o: ../depend_stubs/cstdlib
keyboard_win_api.o: ../depend_stubs/string
keyboard_win_api.o: ../src/win32/keyboard_win_api.h
keyboard_win_api.o: ../depend_stubs/map ../depend_stubs/set
keyboard_win_api.o: ../depend_stubs/windows.h ../src/keyboard.h
keyboard_win_api.o: ../depend_stubs/vector
keyboard_win_api.o: ../src/centralised_log.h
keyboard_win_api.o: ../depend_stubs/sstream
keyboard_win_api.o: ../../dependencies/util/console.h
keyboard_win_api.o: ../depend_stubs/ostream
keyboard_win_api.o: ../../dependencies/util/exception.h
keyboard_win_api.o: ../../dependencies/util/intrinsics.h
mouse_direct_input8.o: ../depend_stubs/assert.h
mouse_direct_input8.o: ../depend_stubs/iostream
mouse_direct_input8.o: ../src/win32/mouse_direct_input8.h
mouse_direct_input8.o: ../depend_stubs/dinput.h ../src/mouse.h
mouse_direct_input8.o: ../depend_stubs/vector
mouse_direct_input8.o: ../src/centralised_log.h
mouse_direct_input8.o: ../depend_stubs/sstream
mouse_direct_input8.o: ../../dependencies/util/console.h
mouse_direct_input8.o: ../depend_stubs/map
mouse_direct_input8.o: ../depend_stubs/ostream
mouse_direct_input8.o: ../depend_stubs/set
mouse_direct_input8.o: ../depend_stubs/string
mouse_direct_input8.o: ../../dependencies/util/exception.h
mouse_direct_input8.o: ../depend_stubs/cstdlib
mouse_direct_input8.o: ../../dependencies/util/intrinsics.h
win32_clipboard.o: ../depend_stubs/cstdlib
win32_clipboard.o: ../depend_stubs/string
win32_clipboard.o: ../depend_stubs/windows.h
win32_clipboard.o: ../../dependencies/util/unicode_util.h
win32_clipboard.o: ../src/clipboard.h
clutter.o: ../depend_stubs/OgreHardwareBufferManager.h
clutter.o: ../depend_stubs/OgreMaterialManager.h
clutter.o: ../depend_stubs/OgreSubMesh.h
clutter.o: ../depend_stubs/OgreMesh.h ../src/main.h
clutter.o: ../depend_stubs/lua.h ../src/mouse.h
clutter.o: ../depend_stubs/vector ../src/keyboard.h
clutter.o: ../depend_stubs/string ../src/background_loader.h
clutter.o: ../src/vect_util.h ../depend_stubs/algorithm
clutter.o: ../src/centralised_log.h ../src/bullet_debug_drawer.h
clutter.o: ../depend_stubs/OgreFrameListener.h
clutter.o: ../depend_stubs/OgreManualObject.h
clutter.o: ../depend_stubs/btBulletCollisionCommon.h
clutter.o: ../src/gfx/gfx.h ../src/vect_util.h ../src/shared_ptr.h
clutter.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx.o: ../depend_stubs/sstream
gfx.o: ../depend_stubs/OgreAutoParamDataSource.h
gfx.o: ../../dependencies/util/sleep.h ../src/path_util.h
gfx.o: ../depend_stubs/vector ../depend_stubs/string ../src/main.h
gfx.o: ../depend_stubs/lua.h ../src/mouse.h ../src/keyboard.h
gfx.o: ../src/background_loader.h ../src/vect_util.h
gfx.o: ../depend_stubs/algorithm ../src/centralised_log.h
gfx.o: ../src/bullet_debug_drawer.h
gfx.o: ../depend_stubs/OgreFrameListener.h
gfx.o: ../depend_stubs/OgreManualObject.h
gfx.o: ../depend_stubs/btBulletCollisionCommon.h
gfx.o: ../src/clipboard.h ../src/gfx/gfx_internal.h
gfx.o: ../depend_stubs/Ogre.h ../depend_stubs/OgreMeshManager.h
gfx.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx.o: ../depend_stubs/OgreVector3.h
gfx.o: ../depend_stubs/OgreQuaternion.h
gfx.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx.o: ../depend_stubs/OgreCompositor.h
gfx.o: ../depend_stubs/OgreOctreePlugin.h
gfx.o: ../depend_stubs/OgreCgPlugin.h
gfx.o: ../depend_stubs/OgreRenderSystem.h
gfx.o: ../depend_stubs/OgreGLRenderSystem.h
gfx.o: ../depend_stubs/OgreGLGpuProgram.h
gfx.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx.o: ../depend_stubs/OgreGLSLProgram.h
gfx.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx.o: ../src/gfx/gfx.h ../src/vect_util.h ../src/shared_ptr.h
gfx.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_pipeline.h
gfx.o: ../src/gfx/clutter.h ../depend_stubs/cfloat
gfx.o: ../depend_stubs/OgreMovableObject.h
gfx.o: ../depend_stubs/OgreVertexIndexData.h
gfx.o: ../depend_stubs/OgreMaterial.h ../depend_stubs/OgreMesh.h
gfx.o: ../../dependencies/util/math_util.h
gfx.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx.o: ../src/cache_friendly_range_space_simd.h
gfx.o: ../depend_stubs/map ../src/sse_allocator.h
gfx.o: ../src/streamer.h ../src/gfx/gfx_material.h
gfx.o: ../depend_stubs/mutex ../src/gfx/gfx_shader.h
gfx.o: ../depend_stubs/cstdint ../depend_stubs/array
gfx.o: ../depend_stubs/unordered_map
gfx.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx.o: ../src/gfx/gfx_gasoline.h ../depend_stubs/ostream
gfx.o: ../src/centralised_log.h ../../dependencies/util/console.h
gfx.o: ../depend_stubs/set ../../dependencies/util/exception.h
gfx.o: ../../dependencies/util/intrinsics.h ../src/gfx/gfx_body.h
gfx.o: ../src/gfx/gfx_fertile_node.h ../src/gfx/gfx_node.h
gfx.o: ../depend_stubs/OgreSceneNode.h ../src/gfx/gfx_light.h
gfx.o: ../src/gfx/gfx_particle_system.h ../depend_stubs/utility
gfx.o: ../src/gfx/gfx_sky_material.h ../src/gfx/gfx_sky_body.h
gfx.o: ../src/gfx/gfx_disk_resource.h
gfx.o: ../depend_stubs/OgreResource.h
gfx.o: ../depend_stubs/OgreTexture.h ../src/background_loader.h
gfx.o: ../depend_stubs/condition_variable ../depend_stubs/list
gfx.o: ../depend_stubs/thread ../src/disk_resource.h
gfx.o: ../src/gfx/gfx_hud.h ../depend_stubs/lauxlib.h
gfx.o: ../depend_stubs/lualib.h ../src/external_table.h
gfx.o: ../src/shared_ptr.h ../src/lua_ptr.h
gfx.o: ../../dependencies/util/spline_table.h
gfx.o: ../src/grit_lua_util.h ../depend_stubs/OgreException.h
gfx.o: ../../dependencies/util/lua_util.h ../depend_stubs/limits
gfx.o: ../../dependencies/util/lua_wrappers_common.h
gfx.o: ../src/lua_ptr.h ../src/gfx/gfx_font.h
gfx.o: ../src/gfx/gfx_text_buffer.h
gfx.o: ../depend_stubs/OgreHardwareBufferManager.h
gfx.o: ../depend_stubs/OgreHardwareVertexBuffer.h
gfx.o: ../depend_stubs/OgreHardwareIndexBuffer.h
gfx_body.o: ../depend_stubs/OgreSceneManager.h
gfx_body.o: ../depend_stubs/OgreSkeleton.h
gfx_body.o: ../depend_stubs/OgreAxisAlignedBox.h
gfx_body.o: ../depend_stubs/OgreVector4.h
gfx_body.o: ../depend_stubs/OgreSkeletonInstance.h
gfx_body.o: ../depend_stubs/OgreAnimation.h
gfx_body.o: ../depend_stubs/OgreOptimisedUtil.h
gfx_body.o: ../depend_stubs/OgreSubMesh.h ../src/centralised_log.h
gfx_body.o: ../depend_stubs/iostream ../depend_stubs/sstream
gfx_body.o: ../../dependencies/util/console.h ../depend_stubs/map
gfx_body.o: ../depend_stubs/ostream ../depend_stubs/set
gfx_body.o: ../depend_stubs/string ../depend_stubs/vector
gfx_body.o: ../../dependencies/util/exception.h
gfx_body.o: ../depend_stubs/cstdlib
gfx_body.o: ../../dependencies/util/intrinsics.h
gfx_body.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
gfx_body.o: ../depend_stubs/OgreMeshManager.h
gfx_body.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_body.o: ../depend_stubs/OgreVector3.h
gfx_body.o: ../depend_stubs/OgreQuaternion.h
gfx_body.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_body.o: ../depend_stubs/OgreCompositor.h
gfx_body.o: ../depend_stubs/OgreOctreePlugin.h
gfx_body.o: ../depend_stubs/OgreCgPlugin.h
gfx_body.o: ../depend_stubs/OgreRenderSystem.h
gfx_body.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_body.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_body.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_body.o: ../depend_stubs/OgreGLSLProgram.h
gfx_body.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_body.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_body.o: ../depend_stubs/algorithm ../src/centralised_log.h
gfx_body.o: ../src/shared_ptr.h ../depend_stubs/functional
gfx_body.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_body.h
gfx_body.o: ../depend_stubs/OgreMesh.h
gfx_body.o: ../src/gfx/gfx_fertile_node.h ../src/gfx/gfx_node.h
gfx_body.o: ../depend_stubs/OgreSceneNode.h
gfx_body.o: ../../dependencies/util/math_util.h
gfx_body.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_body.o: ../src/gfx/gfx_material.h ../depend_stubs/mutex
gfx_body.o: ../depend_stubs/OgreMaterial.h ../src/gfx/gfx_shader.h
gfx_body.o: ../depend_stubs/cstdint ../depend_stubs/array
gfx_body.o: ../depend_stubs/unordered_map
gfx_body.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_body.o: ../src/gfx/gfx_gasoline.h ../src/gfx/gfx_pipeline.h
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
gfx_disk_resource.o: ../src/centralised_log.h
gfx_disk_resource.o: ../depend_stubs/sstream
gfx_disk_resource.o: ../../dependencies/util/console.h
gfx_disk_resource.o: ../depend_stubs/map ../depend_stubs/ostream
gfx_disk_resource.o: ../depend_stubs/set ../depend_stubs/vector
gfx_disk_resource.o: ../../dependencies/util/exception.h
gfx_disk_resource.o: ../depend_stubs/cstdlib
gfx_disk_resource.o: ../../dependencies/util/intrinsics.h
gfx_disk_resource.o: ../src/background_loader.h ../src/vect_util.h
gfx_disk_resource.o: ../depend_stubs/algorithm
gfx_disk_resource.o: ../src/centralised_log.h
gfx_disk_resource.o: ../depend_stubs/condition_variable
gfx_disk_resource.o: ../depend_stubs/list ../depend_stubs/mutex
gfx_disk_resource.o: ../depend_stubs/thread ../src/disk_resource.h
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
gfx_disk_resource.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_disk_resource.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_disk_resource.o: ../depend_stubs/OgreGLSLProgram.h
gfx_disk_resource.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_disk_resource.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_disk_resource.o: ../src/shared_ptr.h
gfx_disk_resource.o: ../depend_stubs/functional
gfx_disk_resource.o: ../src/gfx/gfx_option.h
gfx_disk_resource.o: ../src/gfx/gfx_material.h
gfx_disk_resource.o: ../depend_stubs/OgreMaterial.h
gfx_disk_resource.o: ../src/gfx/gfx_shader.h
gfx_disk_resource.o: ../depend_stubs/cstdint ../depend_stubs/array
gfx_disk_resource.o: ../depend_stubs/unordered_map
gfx_disk_resource.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_disk_resource.o: ../src/gfx/gfx_gasoline.h
gfx_disk_resource.o: ../src/gfx/gfx_pipeline.h
gfx_disk_resource.o: ../src/gfx/gfx_body.h
gfx_disk_resource.o: ../src/gfx/gfx_fertile_node.h
gfx_disk_resource.o: ../src/gfx/gfx_node.h
gfx_disk_resource.o: ../depend_stubs/OgreSceneNode.h
gfx_disk_resource.o: ../src/gfx/gfx_sky_body.h
gfx_disk_resource.o: ../src/gfx/gfx_sky_material.h
gfx_fertile_node.o: ../src/gfx/gfx_fertile_node.h
gfx_fertile_node.o: ../src/shared_ptr.h ../depend_stubs/cstdlib
gfx_fertile_node.o: ../depend_stubs/functional
gfx_fertile_node.o: ../src/gfx/gfx_node.h
gfx_fertile_node.o: ../depend_stubs/OgreSceneNode.h
gfx_fertile_node.o: ../../dependencies/util/math_util.h
gfx_fertile_node.o: ../depend_stubs/string
gfx_fertile_node.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx_fertile_node.o: ../depend_stubs/cfloat ../src/vect_util.h
gfx_fertile_node.o: ../depend_stubs/vector
gfx_fertile_node.o: ../depend_stubs/algorithm
gfx_fertile_node.o: ../src/centralised_log.h
gfx_fertile_node.o: ../src/gfx/gfx_internal.h
gfx_fertile_node.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
gfx_fertile_node.o: ../depend_stubs/OgreMeshManager.h
gfx_fertile_node.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_fertile_node.o: ../depend_stubs/OgreVector3.h
gfx_fertile_node.o: ../depend_stubs/OgreQuaternion.h
gfx_fertile_node.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_fertile_node.o: ../depend_stubs/OgreCompositor.h
gfx_fertile_node.o: ../depend_stubs/OgreOctreePlugin.h
gfx_fertile_node.o: ../depend_stubs/OgreCgPlugin.h
gfx_fertile_node.o: ../depend_stubs/OgreRenderSystem.h
gfx_fertile_node.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_fertile_node.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_fertile_node.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_fertile_node.o: ../depend_stubs/OgreGLSLProgram.h
gfx_fertile_node.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_fertile_node.o: ../src/gfx/gfx.h ../src/gfx/gfx_option.h
gfx_font.o: ../src/gfx/gfx_font.h ../depend_stubs/vector
gfx_font.o: ../depend_stubs/map
gfx_font.o: ../../dependencies/util/math_util.h
gfx_font.o: ../depend_stubs/string ../depend_stubs/iostream
gfx_font.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_font.o: ../src/background_loader.h ../src/vect_util.h
gfx_font.o: ../depend_stubs/algorithm ../src/centralised_log.h
gfx_font.o: ../depend_stubs/condition_variable
gfx_font.o: ../depend_stubs/list ../depend_stubs/mutex
gfx_font.o: ../depend_stubs/thread ../src/disk_resource.h
gfx_font.o: ../depend_stubs/set ../src/main.h
gfx_font.o: ../depend_stubs/lua.h ../src/mouse.h ../src/keyboard.h
gfx_font.o: ../src/background_loader.h
gfx_font.o: ../src/bullet_debug_drawer.h
gfx_font.o: ../depend_stubs/OgreFrameListener.h
gfx_font.o: ../depend_stubs/OgreManualObject.h
gfx_font.o: ../depend_stubs/btBulletCollisionCommon.h
gfx_font.o: ../src/gfx/gfx_disk_resource.h
gfx_font.o: ../depend_stubs/OgreResource.h
gfx_font.o: ../depend_stubs/OgreTexture.h
gfx_font.o: ../depend_stubs/OgreMesh.h ../src/centralised_log.h
gfx_font.o: ../depend_stubs/sstream
gfx_font.o: ../../dependencies/util/console.h
gfx_font.o: ../depend_stubs/ostream
gfx_font.o: ../../dependencies/util/exception.h
gfx_font.o: ../depend_stubs/cstdlib
gfx_font.o: ../../dependencies/util/intrinsics.h
gfx_gasoline.o: ../src/gfx/gfx_gasoline.h ../depend_stubs/cstdlib
gfx_gasoline.o: ../depend_stubs/cstdint ../depend_stubs/map
gfx_gasoline.o: ../depend_stubs/ostream ../depend_stubs/string
gfx_gasoline.o: ../src/centralised_log.h ../depend_stubs/iostream
gfx_gasoline.o: ../depend_stubs/sstream
gfx_gasoline.o: ../../dependencies/util/console.h
gfx_gasoline.o: ../depend_stubs/set ../depend_stubs/vector
gfx_gasoline.o: ../../dependencies/util/exception.h
gfx_gasoline.o: ../../dependencies/util/intrinsics.h
gfx_gasoline.o: ../src/gfx/gfx_gasoline_parser.h
gfx_gasoline.o: ../src/gfx/gfx_gasoline_type_system.h
gfx_gasoline.o: ../src/gfx/gfx_gasoline_backend_gsl.h
gfx_gasoline.o: ../src/gfx/gfx_gasoline_backend_cg.h
gfx_gasoline.o: ../src/gfx/gfx_gasoline_backend_glsl.h
gfx_gasoline_backend.o: ../src/gfx/gfx_gasoline_backend.h
gfx_gasoline_backend.o: ../depend_stubs/sstream
gfx_gasoline_backend.o: ../depend_stubs/string
gfx_gasoline_backend.o: ../src/gfx/gfx_gasoline_parser.h
gfx_gasoline_backend.o: ../depend_stubs/cstdlib
gfx_gasoline_backend.o: ../depend_stubs/cstdint
gfx_gasoline_backend.o: ../depend_stubs/map ../depend_stubs/vector
gfx_gasoline_backend.o: ../../dependencies/util/exception.h
gfx_gasoline_backend.o: ../depend_stubs/iostream
gfx_gasoline_backend.o: ../../dependencies/util/intrinsics.h
gfx_gasoline_backend.o: ../src/gfx/gfx_gasoline_type_system.h
gfx_gasoline_backend.o: ../depend_stubs/set
gfx_gasoline_backend.o: ../src/gfx/gfx_gasoline.h
gfx_gasoline_backend.o: ../depend_stubs/ostream
gfx_gasoline_backend.o: ../src/centralised_log.h
gfx_gasoline_backend.o: ../../dependencies/util/console.h
gfx_gasoline_backend_cg.o: ../src/gfx/gfx_gasoline_backend.h
gfx_gasoline_backend_cg.o: ../depend_stubs/sstream
gfx_gasoline_backend_cg.o: ../depend_stubs/string
gfx_gasoline_backend_cg.o: ../src/gfx/gfx_gasoline_parser.h
gfx_gasoline_backend_cg.o: ../depend_stubs/cstdlib
gfx_gasoline_backend_cg.o: ../depend_stubs/cstdint
gfx_gasoline_backend_cg.o: ../depend_stubs/map
gfx_gasoline_backend_cg.o: ../depend_stubs/vector
gfx_gasoline_backend_cg.o: ../../dependencies/util/exception.h
gfx_gasoline_backend_cg.o: ../depend_stubs/iostream
gfx_gasoline_backend_cg.o: ../../dependencies/util/intrinsics.h
gfx_gasoline_backend_cg.o: ../src/gfx/gfx_gasoline_type_system.h
gfx_gasoline_backend_cg.o: ../depend_stubs/set
gfx_gasoline_backend_cg.o: ../src/gfx/gfx_gasoline.h
gfx_gasoline_backend_cg.o: ../depend_stubs/ostream
gfx_gasoline_backend_cg.o: ../src/centralised_log.h
gfx_gasoline_backend_cg.o: ../../dependencies/util/console.h
gfx_gasoline_backend_cg.o: ../src/gfx/gfx_gasoline_backend_cg.h
gfx_gasoline_backend_glsl.o: ../src/centralised_log.h
gfx_gasoline_backend_glsl.o: ../depend_stubs/iostream
gfx_gasoline_backend_glsl.o: ../depend_stubs/sstream
gfx_gasoline_backend_glsl.o: ../../dependencies/util/console.h
gfx_gasoline_backend_glsl.o: ../depend_stubs/map
gfx_gasoline_backend_glsl.o: ../depend_stubs/ostream
gfx_gasoline_backend_glsl.o: ../depend_stubs/set
gfx_gasoline_backend_glsl.o: ../depend_stubs/string
gfx_gasoline_backend_glsl.o: ../depend_stubs/vector
gfx_gasoline_backend_glsl.o: ../../dependencies/util/exception.h
gfx_gasoline_backend_glsl.o: ../depend_stubs/cstdlib
gfx_gasoline_backend_glsl.o: ../../dependencies/util/intrinsics.h
gfx_gasoline_backend_glsl.o: ../src/gfx/gfx_gasoline_backend.h
gfx_gasoline_backend_glsl.o: ../src/gfx/gfx_gasoline_parser.h
gfx_gasoline_backend_glsl.o: ../depend_stubs/cstdint
gfx_gasoline_backend_glsl.o: ../src/gfx/gfx_gasoline_type_system.h
gfx_gasoline_backend_glsl.o: ../src/gfx/gfx_gasoline.h
gfx_gasoline_backend_glsl.o: ../src/gfx/gfx_gasoline_backend_cg.h
gfx_gasoline_backend_gsl.o: ../depend_stubs/cstdlib
gfx_gasoline_backend_gsl.o: ../../dependencies/util/exception.h
gfx_gasoline_backend_gsl.o: ../depend_stubs/string
gfx_gasoline_backend_gsl.o: ../depend_stubs/sstream
gfx_gasoline_backend_gsl.o: ../depend_stubs/iostream
gfx_gasoline_backend_gsl.o: ../../dependencies/util/intrinsics.h
gfx_gasoline_backend_gsl.o: ../src/centralised_log.h
gfx_gasoline_backend_gsl.o: ../../dependencies/util/console.h
gfx_gasoline_backend_gsl.o: ../depend_stubs/map
gfx_gasoline_backend_gsl.o: ../depend_stubs/ostream
gfx_gasoline_backend_gsl.o: ../depend_stubs/set
gfx_gasoline_backend_gsl.o: ../depend_stubs/vector
gfx_gasoline_backend_gsl.o: ../src/gfx/gfx_gasoline_backend_gsl.h
gfx_gasoline_backend_gsl.o: ../src/gfx/gfx_gasoline_parser.h
gfx_gasoline_backend_gsl.o: ../depend_stubs/cstdint
gfx_gasoline_backend_gsl.o: ../src/gfx/gfx_gasoline_type_system.h
gfx_gasoline_backend_gsl.o: ../src/gfx/gfx_gasoline.h
gfx_gasoline_parser.o: ../depend_stubs/cstdlib
gfx_gasoline_parser.o: ../depend_stubs/list ../depend_stubs/map
gfx_gasoline_parser.o: ../depend_stubs/sstream
gfx_gasoline_parser.o: ../../dependencies/util/exception.h
gfx_gasoline_parser.o: ../depend_stubs/string
gfx_gasoline_parser.o: ../depend_stubs/iostream
gfx_gasoline_parser.o: ../../dependencies/util/intrinsics.h
gfx_gasoline_parser.o: ../src/centralised_log.h
gfx_gasoline_parser.o: ../../dependencies/util/console.h
gfx_gasoline_parser.o: ../depend_stubs/ostream ../depend_stubs/set
gfx_gasoline_parser.o: ../depend_stubs/vector
gfx_gasoline_parser.o: ../src/gfx/gfx_gasoline_parser.h
gfx_gasoline_parser.o: ../depend_stubs/cstdint
gfx_gasoline_standalone.o: ../depend_stubs/fstream
gfx_gasoline_standalone.o: ../depend_stubs/string
gfx_gasoline_standalone.o: ../depend_stubs/vector
gfx_gasoline_standalone.o: ../src/gfx/gfx_gasoline.h
gfx_gasoline_standalone.o: ../depend_stubs/cstdlib
gfx_gasoline_standalone.o: ../depend_stubs/cstdint
gfx_gasoline_standalone.o: ../depend_stubs/map
gfx_gasoline_standalone.o: ../depend_stubs/ostream
gfx_gasoline_standalone.o: ../src/centralised_log.h
gfx_gasoline_standalone.o: ../depend_stubs/iostream
gfx_gasoline_standalone.o: ../depend_stubs/sstream
gfx_gasoline_standalone.o: ../../dependencies/util/console.h
gfx_gasoline_standalone.o: ../depend_stubs/set
gfx_gasoline_standalone.o: ../../dependencies/util/exception.h
gfx_gasoline_standalone.o: ../../dependencies/util/intrinsics.h
gfx_gasoline_type_system.o: ../depend_stubs/cstdlib
gfx_gasoline_type_system.o: ../src/centralised_log.h
gfx_gasoline_type_system.o: ../depend_stubs/iostream
gfx_gasoline_type_system.o: ../depend_stubs/sstream
gfx_gasoline_type_system.o: ../../dependencies/util/console.h
gfx_gasoline_type_system.o: ../depend_stubs/map
gfx_gasoline_type_system.o: ../depend_stubs/ostream
gfx_gasoline_type_system.o: ../depend_stubs/set
gfx_gasoline_type_system.o: ../depend_stubs/string
gfx_gasoline_type_system.o: ../depend_stubs/vector
gfx_gasoline_type_system.o: ../../dependencies/util/exception.h
gfx_gasoline_type_system.o: ../../dependencies/util/intrinsics.h
gfx_gasoline_type_system.o: ../src/gfx/gfx_gasoline_parser.h
gfx_gasoline_type_system.o: ../depend_stubs/cstdint
gfx_gasoline_type_system.o: ../src/gfx/gfx_gasoline_type_system.h
gfx_gasoline_type_system.o: ../src/gfx/gfx_gasoline.h
gfx_hud.o: ../depend_stubs/OgreHighLevelGpuProgramManager.h
gfx_hud.o: ../depend_stubs/OgreCgProgram.h
gfx_hud.o: ../depend_stubs/OgreGLSLProgram.h
gfx_hud.o: ../src/centralised_log.h ../depend_stubs/iostream
gfx_hud.o: ../depend_stubs/sstream
gfx_hud.o: ../../dependencies/util/console.h ../depend_stubs/map
gfx_hud.o: ../depend_stubs/ostream ../depend_stubs/set
gfx_hud.o: ../depend_stubs/string ../depend_stubs/vector
gfx_hud.o: ../../dependencies/util/exception.h
gfx_hud.o: ../depend_stubs/cstdlib
gfx_hud.o: ../../dependencies/util/intrinsics.h ../src/path_util.h
gfx_hud.o: ../src/gfx/lua_wrappers_gfx.h ../src/grit_lua_util.h
gfx_hud.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
gfx_hud.o: ../depend_stubs/lualib.h
gfx_hud.o: ../depend_stubs/OgreException.h
gfx_hud.o: ../src/centralised_log.h
gfx_hud.o: ../../dependencies/util/lua_util.h
gfx_hud.o: ../depend_stubs/limits
gfx_hud.o: ../../dependencies/util/math_util.h
gfx_hud.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_hud.o: ../../dependencies/util/lua_wrappers_common.h
gfx_hud.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_hud.o: ../depend_stubs/algorithm ../src/shared_ptr.h
gfx_hud.o: ../depend_stubs/functional
gfx_hud.o: ../src/gfx/gfx_fertile_node.h ../src/gfx/gfx_node.h
gfx_hud.o: ../depend_stubs/OgreSceneNode.h
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
gfx_hud.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_hud.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_hud.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_hud.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_body.h
gfx_hud.o: ../depend_stubs/OgreMesh.h ../src/gfx/gfx_material.h
gfx_hud.o: ../depend_stubs/mutex ../depend_stubs/OgreMaterial.h
gfx_hud.o: ../src/gfx/gfx_shader.h ../depend_stubs/cstdint
gfx_hud.o: ../depend_stubs/array ../depend_stubs/unordered_map
gfx_hud.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_hud.o: ../src/gfx/gfx_gasoline.h ../src/gfx/gfx_pipeline.h
gfx_hud.o: ../src/gfx/gfx_sky_body.h
gfx_hud.o: ../src/gfx/gfx_disk_resource.h
gfx_hud.o: ../depend_stubs/OgreResource.h
gfx_hud.o: ../depend_stubs/OgreTexture.h
gfx_hud.o: ../src/background_loader.h ../src/vect_util.h
gfx_hud.o: ../depend_stubs/condition_variable ../depend_stubs/list
gfx_hud.o: ../depend_stubs/thread ../src/disk_resource.h
gfx_hud.o: ../src/gfx/gfx_sky_material.h ../src/gfx/gfx_light.h
gfx_hud.o: ../src/gfx/gfx_particle_system.h
gfx_hud.o: ../depend_stubs/utility ../src/gfx/gfx_instances.h
gfx_hud.o: ../depend_stubs/OgreMovableObject.h
gfx_hud.o: ../src/dense_index_map.h
gfx_hud.o: ../src/gfx/gfx_ranged_instances.h ../src/streamer.h
gfx_hud.o: ../src/cache_friendly_range_space_simd.h
gfx_hud.o: ../src/sse_allocator.h ../src/gfx/gfx_hud.h
gfx_hud.o: ../src/external_table.h ../src/shared_ptr.h
gfx_hud.o: ../src/lua_ptr.h ../../dependencies/util/spline_table.h
gfx_hud.o: ../src/lua_ptr.h ../src/gfx/gfx_font.h ../src/main.h
gfx_hud.o: ../src/mouse.h ../src/keyboard.h
gfx_hud.o: ../src/background_loader.h ../src/bullet_debug_drawer.h
gfx_hud.o: ../depend_stubs/OgreFrameListener.h
gfx_hud.o: ../depend_stubs/OgreManualObject.h
gfx_hud.o: ../depend_stubs/btBulletCollisionCommon.h
gfx_hud.o: ../src/gfx/gfx_text_buffer.h
gfx_hud.o: ../depend_stubs/OgreHardwareBufferManager.h
gfx_hud.o: ../depend_stubs/OgreHardwareVertexBuffer.h
gfx_hud.o: ../depend_stubs/OgreHardwareIndexBuffer.h
gfx_instances.o: ../depend_stubs/algorithm ../src/main.h
gfx_instances.o: ../depend_stubs/lua.h ../src/mouse.h
gfx_instances.o: ../depend_stubs/vector ../src/keyboard.h
gfx_instances.o: ../depend_stubs/string ../src/background_loader.h
gfx_instances.o: ../src/vect_util.h ../src/centralised_log.h
gfx_instances.o: ../src/bullet_debug_drawer.h
gfx_instances.o: ../depend_stubs/OgreFrameListener.h
gfx_instances.o: ../depend_stubs/OgreManualObject.h
gfx_instances.o: ../depend_stubs/btBulletCollisionCommon.h
gfx_instances.o: ../src/gfx/gfx_internal.h ../depend_stubs/sstream
gfx_instances.o: ../depend_stubs/Ogre.h
gfx_instances.o: ../depend_stubs/OgreMeshManager.h
gfx_instances.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_instances.o: ../depend_stubs/OgreVector3.h
gfx_instances.o: ../depend_stubs/OgreQuaternion.h
gfx_instances.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_instances.o: ../depend_stubs/OgreCompositor.h
gfx_instances.o: ../depend_stubs/OgreOctreePlugin.h
gfx_instances.o: ../depend_stubs/OgreCgPlugin.h
gfx_instances.o: ../depend_stubs/OgreRenderSystem.h
gfx_instances.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_instances.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_instances.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_instances.o: ../depend_stubs/OgreGLSLProgram.h
gfx_instances.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_instances.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_instances.o: ../src/shared_ptr.h ../depend_stubs/cstdlib
gfx_instances.o: ../depend_stubs/functional
gfx_instances.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_material.h
gfx_instances.o: ../depend_stubs/mutex
gfx_instances.o: ../depend_stubs/OgreMaterial.h
gfx_instances.o: ../src/gfx/gfx_shader.h ../depend_stubs/cstdint
gfx_instances.o: ../depend_stubs/array ../depend_stubs/map
gfx_instances.o: ../depend_stubs/unordered_map
gfx_instances.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_instances.o: ../../dependencies/util/math_util.h
gfx_instances.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx_instances.o: ../depend_stubs/cfloat ../src/gfx/gfx_gasoline.h
gfx_instances.o: ../depend_stubs/ostream ../src/centralised_log.h
gfx_instances.o: ../../dependencies/util/console.h
gfx_instances.o: ../depend_stubs/set
gfx_instances.o: ../../dependencies/util/exception.h
gfx_instances.o: ../../dependencies/util/intrinsics.h
gfx_instances.o: ../src/gfx/gfx_pipeline.h
gfx_instances.o: ../src/gfx/gfx_instances.h
gfx_instances.o: ../depend_stubs/OgreMovableObject.h
gfx_instances.o: ../src/dense_index_map.h
gfx_instances.o: ../src/gfx/gfx_disk_resource.h
gfx_instances.o: ../depend_stubs/OgreResource.h
gfx_instances.o: ../depend_stubs/OgreTexture.h
gfx_instances.o: ../depend_stubs/OgreMesh.h
gfx_instances.o: ../src/background_loader.h
gfx_instances.o: ../depend_stubs/condition_variable
gfx_instances.o: ../depend_stubs/list ../depend_stubs/thread
gfx_instances.o: ../src/disk_resource.h ../src/gfx/gfx_node.h
gfx_instances.o: ../depend_stubs/OgreSceneNode.h
gfx_instances.o: ../src/gfx/gfx_fertile_node.h
gfx_light.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
gfx_light.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
gfx_light.o: ../depend_stubs/OgreMeshManager.h
gfx_light.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_light.o: ../depend_stubs/OgreVector3.h
gfx_light.o: ../depend_stubs/OgreQuaternion.h
gfx_light.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_light.o: ../depend_stubs/OgreCompositor.h
gfx_light.o: ../depend_stubs/OgreOctreePlugin.h
gfx_light.o: ../depend_stubs/OgreCgPlugin.h
gfx_light.o: ../depend_stubs/OgreRenderSystem.h
gfx_light.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_light.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_light.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_light.o: ../depend_stubs/OgreGLSLProgram.h
gfx_light.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_light.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_light.o: ../depend_stubs/vector ../depend_stubs/algorithm
gfx_light.o: ../src/centralised_log.h ../src/shared_ptr.h
gfx_light.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx_light.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_light.h
gfx_light.o: ../src/gfx/gfx_fertile_node.h ../src/gfx/gfx_node.h
gfx_light.o: ../depend_stubs/OgreSceneNode.h
gfx_light.o: ../../dependencies/util/math_util.h
gfx_light.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx_light.o: ../depend_stubs/cfloat
gfx_light.o: ../src/gfx/gfx_particle_system.h
gfx_light.o: ../depend_stubs/utility
gfx_material.o: ../depend_stubs/set ../src/gfx/gfx_internal.h
gfx_material.o: ../depend_stubs/string ../depend_stubs/sstream
gfx_material.o: ../depend_stubs/Ogre.h
gfx_material.o: ../depend_stubs/OgreMeshManager.h
gfx_material.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_material.o: ../depend_stubs/OgreVector3.h
gfx_material.o: ../depend_stubs/OgreQuaternion.h
gfx_material.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_material.o: ../depend_stubs/OgreCompositor.h
gfx_material.o: ../depend_stubs/OgreOctreePlugin.h
gfx_material.o: ../depend_stubs/OgreCgPlugin.h
gfx_material.o: ../depend_stubs/OgreRenderSystem.h
gfx_material.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_material.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_material.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_material.o: ../depend_stubs/OgreGLSLProgram.h
gfx_material.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_material.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_material.o: ../depend_stubs/vector ../depend_stubs/algorithm
gfx_material.o: ../src/centralised_log.h ../src/shared_ptr.h
gfx_material.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx_material.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_material.h
gfx_material.o: ../depend_stubs/mutex
gfx_material.o: ../depend_stubs/OgreMaterial.h
gfx_material.o: ../src/gfx/gfx_shader.h ../depend_stubs/cstdint
gfx_material.o: ../depend_stubs/array ../depend_stubs/map
gfx_material.o: ../depend_stubs/unordered_map
gfx_material.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_material.o: ../../dependencies/util/math_util.h
gfx_material.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx_material.o: ../depend_stubs/cfloat ../src/gfx/gfx_gasoline.h
gfx_material.o: ../depend_stubs/ostream ../src/centralised_log.h
gfx_material.o: ../../dependencies/util/console.h
gfx_material.o: ../../dependencies/util/exception.h
gfx_material.o: ../../dependencies/util/intrinsics.h
gfx_material.o: ../src/gfx/gfx_pipeline.h ../src/gfx/gfx_body.h
gfx_material.o: ../depend_stubs/OgreMesh.h
gfx_material.o: ../src/gfx/gfx_fertile_node.h
gfx_material.o: ../src/gfx/gfx_node.h
gfx_material.o: ../depend_stubs/OgreSceneNode.h
gfx_node.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
gfx_node.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
gfx_node.o: ../depend_stubs/OgreMeshManager.h
gfx_node.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_node.o: ../depend_stubs/OgreVector3.h
gfx_node.o: ../depend_stubs/OgreQuaternion.h
gfx_node.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_node.o: ../depend_stubs/OgreCompositor.h
gfx_node.o: ../depend_stubs/OgreOctreePlugin.h
gfx_node.o: ../depend_stubs/OgreCgPlugin.h
gfx_node.o: ../depend_stubs/OgreRenderSystem.h
gfx_node.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_node.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_node.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_node.o: ../depend_stubs/OgreGLSLProgram.h
gfx_node.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_node.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_node.o: ../depend_stubs/vector ../depend_stubs/algorithm
gfx_node.o: ../src/centralised_log.h ../src/shared_ptr.h
gfx_node.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx_node.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_node.h
gfx_node.o: ../depend_stubs/OgreSceneNode.h
gfx_node.o: ../../dependencies/util/math_util.h
gfx_node.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx_node.o: ../depend_stubs/cfloat ../src/gfx/gfx_fertile_node.h
gfx_node.o: ../src/gfx/gfx_body.h ../depend_stubs/OgreMesh.h
gfx_node.o: ../src/gfx/gfx_material.h ../depend_stubs/mutex
gfx_node.o: ../depend_stubs/OgreMaterial.h ../src/gfx/gfx_shader.h
gfx_node.o: ../depend_stubs/cstdint ../depend_stubs/array
gfx_node.o: ../depend_stubs/map ../depend_stubs/unordered_map
gfx_node.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_node.o: ../src/gfx/gfx_gasoline.h ../depend_stubs/ostream
gfx_node.o: ../src/centralised_log.h
gfx_node.o: ../../dependencies/util/console.h ../depend_stubs/set
gfx_node.o: ../../dependencies/util/exception.h
gfx_node.o: ../../dependencies/util/intrinsics.h
gfx_node.o: ../src/gfx/gfx_pipeline.h
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
gfx_option.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_option.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_option.o: ../depend_stubs/OgreGLSLProgram.h
gfx_option.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_option.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_option.o: ../depend_stubs/vector ../depend_stubs/algorithm
gfx_option.o: ../src/centralised_log.h ../src/shared_ptr.h
gfx_option.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx_option.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_pipeline.h
gfx_option.o: ../src/option.h ../depend_stubs/ostream
gfx_option.o: ../../dependencies/util/exception.h
gfx_option.o: ../depend_stubs/iostream
gfx_option.o: ../../dependencies/util/intrinsics.h
gfx_option.o: ../src/centralised_log.h
gfx_option.o: ../../dependencies/util/console.h
gfx_option.o: ../depend_stubs/set
gfx_particle_system.o: ../depend_stubs/string
gfx_particle_system.o: ../depend_stubs/algorithm
gfx_particle_system.o: ../depend_stubs/map
gfx_particle_system.o: ../../dependencies/util/math_util.h
gfx_particle_system.o: ../depend_stubs/iostream
gfx_particle_system.o: ../depend_stubs/cmath
gfx_particle_system.o: ../depend_stubs/cfloat ../src/vect_util.h
gfx_particle_system.o: ../depend_stubs/vector
gfx_particle_system.o: ../src/centralised_log.h ../src/gfx/gfx.h
gfx_particle_system.o: ../src/shared_ptr.h ../depend_stubs/cstdlib
gfx_particle_system.o: ../depend_stubs/functional
gfx_particle_system.o: ../src/gfx/gfx_internal.h
gfx_particle_system.o: ../depend_stubs/sstream
gfx_particle_system.o: ../depend_stubs/Ogre.h
gfx_particle_system.o: ../depend_stubs/OgreMeshManager.h
gfx_particle_system.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_particle_system.o: ../depend_stubs/OgreVector3.h
gfx_particle_system.o: ../depend_stubs/OgreQuaternion.h
gfx_particle_system.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_particle_system.o: ../depend_stubs/OgreCompositor.h
gfx_particle_system.o: ../depend_stubs/OgreOctreePlugin.h
gfx_particle_system.o: ../depend_stubs/OgreCgPlugin.h
gfx_particle_system.o: ../depend_stubs/OgreRenderSystem.h
gfx_particle_system.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_particle_system.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_particle_system.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_particle_system.o: ../depend_stubs/OgreGLSLProgram.h
gfx_particle_system.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_particle_system.o: ../src/gfx/gfx_option.h
gfx_particle_system.o: ../src/gfx/gfx_particle_system.h
gfx_particle_system.o: ../depend_stubs/utility
gfx_particle_system.o: ../src/gfx/gfx_pipeline.h
gfx_particle_system.o: ../src/gfx/gfx_shader.h
gfx_particle_system.o: ../depend_stubs/cstdint
gfx_particle_system.o: ../depend_stubs/array
gfx_particle_system.o: ../depend_stubs/unordered_map
gfx_particle_system.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_particle_system.o: ../src/gfx/gfx_gasoline.h
gfx_particle_system.o: ../depend_stubs/ostream
gfx_particle_system.o: ../src/centralised_log.h
gfx_particle_system.o: ../../dependencies/util/console.h
gfx_particle_system.o: ../depend_stubs/set
gfx_particle_system.o: ../../dependencies/util/exception.h
gfx_particle_system.o: ../../dependencies/util/intrinsics.h
gfx_particle_system.o: ../src/gfx/gfx_material.h
gfx_particle_system.o: ../depend_stubs/mutex
gfx_particle_system.o: ../depend_stubs/OgreMaterial.h
gfx_pipeline.o: ../depend_stubs/OgreString.h
gfx_pipeline.o: ../../dependencies/util/math_util.h
gfx_pipeline.o: ../depend_stubs/string ../depend_stubs/iostream
gfx_pipeline.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_pipeline.o: ../../dependencies/util/sleep.h
gfx_pipeline.o: ../src/gfx/gfx_pipeline.h ../src/shared_ptr.h
gfx_pipeline.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx_pipeline.o: ../src/vect_util.h ../depend_stubs/vector
gfx_pipeline.o: ../depend_stubs/algorithm ../src/centralised_log.h
gfx_pipeline.o: ../src/gfx/gfx_internal.h ../depend_stubs/sstream
gfx_pipeline.o: ../depend_stubs/Ogre.h
gfx_pipeline.o: ../depend_stubs/OgreMeshManager.h
gfx_pipeline.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_pipeline.o: ../depend_stubs/OgreVector3.h
gfx_pipeline.o: ../depend_stubs/OgreQuaternion.h
gfx_pipeline.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_pipeline.o: ../depend_stubs/OgreCompositor.h
gfx_pipeline.o: ../depend_stubs/OgreOctreePlugin.h
gfx_pipeline.o: ../depend_stubs/OgreCgPlugin.h
gfx_pipeline.o: ../depend_stubs/OgreRenderSystem.h
gfx_pipeline.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_pipeline.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_pipeline.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_pipeline.o: ../depend_stubs/OgreGLSLProgram.h
gfx_pipeline.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_pipeline.o: ../src/gfx/gfx.h ../src/gfx/gfx_option.h
gfx_pipeline.o: ../src/gfx/gfx_particle_system.h
gfx_pipeline.o: ../depend_stubs/utility ../src/gfx/gfx_sky_body.h
gfx_pipeline.o: ../depend_stubs/OgreMesh.h
gfx_pipeline.o: ../depend_stubs/OgreSceneNode.h
gfx_pipeline.o: ../src/gfx/gfx_disk_resource.h
gfx_pipeline.o: ../depend_stubs/OgreResource.h
gfx_pipeline.o: ../depend_stubs/OgreTexture.h
gfx_pipeline.o: ../src/centralised_log.h
gfx_pipeline.o: ../../dependencies/util/console.h
gfx_pipeline.o: ../depend_stubs/map ../depend_stubs/ostream
gfx_pipeline.o: ../depend_stubs/set
gfx_pipeline.o: ../../dependencies/util/exception.h
gfx_pipeline.o: ../../dependencies/util/intrinsics.h
gfx_pipeline.o: ../src/background_loader.h ../src/vect_util.h
gfx_pipeline.o: ../depend_stubs/condition_variable
gfx_pipeline.o: ../depend_stubs/list ../depend_stubs/mutex
gfx_pipeline.o: ../depend_stubs/thread ../src/disk_resource.h
gfx_pipeline.o: ../src/gfx/gfx_sky_material.h
gfx_pipeline.o: ../depend_stubs/OgreMaterial.h
gfx_pipeline.o: ../src/gfx/gfx_material.h ../src/gfx/gfx_shader.h
gfx_pipeline.o: ../depend_stubs/cstdint ../depend_stubs/array
gfx_pipeline.o: ../depend_stubs/unordered_map
gfx_pipeline.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_pipeline.o: ../src/gfx/gfx_gasoline.h ../src/gfx/gfx_body.h
gfx_pipeline.o: ../src/gfx/gfx_fertile_node.h
gfx_pipeline.o: ../src/gfx/gfx_node.h
gfx_ranged_instances.o: ../depend_stubs/algorithm ../src/main.h
gfx_ranged_instances.o: ../depend_stubs/lua.h ../src/mouse.h
gfx_ranged_instances.o: ../depend_stubs/vector ../src/keyboard.h
gfx_ranged_instances.o: ../depend_stubs/string
gfx_ranged_instances.o: ../src/background_loader.h
gfx_ranged_instances.o: ../src/vect_util.h
gfx_ranged_instances.o: ../src/centralised_log.h
gfx_ranged_instances.o: ../src/bullet_debug_drawer.h
gfx_ranged_instances.o: ../depend_stubs/OgreFrameListener.h
gfx_ranged_instances.o: ../depend_stubs/OgreManualObject.h
gfx_ranged_instances.o: ../depend_stubs/btBulletCollisionCommon.h
gfx_ranged_instances.o: ../src/gfx/gfx_internal.h
gfx_ranged_instances.o: ../depend_stubs/sstream
gfx_ranged_instances.o: ../depend_stubs/Ogre.h
gfx_ranged_instances.o: ../depend_stubs/OgreMeshManager.h
gfx_ranged_instances.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_ranged_instances.o: ../depend_stubs/OgreVector3.h
gfx_ranged_instances.o: ../depend_stubs/OgreQuaternion.h
gfx_ranged_instances.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_ranged_instances.o: ../depend_stubs/OgreCompositor.h
gfx_ranged_instances.o: ../depend_stubs/OgreOctreePlugin.h
gfx_ranged_instances.o: ../depend_stubs/OgreCgPlugin.h
gfx_ranged_instances.o: ../depend_stubs/OgreRenderSystem.h
gfx_ranged_instances.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_ranged_instances.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_ranged_instances.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_ranged_instances.o: ../depend_stubs/OgreGLSLProgram.h
gfx_ranged_instances.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_ranged_instances.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_ranged_instances.o: ../src/shared_ptr.h
gfx_ranged_instances.o: ../depend_stubs/cstdlib
gfx_ranged_instances.o: ../depend_stubs/functional
gfx_ranged_instances.o: ../src/gfx/gfx_option.h
gfx_ranged_instances.o: ../src/gfx/gfx_ranged_instances.h
gfx_ranged_instances.o: ../src/streamer.h
gfx_ranged_instances.o: ../src/cache_friendly_range_space_simd.h
gfx_ranged_instances.o: ../depend_stubs/map
gfx_ranged_instances.o: ../depend_stubs/iostream
gfx_ranged_instances.o: ../depend_stubs/cmath
gfx_ranged_instances.o: ../src/sse_allocator.h
gfx_ranged_instances.o: ../src/gfx/gfx_instances.h
gfx_ranged_instances.o: ../depend_stubs/OgreMovableObject.h
gfx_ranged_instances.o: ../src/dense_index_map.h
gfx_ranged_instances.o: ../src/gfx/gfx_disk_resource.h
gfx_ranged_instances.o: ../depend_stubs/OgreResource.h
gfx_ranged_instances.o: ../depend_stubs/OgreTexture.h
gfx_ranged_instances.o: ../depend_stubs/OgreMesh.h
gfx_ranged_instances.o: ../../dependencies/util/math_util.h
gfx_ranged_instances.o: ../depend_stubs/cfloat
gfx_ranged_instances.o: ../src/centralised_log.h
gfx_ranged_instances.o: ../../dependencies/util/console.h
gfx_ranged_instances.o: ../depend_stubs/ostream
gfx_ranged_instances.o: ../depend_stubs/set
gfx_ranged_instances.o: ../../dependencies/util/exception.h
gfx_ranged_instances.o: ../../dependencies/util/intrinsics.h
gfx_ranged_instances.o: ../src/background_loader.h
gfx_ranged_instances.o: ../depend_stubs/condition_variable
gfx_ranged_instances.o: ../depend_stubs/list ../depend_stubs/mutex
gfx_ranged_instances.o: ../depend_stubs/thread
gfx_ranged_instances.o: ../src/disk_resource.h
gfx_ranged_instances.o: ../src/gfx/gfx_node.h
gfx_ranged_instances.o: ../depend_stubs/OgreSceneNode.h
gfx_ranged_instances.o: ../src/gfx/gfx_fertile_node.h
gfx_shader.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_shader.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_shader.o: ../depend_stubs/OgreGLSLProgram.h
gfx_shader.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_shader.o: ../depend_stubs/OgreCgProgram.h
gfx_shader.o: ../src/centralised_log.h ../depend_stubs/iostream
gfx_shader.o: ../depend_stubs/sstream
gfx_shader.o: ../../dependencies/util/console.h
gfx_shader.o: ../depend_stubs/map ../depend_stubs/ostream
gfx_shader.o: ../depend_stubs/set ../depend_stubs/string
gfx_shader.o: ../depend_stubs/vector
gfx_shader.o: ../../dependencies/util/exception.h
gfx_shader.o: ../depend_stubs/cstdlib
gfx_shader.o: ../../dependencies/util/intrinsics.h
gfx_shader.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_shader.o: ../depend_stubs/algorithm ../src/centralised_log.h
gfx_shader.o: ../src/shared_ptr.h ../depend_stubs/functional
gfx_shader.o: ../src/gfx/gfx_gasoline.h ../depend_stubs/cstdint
gfx_shader.o: ../src/gfx/gfx_internal.h ../depend_stubs/Ogre.h
gfx_shader.o: ../depend_stubs/OgreMeshManager.h
gfx_shader.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_shader.o: ../depend_stubs/OgreVector3.h
gfx_shader.o: ../depend_stubs/OgreQuaternion.h
gfx_shader.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_shader.o: ../depend_stubs/OgreCompositor.h
gfx_shader.o: ../depend_stubs/OgreOctreePlugin.h
gfx_shader.o: ../depend_stubs/OgreCgPlugin.h
gfx_shader.o: ../depend_stubs/OgreRenderSystem.h
gfx_shader.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_shader.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_shader.h
gfx_shader.o: ../depend_stubs/array ../depend_stubs/unordered_map
gfx_shader.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_shader.o: ../../dependencies/util/math_util.h
gfx_shader.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_shader.o: ../src/gfx/gfx_pipeline.h ../src/gfx/gfx_material.h
gfx_shader.o: ../depend_stubs/mutex ../depend_stubs/OgreMaterial.h
gfx_sky_body.o: ../depend_stubs/OgreMeshManager.h
gfx_sky_body.o: ../src/gfx/gfx_internal.h ../depend_stubs/string
gfx_sky_body.o: ../depend_stubs/sstream ../depend_stubs/Ogre.h
gfx_sky_body.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_sky_body.o: ../depend_stubs/OgreVector3.h
gfx_sky_body.o: ../depend_stubs/OgreQuaternion.h
gfx_sky_body.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_sky_body.o: ../depend_stubs/OgreCompositor.h
gfx_sky_body.o: ../depend_stubs/OgreOctreePlugin.h
gfx_sky_body.o: ../depend_stubs/OgreCgPlugin.h
gfx_sky_body.o: ../depend_stubs/OgreRenderSystem.h
gfx_sky_body.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_sky_body.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_sky_body.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_sky_body.o: ../depend_stubs/OgreGLSLProgram.h
gfx_sky_body.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_sky_body.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_sky_body.o: ../depend_stubs/vector ../depend_stubs/algorithm
gfx_sky_body.o: ../src/centralised_log.h ../src/shared_ptr.h
gfx_sky_body.o: ../depend_stubs/cstdlib ../depend_stubs/functional
gfx_sky_body.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_sky_body.h
gfx_sky_body.o: ../depend_stubs/OgreMesh.h
gfx_sky_body.o: ../depend_stubs/OgreSceneNode.h
gfx_sky_body.o: ../../dependencies/util/math_util.h
gfx_sky_body.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx_sky_body.o: ../depend_stubs/cfloat
gfx_sky_body.o: ../src/gfx/gfx_disk_resource.h
gfx_sky_body.o: ../depend_stubs/OgreResource.h
gfx_sky_body.o: ../depend_stubs/OgreTexture.h
gfx_sky_body.o: ../src/centralised_log.h
gfx_sky_body.o: ../../dependencies/util/console.h
gfx_sky_body.o: ../depend_stubs/map ../depend_stubs/ostream
gfx_sky_body.o: ../depend_stubs/set
gfx_sky_body.o: ../../dependencies/util/exception.h
gfx_sky_body.o: ../../dependencies/util/intrinsics.h
gfx_sky_body.o: ../src/background_loader.h ../src/vect_util.h
gfx_sky_body.o: ../depend_stubs/condition_variable
gfx_sky_body.o: ../depend_stubs/list ../depend_stubs/mutex
gfx_sky_body.o: ../depend_stubs/thread ../src/disk_resource.h
gfx_sky_body.o: ../src/gfx/gfx_sky_material.h
gfx_sky_body.o: ../depend_stubs/OgreMaterial.h
gfx_sky_body.o: ../src/gfx/gfx_material.h ../src/gfx/gfx_shader.h
gfx_sky_body.o: ../depend_stubs/cstdint ../depend_stubs/array
gfx_sky_body.o: ../depend_stubs/unordered_map
gfx_sky_body.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_sky_body.o: ../src/gfx/gfx_gasoline.h
gfx_sky_body.o: ../src/gfx/gfx_pipeline.h
gfx_sky_material.o: ../depend_stubs/string ../depend_stubs/map
gfx_sky_material.o: ../depend_stubs/OgreMaterial.h
gfx_sky_material.o: ../depend_stubs/OgreCgProgram.h
gfx_sky_material.o: ../src/centralised_log.h
gfx_sky_material.o: ../depend_stubs/iostream
gfx_sky_material.o: ../depend_stubs/sstream
gfx_sky_material.o: ../../dependencies/util/console.h
gfx_sky_material.o: ../depend_stubs/ostream ../depend_stubs/set
gfx_sky_material.o: ../depend_stubs/vector
gfx_sky_material.o: ../../dependencies/util/exception.h
gfx_sky_material.o: ../depend_stubs/cstdlib
gfx_sky_material.o: ../../dependencies/util/intrinsics.h
gfx_sky_material.o: ../src/gfx/gfx_internal.h
gfx_sky_material.o: ../depend_stubs/Ogre.h
gfx_sky_material.o: ../depend_stubs/OgreMeshManager.h
gfx_sky_material.o: ../depend_stubs/OgreOctreeSceneManager.h
gfx_sky_material.o: ../depend_stubs/OgreVector3.h
gfx_sky_material.o: ../depend_stubs/OgreQuaternion.h
gfx_sky_material.o: ../depend_stubs/OgreCustomCompositionPass.h
gfx_sky_material.o: ../depend_stubs/OgreCompositor.h
gfx_sky_material.o: ../depend_stubs/OgreOctreePlugin.h
gfx_sky_material.o: ../depend_stubs/OgreCgPlugin.h
gfx_sky_material.o: ../depend_stubs/OgreRenderSystem.h
gfx_sky_material.o: ../depend_stubs/OgreGLRenderSystem.h
gfx_sky_material.o: ../depend_stubs/OgreGLGpuProgram.h
gfx_sky_material.o: ../depend_stubs/OgreGLSLGpuProgram.h
gfx_sky_material.o: ../depend_stubs/OgreGLSLProgram.h
gfx_sky_material.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
gfx_sky_material.o: ../src/gfx/gfx.h ../src/vect_util.h
gfx_sky_material.o: ../depend_stubs/algorithm
gfx_sky_material.o: ../src/centralised_log.h ../src/shared_ptr.h
gfx_sky_material.o: ../depend_stubs/functional
gfx_sky_material.o: ../src/gfx/gfx_option.h
gfx_sky_material.o: ../src/gfx/gfx_sky_material.h
gfx_sky_material.o: ../src/gfx/gfx_material.h
gfx_sky_material.o: ../depend_stubs/mutex ../src/gfx/gfx_shader.h
gfx_sky_material.o: ../depend_stubs/cstdint ../depend_stubs/array
gfx_sky_material.o: ../depend_stubs/unordered_map
gfx_sky_material.o: ../depend_stubs/OgreHighLevelGpuProgram.h
gfx_sky_material.o: ../../dependencies/util/math_util.h
gfx_sky_material.o: ../depend_stubs/cmath ../depend_stubs/cfloat
gfx_sky_material.o: ../src/gfx/gfx_gasoline.h
gfx_sky_material.o: ../src/gfx/gfx_pipeline.h
gfx_sky_material.o: ../src/gfx/gfx_sky_body.h
gfx_sky_material.o: ../depend_stubs/OgreMesh.h
gfx_sky_material.o: ../depend_stubs/OgreSceneNode.h
gfx_sky_material.o: ../src/gfx/gfx_disk_resource.h
gfx_sky_material.o: ../depend_stubs/OgreResource.h
gfx_sky_material.o: ../depend_stubs/OgreTexture.h
gfx_sky_material.o: ../src/background_loader.h ../src/vect_util.h
gfx_sky_material.o: ../depend_stubs/condition_variable
gfx_sky_material.o: ../depend_stubs/list ../depend_stubs/thread
gfx_sky_material.o: ../src/disk_resource.h
gfx_text_buffer.o: ../../dependencies/util/unicode_util.h
gfx_text_buffer.o: ../depend_stubs/cstdlib ../depend_stubs/string
gfx_text_buffer.o: ../src/gfx/gfx_text_buffer.h
gfx_text_buffer.o: ../depend_stubs/cstdint
gfx_text_buffer.o: ../depend_stubs/OgreHardwareBufferManager.h
gfx_text_buffer.o: ../depend_stubs/OgreHardwareVertexBuffer.h
gfx_text_buffer.o: ../depend_stubs/OgreHardwareIndexBuffer.h
gfx_text_buffer.o: ../../dependencies/util/math_util.h
gfx_text_buffer.o: ../depend_stubs/iostream ../depend_stubs/cmath
gfx_text_buffer.o: ../depend_stubs/cfloat ../src/gfx/gfx_font.h
gfx_text_buffer.o: ../depend_stubs/vector ../depend_stubs/map
gfx_text_buffer.o: ../src/background_loader.h ../src/vect_util.h
gfx_text_buffer.o: ../depend_stubs/algorithm
gfx_text_buffer.o: ../src/centralised_log.h
gfx_text_buffer.o: ../depend_stubs/condition_variable
gfx_text_buffer.o: ../depend_stubs/list ../depend_stubs/mutex
gfx_text_buffer.o: ../depend_stubs/thread ../src/disk_resource.h
gfx_text_buffer.o: ../depend_stubs/set ../src/main.h
gfx_text_buffer.o: ../depend_stubs/lua.h ../src/mouse.h
gfx_text_buffer.o: ../src/keyboard.h ../src/background_loader.h
gfx_text_buffer.o: ../src/bullet_debug_drawer.h
gfx_text_buffer.o: ../depend_stubs/OgreFrameListener.h
gfx_text_buffer.o: ../depend_stubs/OgreManualObject.h
gfx_text_buffer.o: ../depend_stubs/btBulletCollisionCommon.h
gfx_text_buffer.o: ../src/gfx/gfx_disk_resource.h
gfx_text_buffer.o: ../depend_stubs/OgreResource.h
gfx_text_buffer.o: ../depend_stubs/OgreTexture.h
gfx_text_buffer.o: ../depend_stubs/OgreMesh.h
gfx_text_buffer.o: ../src/centralised_log.h
gfx_text_buffer.o: ../depend_stubs/sstream
gfx_text_buffer.o: ../../dependencies/util/console.h
gfx_text_buffer.o: ../depend_stubs/ostream
gfx_text_buffer.o: ../../dependencies/util/exception.h
gfx_text_buffer.o: ../../dependencies/util/intrinsics.h
lua_wrappers_gfx.o: ../../dependencies/util/colour_conversion.h
lua_wrappers_gfx.o: ../../dependencies/util/unicode_util.h
lua_wrappers_gfx.o: ../depend_stubs/cstdlib ../depend_stubs/string
lua_wrappers_gfx.o: ../src/grit_lua_util.h ../depend_stubs/lua.h
lua_wrappers_gfx.o: ../depend_stubs/lauxlib.h
lua_wrappers_gfx.o: ../depend_stubs/lualib.h
lua_wrappers_gfx.o: ../depend_stubs/OgreException.h
lua_wrappers_gfx.o: ../src/centralised_log.h
lua_wrappers_gfx.o: ../../dependencies/util/lua_util.h
lua_wrappers_gfx.o: ../depend_stubs/sstream ../depend_stubs/limits
lua_wrappers_gfx.o: ../depend_stubs/vector
lua_wrappers_gfx.o: ../../dependencies/util/exception.h
lua_wrappers_gfx.o: ../depend_stubs/iostream
lua_wrappers_gfx.o: ../../dependencies/util/intrinsics.h
lua_wrappers_gfx.o: ../../dependencies/util/math_util.h
lua_wrappers_gfx.o: ../depend_stubs/cmath ../depend_stubs/cfloat
lua_wrappers_gfx.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_gfx.o: ../src/lua_wrappers_primitives.h
lua_wrappers_gfx.o: ../src/grit_lua_util.h
lua_wrappers_gfx.o: ../../dependencies/util/spline_table.h
lua_wrappers_gfx.o: ../depend_stubs/map ../src/main.h
lua_wrappers_gfx.o: ../src/mouse.h ../src/keyboard.h
lua_wrappers_gfx.o: ../src/background_loader.h ../src/vect_util.h
lua_wrappers_gfx.o: ../depend_stubs/algorithm
lua_wrappers_gfx.o: ../src/bullet_debug_drawer.h
lua_wrappers_gfx.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_gfx.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_gfx.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_gfx.o: ../src/external_table.h ../src/shared_ptr.h
lua_wrappers_gfx.o: ../src/lua_ptr.h ../src/lua_ptr.h
lua_wrappers_gfx.o: ../src/path_util.h
lua_wrappers_gfx.o: ../src/gfx/lua_wrappers_gfx.h ../src/gfx/gfx.h
lua_wrappers_gfx.o: ../src/vect_util.h ../src/shared_ptr.h
lua_wrappers_gfx.o: ../depend_stubs/functional
lua_wrappers_gfx.o: ../src/gfx/gfx_fertile_node.h
lua_wrappers_gfx.o: ../src/gfx/gfx_node.h
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
lua_wrappers_gfx.o: ../depend_stubs/OgreGLGpuProgram.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGLSLGpuProgram.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGLSLProgram.h
lua_wrappers_gfx.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
lua_wrappers_gfx.o: ../src/gfx/gfx_option.h ../src/gfx/gfx_body.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMesh.h
lua_wrappers_gfx.o: ../src/gfx/gfx_material.h
lua_wrappers_gfx.o: ../depend_stubs/mutex
lua_wrappers_gfx.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_gfx.o: ../src/gfx/gfx_shader.h
lua_wrappers_gfx.o: ../depend_stubs/cstdint ../depend_stubs/array
lua_wrappers_gfx.o: ../depend_stubs/unordered_map
lua_wrappers_gfx.o: ../depend_stubs/OgreHighLevelGpuProgram.h
lua_wrappers_gfx.o: ../src/gfx/gfx_gasoline.h
lua_wrappers_gfx.o: ../depend_stubs/ostream
lua_wrappers_gfx.o: ../src/centralised_log.h
lua_wrappers_gfx.o: ../../dependencies/util/console.h
lua_wrappers_gfx.o: ../depend_stubs/set ../src/gfx/gfx_pipeline.h
lua_wrappers_gfx.o: ../src/gfx/gfx_sky_body.h
lua_wrappers_gfx.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_gfx.o: ../depend_stubs/OgreResource.h
lua_wrappers_gfx.o: ../depend_stubs/OgreTexture.h
lua_wrappers_gfx.o: ../src/background_loader.h
lua_wrappers_gfx.o: ../depend_stubs/condition_variable
lua_wrappers_gfx.o: ../depend_stubs/list ../depend_stubs/thread
lua_wrappers_gfx.o: ../src/disk_resource.h
lua_wrappers_gfx.o: ../src/gfx/gfx_sky_material.h
lua_wrappers_gfx.o: ../src/gfx/gfx_light.h
lua_wrappers_gfx.o: ../src/gfx/gfx_particle_system.h
lua_wrappers_gfx.o: ../depend_stubs/utility
lua_wrappers_gfx.o: ../src/gfx/gfx_instances.h
lua_wrappers_gfx.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_gfx.o: ../src/dense_index_map.h
lua_wrappers_gfx.o: ../src/gfx/gfx_ranged_instances.h
lua_wrappers_gfx.o: ../src/streamer.h
lua_wrappers_gfx.o: ../src/cache_friendly_range_space_simd.h
lua_wrappers_gfx.o: ../src/sse_allocator.h ../src/gfx/gfx_hud.h
lua_wrappers_gfx.o: ../src/gfx/gfx_font.h
lua_wrappers_gfx.o: ../src/gfx/gfx_text_buffer.h
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
lua_wrappers_gpuprog.o: ../src/centralised_log.h
lua_wrappers_gpuprog.o: ../../dependencies/util/lua_util.h
lua_wrappers_gpuprog.o: ../depend_stubs/sstream
lua_wrappers_gpuprog.o: ../depend_stubs/limits
lua_wrappers_gpuprog.o: ../depend_stubs/vector
lua_wrappers_gpuprog.o: ../../dependencies/util/exception.h
lua_wrappers_gpuprog.o: ../depend_stubs/cstdlib
lua_wrappers_gpuprog.o: ../depend_stubs/iostream
lua_wrappers_gpuprog.o: ../../dependencies/util/intrinsics.h
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
lua_wrappers_material.o: ../src/centralised_log.h
lua_wrappers_material.o: ../../dependencies/util/lua_util.h
lua_wrappers_material.o: ../depend_stubs/sstream
lua_wrappers_material.o: ../depend_stubs/limits
lua_wrappers_material.o: ../depend_stubs/vector
lua_wrappers_material.o: ../../dependencies/util/exception.h
lua_wrappers_material.o: ../depend_stubs/cstdlib
lua_wrappers_material.o: ../depend_stubs/iostream
lua_wrappers_material.o: ../../dependencies/util/intrinsics.h
lua_wrappers_material.o: ../../dependencies/util/math_util.h
lua_wrappers_material.o: ../depend_stubs/cmath
lua_wrappers_material.o: ../depend_stubs/cfloat
lua_wrappers_material.o: ../../dependencies/util/lua_wrappers_common.h
ray.o: ../depend_stubs/float.h ../src/gfx/ray.h
ray.o: ../depend_stubs/Ogre.h ../depend_stubs/OgreSceneQuery.h
bcol_parser.o: ../depend_stubs/cmath ../depend_stubs/sstream
bcol_parser.o: ../src/portable_io.h ../depend_stubs/cstdlib
bcol_parser.o: ../depend_stubs/cstdio ../depend_stubs/cassert
bcol_parser.o: ../depend_stubs/cstring ../depend_stubs/string
bcol_parser.o: ../depend_stubs/iostream
bcol_parser.o: ../depend_stubs/fstream
bcol_parser.o: ../depend_stubs/stdint.h
bcol_parser.o: ../src/centralised_log.h
bcol_parser.o: ../src/physics/col_defaults.h
bcol_parser.o: ../src/physics/bcol_parser.h
bcol_parser.o: ../depend_stubs/vector
bcol_parser.o: ../src/physics/tcol_parser.h
bcol_parser.o: ../../dependencies/util/math_util.h
bcol_parser.o: ../depend_stubs/cfloat
bcol_parser.o: ../src/physics/tcol_lexer ../depend_stubs/map
bcol_parser.o: ../depend_stubs/istream
bcol_parser.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
bcol_parser.o: ../src/physics/tcol_lexer-token_ids
bcol_parser.o: ../depend_stubs/quex/code_base/Token
bcol_parser.o: ../depend_stubs/quex/code_base/definitions
bcol_parser.o: ../depend_stubs/quex/code_base/buffer/Buffer
bcol_parser.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
bcol_parser.o: ../depend_stubs/quex/code_base/template/QuexMode
bcol_parser.o: ../depend_stubs/quex/code_base/template/Analyser
bcol_parser.o: ../depend_stubs/quex/code_base/template/Counter
bcol_parser.o: ../depend_stubs/quex/code_base/template/Accumulator
bcol_parser.o: ../depend_stubs/quex/code_base/template/IncludeStack
bcol_parser.o: ../depend_stubs/quex/code_base/temporary_macros_on
bcol_parser.o: ../depend_stubs/quex/code_base/temporary_macros_off
bcol_parser.o: ../depend_stubs/quex/code_base/template/constructor.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/Counter.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/misc.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/buffer_access.i
bcol_parser.o: ../depend_stubs/quex/code_base/template/Analyser.i
collision_mesh.o: ../depend_stubs/iostream
collision_mesh.o: ../depend_stubs/cstdlib
collision_mesh.o: ../depend_stubs/ctime
collision_mesh.o: ../depend_stubs/OgreException.h
collision_mesh.o: ../depend_stubs/OgreTimer.h
collision_mesh.o: ../depend_stubs/OgreResourceGroupManager.h
collision_mesh.o: ../depend_stubs/LinearMath/btGeometryUtil.h
collision_mesh.o: ../depend_stubs/btBulletCollisionCommon.h
collision_mesh.o: ../depend_stubs/BulletCollision/Gimpact/btGImpactShape.h
collision_mesh.o: ../depend_stubs/BulletCollision/CollisionDispatch/btInternalEdgeUtility.h
collision_mesh.o: ../src/centralised_log.h
collision_mesh.o: ../depend_stubs/sstream
collision_mesh.o: ../../dependencies/util/console.h
collision_mesh.o: ../depend_stubs/map ../depend_stubs/ostream
collision_mesh.o: ../depend_stubs/set ../depend_stubs/string
collision_mesh.o: ../depend_stubs/vector
collision_mesh.o: ../../dependencies/util/exception.h
collision_mesh.o: ../../dependencies/util/intrinsics.h
collision_mesh.o: ../src/path_util.h
collision_mesh.o: ../src/physics/collision_mesh.h
collision_mesh.o: ../depend_stubs/OgreDataStream.h
collision_mesh.o: ../../dependencies/util/sleep.h
collision_mesh.o: ../src/disk_resource.h ../src/shared_ptr.h
collision_mesh.o: ../depend_stubs/functional
collision_mesh.o: ../src/physics/tcol_parser.h
collision_mesh.o: ../../dependencies/util/math_util.h
collision_mesh.o: ../depend_stubs/cmath ../depend_stubs/cfloat
collision_mesh.o: ../src/physics/tcol_lexer
collision_mesh.o: ../depend_stubs/istream
collision_mesh.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
collision_mesh.o: ../src/physics/tcol_lexer-token_ids
collision_mesh.o: ../depend_stubs/cstdio
collision_mesh.o: ../depend_stubs/quex/code_base/Token
collision_mesh.o: ../depend_stubs/quex/code_base/definitions
collision_mesh.o: ../depend_stubs/quex/code_base/buffer/Buffer
collision_mesh.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
collision_mesh.o: ../depend_stubs/quex/code_base/template/QuexMode
collision_mesh.o: ../depend_stubs/quex/code_base/template/Analyser
collision_mesh.o: ../depend_stubs/quex/code_base/template/Counter
collision_mesh.o: ../depend_stubs/quex/code_base/template/Accumulator
collision_mesh.o: ../depend_stubs/quex/code_base/template/IncludeStack
collision_mesh.o: ../depend_stubs/quex/code_base/temporary_macros_on
collision_mesh.o: ../depend_stubs/quex/code_base/temporary_macros_off
collision_mesh.o: ../depend_stubs/quex/code_base/template/constructor.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/Counter.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/Accumulator.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/mode_handling.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/misc.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/buffer_access.i
collision_mesh.o: ../depend_stubs/quex/code_base/template/Analyser.i
collision_mesh.o: ../src/physics/bcol_parser.h
collision_mesh.o: ../depend_stubs/stdint.h
collision_mesh.o: ../src/physics/loose_end.h
collision_mesh.o: ../src/physics/physical_material.h
grit_col_conv.o: ../depend_stubs/cstdlib
grit_col_conv.o: ../depend_stubs/cerrno
grit_col_conv.o: ../depend_stubs/cstring
grit_col_conv.o: ../depend_stubs/iostream
grit_col_conv.o: ../depend_stubs/fstream
grit_col_conv.o: ../depend_stubs/unistd.h
grit_col_conv.o: ../depend_stubs/sys/types.h
grit_col_conv.o: ../depend_stubs/sys/stat.h
grit_col_conv.o: ../depend_stubs/sys/mman.h
grit_col_conv.o: ../depend_stubs/fcntl.h ../src/portable_io.h
grit_col_conv.o: ../depend_stubs/cstdio
grit_col_conv.o: ../depend_stubs/cassert
grit_col_conv.o: ../depend_stubs/string
grit_col_conv.o: ../depend_stubs/stdint.h
grit_col_conv.o: ../src/centralised_log.h
grit_col_conv.o: ../src/physics/tcol_parser.h
grit_col_conv.o: ../../dependencies/util/math_util.h
grit_col_conv.o: ../depend_stubs/cmath ../depend_stubs/cfloat
grit_col_conv.o: ../src/physics/tcol_lexer
grit_col_conv.o: ../depend_stubs/vector ../depend_stubs/map
grit_col_conv.o: ../depend_stubs/istream
grit_col_conv.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
grit_col_conv.o: ../src/physics/tcol_lexer-token_ids
grit_col_conv.o: ../depend_stubs/quex/code_base/Token
grit_col_conv.o: ../depend_stubs/quex/code_base/definitions
grit_col_conv.o: ../depend_stubs/quex/code_base/buffer/Buffer
grit_col_conv.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
grit_col_conv.o: ../depend_stubs/quex/code_base/template/QuexMode
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Analyser
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Counter
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Accumulator
grit_col_conv.o: ../depend_stubs/quex/code_base/template/IncludeStack
grit_col_conv.o: ../depend_stubs/quex/code_base/temporary_macros_on
grit_col_conv.o: ../depend_stubs/quex/code_base/temporary_macros_off
grit_col_conv.o: ../depend_stubs/quex/code_base/template/constructor.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Counter.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Accumulator.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/mode_handling.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/misc.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/buffer_access.i
grit_col_conv.o: ../depend_stubs/quex/code_base/template/Analyser.i
grit_col_conv.o: ../src/physics/bcol_parser.h
lua_wrappers_physics.o: ../depend_stubs/limits
lua_wrappers_physics.o: ../src/gfx/gfx_ranged_instances.h
lua_wrappers_physics.o: ../src/shared_ptr.h
lua_wrappers_physics.o: ../depend_stubs/cstdlib
lua_wrappers_physics.o: ../depend_stubs/functional
lua_wrappers_physics.o: ../src/streamer.h
lua_wrappers_physics.o: ../src/cache_friendly_range_space_simd.h
lua_wrappers_physics.o: ../depend_stubs/map
lua_wrappers_physics.o: ../depend_stubs/vector
lua_wrappers_physics.o: ../depend_stubs/iostream
lua_wrappers_physics.o: ../depend_stubs/algorithm
lua_wrappers_physics.o: ../depend_stubs/cmath
lua_wrappers_physics.o: ../src/sse_allocator.h
lua_wrappers_physics.o: ../src/centralised_log.h
lua_wrappers_physics.o: ../src/gfx/gfx_instances.h
lua_wrappers_physics.o: ../depend_stubs/OgreMovableObject.h
lua_wrappers_physics.o: ../src/dense_index_map.h
lua_wrappers_physics.o: ../src/gfx/gfx_disk_resource.h
lua_wrappers_physics.o: ../depend_stubs/OgreResource.h
lua_wrappers_physics.o: ../depend_stubs/OgreTexture.h
lua_wrappers_physics.o: ../depend_stubs/OgreMesh.h
lua_wrappers_physics.o: ../../dependencies/util/math_util.h
lua_wrappers_physics.o: ../depend_stubs/string
lua_wrappers_physics.o: ../depend_stubs/cfloat
lua_wrappers_physics.o: ../src/centralised_log.h
lua_wrappers_physics.o: ../depend_stubs/sstream
lua_wrappers_physics.o: ../../dependencies/util/console.h
lua_wrappers_physics.o: ../depend_stubs/ostream
lua_wrappers_physics.o: ../depend_stubs/set
lua_wrappers_physics.o: ../../dependencies/util/exception.h
lua_wrappers_physics.o: ../../dependencies/util/intrinsics.h
lua_wrappers_physics.o: ../src/background_loader.h
lua_wrappers_physics.o: ../src/vect_util.h
lua_wrappers_physics.o: ../depend_stubs/condition_variable
lua_wrappers_physics.o: ../depend_stubs/list
lua_wrappers_physics.o: ../depend_stubs/mutex
lua_wrappers_physics.o: ../depend_stubs/thread
lua_wrappers_physics.o: ../src/disk_resource.h
lua_wrappers_physics.o: ../src/gfx/gfx_node.h
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
lua_wrappers_physics.o: ../depend_stubs/OgreGLGpuProgram.h
lua_wrappers_physics.o: ../depend_stubs/OgreGLSLGpuProgram.h
lua_wrappers_physics.o: ../depend_stubs/OgreGLSLProgram.h
lua_wrappers_physics.o: ../depend_stubs/OgreGLSLLinkProgramManager.h
lua_wrappers_physics.o: ../src/gfx/gfx.h
lua_wrappers_physics.o: ../src/gfx/gfx_option.h
lua_wrappers_physics.o: ../src/gfx/gfx_fertile_node.h
lua_wrappers_physics.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_physics.o: ../src/grit_lua_util.h
lua_wrappers_physics.o: ../depend_stubs/lua.h
lua_wrappers_physics.o: ../depend_stubs/lauxlib.h
lua_wrappers_physics.o: ../depend_stubs/lualib.h
lua_wrappers_physics.o: ../depend_stubs/OgreException.h
lua_wrappers_physics.o: ../../dependencies/util/lua_util.h
lua_wrappers_physics.o: ../../dependencies/util/lua_wrappers_common.h
lua_wrappers_physics.o: ../src/streamer.h ../src/grit_object.h
lua_wrappers_physics.o: ../src/shared_ptr.h ../src/main.h
lua_wrappers_physics.o: ../src/mouse.h ../src/keyboard.h
lua_wrappers_physics.o: ../src/background_loader.h
lua_wrappers_physics.o: ../src/bullet_debug_drawer.h
lua_wrappers_physics.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_physics.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_physics.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_physics.o: ../src/path_util.h
lua_wrappers_physics.o: ../src/lua_wrappers_disk_resource.h
lua_wrappers_physics.o: ../src/gfx/gfx.h
lua_wrappers_physics.o: ../src/gfx/gfx_particle_system.h
lua_wrappers_physics.o: ../depend_stubs/utility
lua_wrappers_physics.o: ../depend_stubs/OgreRoot.h
lua_wrappers_physics.o: ../depend_stubs/OgreRenderWindow.h
lua_wrappers_physics.o: ../src/gfx/lua_wrappers_gfx.h
lua_wrappers_physics.o: ../src/grit_lua_util.h
lua_wrappers_physics.o: ../src/gfx/gfx_body.h
lua_wrappers_physics.o: ../src/gfx/gfx_material.h
lua_wrappers_physics.o: ../depend_stubs/OgreMaterial.h
lua_wrappers_physics.o: ../src/gfx/gfx_shader.h
lua_wrappers_physics.o: ../depend_stubs/cstdint
lua_wrappers_physics.o: ../depend_stubs/array
lua_wrappers_physics.o: ../depend_stubs/unordered_map
lua_wrappers_physics.o: ../depend_stubs/OgreHighLevelGpuProgram.h
lua_wrappers_physics.o: ../src/gfx/gfx_gasoline.h
lua_wrappers_physics.o: ../src/gfx/gfx_pipeline.h
lua_wrappers_physics.o: ../src/gfx/gfx_sky_body.h
lua_wrappers_physics.o: ../src/gfx/gfx_sky_material.h
lua_wrappers_physics.o: ../src/gfx/gfx_light.h
lua_wrappers_physics.o: ../src/gfx/gfx_ranged_instances.h
lua_wrappers_physics.o: ../src/gfx/gfx_hud.h
lua_wrappers_physics.o: ../src/external_table.h
lua_wrappers_physics.o: ../src/lua_ptr.h
lua_wrappers_physics.o: ../../dependencies/util/spline_table.h
lua_wrappers_physics.o: ../src/lua_ptr.h ../src/gfx/gfx_font.h
lua_wrappers_physics.o: ../src/gfx/gfx_text_buffer.h
lua_wrappers_physics.o: ../depend_stubs/OgreHardwareBufferManager.h
lua_wrappers_physics.o: ../depend_stubs/OgreHardwareVertexBuffer.h
lua_wrappers_physics.o: ../depend_stubs/OgreHardwareIndexBuffer.h
lua_wrappers_physics.o: ../src/physics/physics_world.h
lua_wrappers_physics.o: ../depend_stubs/btBulletDynamicsCommon.h
lua_wrappers_physics.o: ../src/physics/tcol_parser.h
lua_wrappers_physics.o: ../src/physics/tcol_lexer
lua_wrappers_physics.o: ../depend_stubs/istream
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
lua_wrappers_physics.o: ../src/physics/tcol_lexer-token_ids
lua_wrappers_physics.o: ../depend_stubs/cstdio
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/Token
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/definitions
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/buffer/Buffer
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/QuexMode
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Analyser
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Counter
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Accumulator
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/IncludeStack
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/temporary_macros_on
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/temporary_macros_off
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/constructor.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Counter.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Accumulator.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/mode_handling.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/misc.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/buffer_access.i
lua_wrappers_physics.o: ../depend_stubs/quex/code_base/template/Analyser.i
lua_wrappers_physics.o: ../src/physics/collision_mesh.h
lua_wrappers_physics.o: ../depend_stubs/OgreDataStream.h
lua_wrappers_physics.o: ../../dependencies/util/sleep.h
lua_wrappers_physics.o: ../src/disk_resource.h
lua_wrappers_physics.o: ../src/physics/bcol_parser.h
lua_wrappers_physics.o: ../depend_stubs/stdint.h
lua_wrappers_physics.o: ../src/physics/loose_end.h
lua_wrappers_physics.o: ../src/physics/physical_material.h
lua_wrappers_physics.o: ../src/grit_object.h
lua_wrappers_physics.o: ../src/external_table.h
lua_wrappers_physics.o: ../src/physics/lua_wrappers_physics.h
physical_material.o: ../src/physics/physical_material.h
physical_material.o: ../depend_stubs/map
physical_material.o: ../depend_stubs/string
physical_material.o: ../depend_stubs/vector
physical_material.o: ../src/centralised_log.h
physical_material.o: ../depend_stubs/iostream
physical_material.o: ../depend_stubs/sstream
physical_material.o: ../../dependencies/util/console.h
physical_material.o: ../depend_stubs/ostream
physical_material.o: ../depend_stubs/set
physical_material.o: ../../dependencies/util/exception.h
physical_material.o: ../depend_stubs/cstdlib
physical_material.o: ../../dependencies/util/intrinsics.h
physical_material.o: ../src/path_util.h
physics_world.o: ../depend_stubs/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h
physics_world.o: ../depend_stubs/BulletCollision/CollisionDispatch/btInternalEdgeUtility.h
physics_world.o: ../src/grit_object.h ../depend_stubs/map
physics_world.o: ../depend_stubs/vector ../depend_stubs/set
physics_world.o: ../depend_stubs/string ../src/shared_ptr.h
physics_world.o: ../src/streamer.h ../depend_stubs/lua.h
physics_world.o: ../depend_stubs/lauxlib.h
physics_world.o: ../depend_stubs/lualib.h ../src/grit_object.h
physics_world.o: ../../dependencies/util/math_util.h
physics_world.o: ../depend_stubs/iostream
physics_world.o: ../depend_stubs/cmath ../depend_stubs/cfloat
physics_world.o: ../src/external_table.h ../src/lua_ptr.h
physics_world.o: ../src/centralised_log.h
physics_world.o: ../../dependencies/util/spline_table.h
physics_world.o: ../depend_stubs/cstdlib
physics_world.o: ../src/background_loader.h ../src/vect_util.h
physics_world.o: ../depend_stubs/algorithm ../src/main.h
physics_world.o: ../src/mouse.h ../src/keyboard.h
physics_world.o: ../src/bullet_debug_drawer.h
physics_world.o: ../depend_stubs/OgreFrameListener.h
physics_world.o: ../depend_stubs/OgreManualObject.h
physics_world.o: ../depend_stubs/btBulletCollisionCommon.h
physics_world.o: ../src/centralised_log.h
physics_world.o: ../depend_stubs/sstream
physics_world.o: ../../dependencies/util/console.h
physics_world.o: ../depend_stubs/ostream
physics_world.o: ../../dependencies/util/exception.h
physics_world.o: ../../dependencies/util/intrinsics.h
physics_world.o: ../src/option.h ../src/grit_lua_util.h
physics_world.o: ../depend_stubs/OgreException.h
physics_world.o: ../../dependencies/util/lua_util.h
physics_world.o: ../depend_stubs/limits
physics_world.o: ../../dependencies/util/lua_wrappers_common.h
physics_world.o: ../src/physics/physics_world.h
physics_world.o: ../src/shared_ptr.h
physics_world.o: ../depend_stubs/functional
physics_world.o: ../depend_stubs/btBulletDynamicsCommon.h
physics_world.o: ../src/physics/tcol_parser.h
physics_world.o: ../src/physics/tcol_lexer
physics_world.o: ../depend_stubs/istream
physics_world.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
physics_world.o: ../src/physics/tcol_lexer-token_ids
physics_world.o: ../depend_stubs/cstdio
physics_world.o: ../depend_stubs/quex/code_base/Token
physics_world.o: ../depend_stubs/quex/code_base/definitions
physics_world.o: ../depend_stubs/quex/code_base/buffer/Buffer
physics_world.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
physics_world.o: ../depend_stubs/quex/code_base/template/QuexMode
physics_world.o: ../depend_stubs/quex/code_base/template/Analyser
physics_world.o: ../depend_stubs/quex/code_base/template/Counter
physics_world.o: ../depend_stubs/quex/code_base/template/Accumulator
physics_world.o: ../depend_stubs/quex/code_base/template/IncludeStack
physics_world.o: ../depend_stubs/quex/code_base/temporary_macros_on
physics_world.o: ../depend_stubs/quex/code_base/temporary_macros_off
physics_world.o: ../depend_stubs/quex/code_base/template/constructor.i
physics_world.o: ../depend_stubs/quex/code_base/template/Counter.i
physics_world.o: ../depend_stubs/quex/code_base/template/Accumulator.i
physics_world.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
physics_world.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
physics_world.o: ../depend_stubs/quex/code_base/template/mode_handling.i
physics_world.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
physics_world.o: ../depend_stubs/quex/code_base/template/misc.i
physics_world.o: ../depend_stubs/quex/code_base/template/buffer_access.i
physics_world.o: ../depend_stubs/quex/code_base/template/Analyser.i
physics_world.o: ../src/physics/collision_mesh.h
physics_world.o: ../depend_stubs/OgreDataStream.h
physics_world.o: ../../dependencies/util/sleep.h
physics_world.o: ../src/disk_resource.h
physics_world.o: ../src/physics/bcol_parser.h
physics_world.o: ../depend_stubs/stdint.h
physics_world.o: ../src/physics/loose_end.h
physics_world.o: ../src/physics/physical_material.h
physics_world.o: ../src/lua_ptr.h
physics_world.o: ../src/physics/lua_wrappers_physics.h
tcol_lexer-core-engine.o: ../src/physics/tcol_lexer
tcol_lexer-core-engine.o: ../depend_stubs/vector
tcol_lexer-core-engine.o: ../depend_stubs/map
tcol_lexer-core-engine.o: ../depend_stubs/istream
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
tcol_lexer-core-engine.o: ../src/physics/tcol_lexer-token_ids
tcol_lexer-core-engine.o: ../depend_stubs/cstdio
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/Token
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/definitions
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/buffer/Buffer
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/QuexMode
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/Analyser
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/Counter
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/Accumulator
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/IncludeStack
tcol_lexer-core-engine.o: ../depend_stubs/cstdlib
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/temporary_macros_on
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/temporary_macros_off
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/constructor.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/Counter.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/Accumulator.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/mode_handling.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/misc.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/buffer_access.i
tcol_lexer-core-engine.o: ../depend_stubs/quex/code_base/template/Analyser.i
tcol_lexer.o: ../src/physics/tcol_lexer ../depend_stubs/vector
tcol_lexer.o: ../depend_stubs/map ../depend_stubs/istream
tcol_lexer.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
tcol_lexer.o: ../src/physics/tcol_lexer-token_ids
tcol_lexer.o: ../depend_stubs/cstdio
tcol_lexer.o: ../depend_stubs/quex/code_base/Token
tcol_lexer.o: ../depend_stubs/quex/code_base/definitions
tcol_lexer.o: ../depend_stubs/quex/code_base/buffer/Buffer
tcol_lexer.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
tcol_lexer.o: ../depend_stubs/quex/code_base/template/QuexMode
tcol_lexer.o: ../depend_stubs/quex/code_base/template/Analyser
tcol_lexer.o: ../depend_stubs/quex/code_base/template/Counter
tcol_lexer.o: ../depend_stubs/quex/code_base/template/Accumulator
tcol_lexer.o: ../depend_stubs/quex/code_base/template/IncludeStack
tcol_lexer.o: ../depend_stubs/cstdlib
tcol_lexer.o: ../depend_stubs/quex/code_base/temporary_macros_on
tcol_lexer.o: ../depend_stubs/quex/code_base/temporary_macros_off
tcol_lexer.o: ../depend_stubs/quex/code_base/template/constructor.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/Counter.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/Accumulator.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/mode_handling.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/misc.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/buffer_access.i
tcol_lexer.o: ../depend_stubs/quex/code_base/template/Analyser.i
tcol_parser.o: ../depend_stubs/iostream
tcol_parser.o: ../depend_stubs/cstdlib
tcol_parser.o: ../../dependencies/util/math_util.h
tcol_parser.o: ../depend_stubs/string ../depend_stubs/cmath
tcol_parser.o: ../depend_stubs/cfloat ../src/path_util.h
tcol_parser.o: ../depend_stubs/vector ../src/centralised_log.h
tcol_parser.o: ../depend_stubs/sstream
tcol_parser.o: ../../dependencies/util/console.h
tcol_parser.o: ../depend_stubs/map ../depend_stubs/ostream
tcol_parser.o: ../depend_stubs/set
tcol_parser.o: ../../dependencies/util/exception.h
tcol_parser.o: ../../dependencies/util/intrinsics.h
tcol_parser.o: ../src/physics/tcol_lexer
tcol_parser.o: ../depend_stubs/istream
tcol_parser.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
tcol_parser.o: ../src/physics/tcol_lexer-token_ids
tcol_parser.o: ../depend_stubs/cstdio
tcol_parser.o: ../depend_stubs/quex/code_base/Token
tcol_parser.o: ../depend_stubs/quex/code_base/definitions
tcol_parser.o: ../depend_stubs/quex/code_base/buffer/Buffer
tcol_parser.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
tcol_parser.o: ../depend_stubs/quex/code_base/template/QuexMode
tcol_parser.o: ../depend_stubs/quex/code_base/template/Analyser
tcol_parser.o: ../depend_stubs/quex/code_base/template/Counter
tcol_parser.o: ../depend_stubs/quex/code_base/template/Accumulator
tcol_parser.o: ../depend_stubs/quex/code_base/template/IncludeStack
tcol_parser.o: ../depend_stubs/quex/code_base/temporary_macros_on
tcol_parser.o: ../depend_stubs/quex/code_base/temporary_macros_off
tcol_parser.o: ../depend_stubs/quex/code_base/template/constructor.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/Counter.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_singleton.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/token_sending_via_singleton.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/misc.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/buffer_access.i
tcol_parser.o: ../depend_stubs/quex/code_base/template/Analyser.i
tcol_parser.o: ../src/physics/tcol_parser.h
tcol_parser.o: ../src/physics/col_defaults.h
audio.o: ../depend_stubs/AL/al.h ../depend_stubs/AL/alc.h
audio.o: ../src/centralised_log.h ../depend_stubs/iostream
audio.o: ../depend_stubs/sstream
audio.o: ../../dependencies/util/console.h ../depend_stubs/map
audio.o: ../depend_stubs/ostream ../depend_stubs/set
audio.o: ../depend_stubs/string ../depend_stubs/vector
audio.o: ../../dependencies/util/exception.h
audio.o: ../depend_stubs/cstdlib
audio.o: ../../dependencies/util/intrinsics.h ../src/vect_util.h
audio.o: ../depend_stubs/algorithm ../src/centralised_log.h
audio.o: ../src/option.h ../src/audio/audio.h
audio.o: ../src/shared_ptr.h ../depend_stubs/functional
audio.o: ../../dependencies/util/math_util.h
audio.o: ../depend_stubs/cmath ../depend_stubs/cfloat
audio.o: ../src/background_loader.h ../src/vect_util.h
audio.o: ../depend_stubs/condition_variable ../depend_stubs/list
audio.o: ../depend_stubs/mutex ../depend_stubs/thread
audio.o: ../src/disk_resource.h ../src/disk_resource.h
audio.o: ../src/audio/audio_disk_resource.h
audio.o: ../depend_stubs/OgreResourceGroupManager.h
audio_disk_resource.o: ../src/audio/audio_disk_resource.h
audio_disk_resource.o: ../depend_stubs/AL/al.h
audio_disk_resource.o: ../depend_stubs/OgreResourceGroupManager.h
audio_disk_resource.o: ../src/centralised_log.h
audio_disk_resource.o: ../depend_stubs/iostream
audio_disk_resource.o: ../depend_stubs/sstream
audio_disk_resource.o: ../../dependencies/util/console.h
audio_disk_resource.o: ../depend_stubs/map
audio_disk_resource.o: ../depend_stubs/ostream
audio_disk_resource.o: ../depend_stubs/set
audio_disk_resource.o: ../depend_stubs/string
audio_disk_resource.o: ../depend_stubs/vector
audio_disk_resource.o: ../../dependencies/util/exception.h
audio_disk_resource.o: ../depend_stubs/cstdlib
audio_disk_resource.o: ../../dependencies/util/intrinsics.h
audio_disk_resource.o: ../src/background_loader.h
audio_disk_resource.o: ../src/vect_util.h
audio_disk_resource.o: ../depend_stubs/algorithm
audio_disk_resource.o: ../src/centralised_log.h
audio_disk_resource.o: ../depend_stubs/condition_variable
audio_disk_resource.o: ../depend_stubs/list
audio_disk_resource.o: ../depend_stubs/mutex
audio_disk_resource.o: ../depend_stubs/thread
audio_disk_resource.o: ../src/disk_resource.h
audio_disk_resource.o: ../src/audio/audio.h ../src/shared_ptr.h
audio_disk_resource.o: ../depend_stubs/functional
audio_disk_resource.o: ../../dependencies/util/math_util.h
audio_disk_resource.o: ../depend_stubs/cmath
audio_disk_resource.o: ../depend_stubs/cfloat
audio_disk_resource.o: ../src/disk_resource.h
audio_disk_resource.o: ../src/portable_io.h
audio_disk_resource.o: ../depend_stubs/cstdio
audio_disk_resource.o: ../depend_stubs/cassert
audio_disk_resource.o: ../depend_stubs/cstring
audio_disk_resource.o: ../depend_stubs/fstream
audio_disk_resource.o: ../depend_stubs/stdint.h
audio_disk_resource.o: ../src/audio/ogg_vorbis_decoder.h
audio_disk_resource.o: ../depend_stubs/vorbis/codec.h
audio_disk_resource.o: ../depend_stubs/vorbis/vorbisfile.h
lua_wrappers_audio.o: ../src/main.h ../depend_stubs/lua.h
lua_wrappers_audio.o: ../src/mouse.h ../depend_stubs/vector
lua_wrappers_audio.o: ../src/keyboard.h ../depend_stubs/string
lua_wrappers_audio.o: ../src/background_loader.h
lua_wrappers_audio.o: ../src/vect_util.h
lua_wrappers_audio.o: ../depend_stubs/algorithm
lua_wrappers_audio.o: ../src/centralised_log.h
lua_wrappers_audio.o: ../src/bullet_debug_drawer.h
lua_wrappers_audio.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_audio.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_audio.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_audio.o: ../src/external_table.h
lua_wrappers_audio.o: ../depend_stubs/map
lua_wrappers_audio.o: ../depend_stubs/lauxlib.h
lua_wrappers_audio.o: ../../dependencies/util/math_util.h
lua_wrappers_audio.o: ../depend_stubs/iostream
lua_wrappers_audio.o: ../depend_stubs/cmath
lua_wrappers_audio.o: ../depend_stubs/cfloat ../src/shared_ptr.h
lua_wrappers_audio.o: ../src/lua_ptr.h ../depend_stubs/lualib.h
lua_wrappers_audio.o: ../../dependencies/util/spline_table.h
lua_wrappers_audio.o: ../depend_stubs/cstdlib ../src/lua_ptr.h
lua_wrappers_audio.o: ../src/path_util.h ../src/audio/audio.h
lua_wrappers_audio.o: ../src/shared_ptr.h
lua_wrappers_audio.o: ../depend_stubs/functional
lua_wrappers_audio.o: ../depend_stubs/AL/al.h
lua_wrappers_audio.o: ../src/background_loader.h
lua_wrappers_audio.o: ../depend_stubs/condition_variable
lua_wrappers_audio.o: ../depend_stubs/list ../depend_stubs/mutex
lua_wrappers_audio.o: ../depend_stubs/thread
lua_wrappers_audio.o: ../src/disk_resource.h ../depend_stubs/set
lua_wrappers_audio.o: ../src/disk_resource.h
lua_wrappers_audio.o: ../src/audio/audio_disk_resource.h
lua_wrappers_audio.o: ../depend_stubs/OgreResourceGroupManager.h
lua_wrappers_audio.o: ../src/centralised_log.h
lua_wrappers_audio.o: ../depend_stubs/sstream
lua_wrappers_audio.o: ../../dependencies/util/console.h
lua_wrappers_audio.o: ../depend_stubs/ostream
lua_wrappers_audio.o: ../../dependencies/util/exception.h
lua_wrappers_audio.o: ../../dependencies/util/intrinsics.h
lua_wrappers_audio.o: ../src/audio/lua_wrappers_audio.h
lua_wrappers_audio.o: ../src/grit_lua_util.h
lua_wrappers_audio.o: ../depend_stubs/OgreException.h
lua_wrappers_audio.o: ../../dependencies/util/lua_util.h
lua_wrappers_audio.o: ../depend_stubs/limits
lua_wrappers_audio.o: ../../dependencies/util/lua_wrappers_common.h
ogg_vorbis_decoder.o: ../src/audio/ogg_vorbis_decoder.h
ogg_vorbis_decoder.o: ../depend_stubs/string
ogg_vorbis_decoder.o: ../depend_stubs/vorbis/codec.h
ogg_vorbis_decoder.o: ../depend_stubs/vorbis/vorbisfile.h
ogg_vorbis_decoder.o: ../depend_stubs/OgreResourceGroupManager.h
ogg_vorbis_decoder.o: ../src/centralised_log.h
ogg_vorbis_decoder.o: ../depend_stubs/iostream
ogg_vorbis_decoder.o: ../depend_stubs/sstream
ogg_vorbis_decoder.o: ../../dependencies/util/console.h
ogg_vorbis_decoder.o: ../depend_stubs/map
ogg_vorbis_decoder.o: ../depend_stubs/ostream
ogg_vorbis_decoder.o: ../depend_stubs/set ../depend_stubs/vector
ogg_vorbis_decoder.o: ../../dependencies/util/exception.h
ogg_vorbis_decoder.o: ../depend_stubs/cstdlib
ogg_vorbis_decoder.o: ../../dependencies/util/intrinsics.h
lua_wrappers_net.o: ../src/main.h ../depend_stubs/lua.h
lua_wrappers_net.o: ../src/mouse.h ../depend_stubs/vector
lua_wrappers_net.o: ../src/keyboard.h ../depend_stubs/string
lua_wrappers_net.o: ../src/background_loader.h ../src/vect_util.h
lua_wrappers_net.o: ../depend_stubs/algorithm
lua_wrappers_net.o: ../src/centralised_log.h
lua_wrappers_net.o: ../src/bullet_debug_drawer.h
lua_wrappers_net.o: ../depend_stubs/OgreFrameListener.h
lua_wrappers_net.o: ../depend_stubs/OgreManualObject.h
lua_wrappers_net.o: ../depend_stubs/btBulletCollisionCommon.h
lua_wrappers_net.o: ../src/external_table.h ../depend_stubs/map
lua_wrappers_net.o: ../depend_stubs/lauxlib.h
lua_wrappers_net.o: ../../dependencies/util/math_util.h
lua_wrappers_net.o: ../depend_stubs/iostream ../depend_stubs/cmath
lua_wrappers_net.o: ../depend_stubs/cfloat ../src/shared_ptr.h
lua_wrappers_net.o: ../src/lua_ptr.h ../depend_stubs/lualib.h
lua_wrappers_net.o: ../../dependencies/util/spline_table.h
lua_wrappers_net.o: ../depend_stubs/cstdlib ../src/lua_ptr.h
lua_wrappers_net.o: ../src/path_util.h ../src/net/net.h
lua_wrappers_net.o: ../src/shared_ptr.h ../depend_stubs/functional
lua_wrappers_net.o: ../depend_stubs/stdint.h
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
lua_wrappers_net.o: ../depend_stubs/sstream ../depend_stubs/limits
lua_wrappers_net.o: ../../dependencies/util/exception.h
lua_wrappers_net.o: ../../dependencies/util/intrinsics.h
lua_wrappers_net.o: ../../dependencies/util/lua_wrappers_common.h
net.o: ../../dependencies/util/sleep.h ../src/centralised_log.h
net.o: ../depend_stubs/iostream ../depend_stubs/sstream
net.o: ../../dependencies/util/console.h ../depend_stubs/map
net.o: ../depend_stubs/ostream ../depend_stubs/set
net.o: ../depend_stubs/string ../depend_stubs/vector
net.o: ../../dependencies/util/exception.h ../depend_stubs/cstdlib
net.o: ../../dependencies/util/intrinsics.h ../src/net/net.h
net.o: ../src/shared_ptr.h ../depend_stubs/functional
net.o: ../depend_stubs/stdint.h ../depend_stubs/sys/socket.h
net.o: ../depend_stubs/arpa/inet.h ../depend_stubs/sys/param.h
net.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
net.o: ../depend_stubs/sys/ioctl.h ../src/net/net_manager.h
net.o: ../depend_stubs/queue ../src/external_table.h
net.o: ../depend_stubs/lua.h ../depend_stubs/lauxlib.h
net.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
net.o: ../depend_stubs/cfloat ../src/shared_ptr.h ../src/lua_ptr.h
net.o: ../depend_stubs/lualib.h ../src/centralised_log.h
net.o: ../../dependencies/util/spline_table.h
net_address.o: ../depend_stubs/cstdio ../depend_stubs/cstring
net_address.o: ../src/centralised_log.h ../depend_stubs/iostream
net_address.o: ../depend_stubs/sstream
net_address.o: ../../dependencies/util/console.h
net_address.o: ../depend_stubs/map ../depend_stubs/ostream
net_address.o: ../depend_stubs/set ../depend_stubs/string
net_address.o: ../depend_stubs/vector
net_address.o: ../../dependencies/util/exception.h
net_address.o: ../depend_stubs/cstdlib
net_address.o: ../../dependencies/util/intrinsics.h
net_address.o: ../src/net/net.h ../src/shared_ptr.h
net_address.o: ../depend_stubs/functional ../depend_stubs/stdint.h
net_address.o: ../depend_stubs/sys/socket.h
net_address.o: ../depend_stubs/arpa/inet.h
net_address.o: ../depend_stubs/sys/param.h
net_address.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
net_address.o: ../depend_stubs/sys/ioctl.h
net_manager.o: ../depend_stubs/cstdlib
net_manager.o: ../../dependencies/util/sleep.h
net_manager.o: ../src/centralised_log.h ../depend_stubs/iostream
net_manager.o: ../depend_stubs/sstream
net_manager.o: ../../dependencies/util/console.h
net_manager.o: ../depend_stubs/map ../depend_stubs/ostream
net_manager.o: ../depend_stubs/set ../depend_stubs/string
net_manager.o: ../depend_stubs/vector
net_manager.o: ../../dependencies/util/exception.h
net_manager.o: ../../dependencies/util/intrinsics.h
net_manager.o: ../src/grit_lua_util.h ../depend_stubs/lua.h
net_manager.o: ../depend_stubs/lauxlib.h ../depend_stubs/lualib.h
net_manager.o: ../depend_stubs/OgreException.h
net_manager.o: ../src/centralised_log.h
net_manager.o: ../../dependencies/util/lua_util.h
net_manager.o: ../depend_stubs/limits
net_manager.o: ../../dependencies/util/math_util.h
net_manager.o: ../depend_stubs/cmath ../depend_stubs/cfloat
net_manager.o: ../../dependencies/util/lua_wrappers_common.h
net_manager.o: ../src/net/net.h ../src/shared_ptr.h
net_manager.o: ../depend_stubs/functional ../depend_stubs/stdint.h
net_manager.o: ../depend_stubs/sys/socket.h
net_manager.o: ../depend_stubs/arpa/inet.h
net_manager.o: ../depend_stubs/sys/param.h
net_manager.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
net_manager.o: ../depend_stubs/sys/ioctl.h
net_manager.o: ../src/net/net_manager.h ../depend_stubs/queue
net_manager.o: ../src/external_table.h ../src/shared_ptr.h
net_manager.o: ../src/lua_ptr.h
net_manager.o: ../../dependencies/util/spline_table.h
net_manager.o: ../src/net/lua_wrappers_net.h
net_message.o: ../depend_stubs/cstdlib ../depend_stubs/cstdio
net_message.o: ../depend_stubs/cstring ../src/centralised_log.h
net_message.o: ../depend_stubs/iostream ../depend_stubs/sstream
net_message.o: ../../dependencies/util/console.h
net_message.o: ../depend_stubs/map ../depend_stubs/ostream
net_message.o: ../depend_stubs/set ../depend_stubs/string
net_message.o: ../depend_stubs/vector
net_message.o: ../../dependencies/util/exception.h
net_message.o: ../../dependencies/util/intrinsics.h
net_message.o: ../src/net/net.h ../src/shared_ptr.h
net_message.o: ../depend_stubs/functional ../depend_stubs/stdint.h
net_message.o: ../depend_stubs/sys/socket.h
net_message.o: ../depend_stubs/arpa/inet.h
net_message.o: ../depend_stubs/sys/param.h
net_message.o: ../depend_stubs/sys/types.h ../depend_stubs/netdb.h
net_message.o: ../depend_stubs/sys/ioctl.h
