COMPILING=echo "Compiling: [32m$<[0m"
LINKING=echo "Linking: [1;32m$@[0m"

%.o: ../src/win32/%.c++
	@$(COMPILING)
	@$(COMPILER) -c $< -o $@ $(CFLAGS)

%.o: ../src/linux/%.c++
	@$(COMPILING)
	@$(COMPILER) -c $< -o $@ $(CFLAGS)

%.o: ../src/%.c++
	@$(COMPILING)
	@$(COMPILER) -c $< -o $@ $(CFLAGS)

LEXER=TColLexer
../src/$(LEXER): ../src/$(LEXER).qx
	cd ../src && $(QUEX_PATH)/quex-exe.py -i $< --engine $(LEXER)

%.o: ../src/%.cpp
	@$(COMPILING)
	@$(WEAKCOMPILER) -c $< -o $@ $(CFLAGS)



COMMON_OBJ=\
        main.o \
        Grit.o \
        matbin.o \
        ray.o \
        lua_util.o \
        lua_wrappers_core.o \
        lua_wrappers_mesh.o \
        lua_wrappers_gpuprog.o \
        lua_wrappers_render.o \
        lua_wrappers_gritobj.o \
        lua_wrappers_mobj.o \
        lua_wrappers_scnmgr.o \
        lua_wrappers_hud.o \
        lua_wrappers_physics.o \
        lua_wrappers_tex.o \
        lua_wrappers_material.o \
        lua_wrappers_primitives.o \
        HUD.o \
        TextListOverlayElement.o \
        BackgroundMeshLoader.o \
        CollisionMesh.o \
        PhysicsWorld.o \
        GritObject.o \
        GritObjectManager.o \
        $(LEXER).o \
        $(LEXER)-core-engine.o \
        TColParser.o

grit.x11: $(COMMON_OBJ) MouseX11.o KeyboardX11.o posix_sleep.o
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(LDFLAGS)

grit.exe: $(COMMON_OBJ) MouseDirectInput8.o KeyboardDirectInput8.o win32_sleep.o
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(LDFLAGS)

matbin: ../src/matbin.c++
	@$(LINKING)
	@$(COMPILER) -D_MATBIN_TEST $^ -o $@ $(CLDFLAGS)

clean:
	rm -fv $(TARGETS) *.o




# DO NOT DELETE

BackgroundMeshLoader.o: ../src/sleep.h ../src/BackgroundMeshLoader.h
BackgroundMeshLoader.o: ../src/app_error.h
CollisionMesh.o: ../src/CollisionMesh.h ../src/LooseEnd.h
CollisionMesh.o: ../src/TColParser.h ../src/TColLexer
CollisionMesh.o: ../src/TColLexer-token_ids
Grit.o: ../src/Grit.h ../src/lua_userdata_dependency_tracker.h
Grit.o: ../src/sleep.h ../src/ray.h ../src/app_error.h
Grit.o: ../src/lua_util.h ../src/Mouse.h ../src/Keyboard.h
Grit.o: ../src/lua_wrappers_core.h ../src/GritObject.h
Grit.o: ../src/PhysicsWorld.h ../src/CollisionMesh.h
Grit.o: ../src/LooseEnd.h ../src/GritObjectManager.h
Grit.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../src/SSEAllocator.h
Grit.o: ../src/BackgroundMeshLoader.h
Grit.o: ../src/lua_wrappers_primitives.h ../src/lua_wrappers_common.h
Grit.o: ../src/HUD.h ../src/TextListOverlayElement.h
GritObject.o: ../src/PhysicsWorld.h ../src/CollisionMesh.h
GritObject.o: ../src/LooseEnd.h ../src/GritObject.h
GritObject.o: ../src/GritObjectManager.h
GritObject.o: ../src/CacheFriendlyRangeSpaceSIMD.h
GritObject.o: ../src/SSEAllocator.h ../src/app_error.h
GritObject.o: ../src/BackgroundMeshLoader.h
GritObject.o: ../src/lua_wrappers_primitives.h
GritObject.o: ../src/lua_wrappers_common.h ../src/lua_util.h
GritObject.o: ../src/lua_wrappers_gritobj.h
GritObject.o: ../src/lua_wrappers_physics.h
GritObject.o: ../src/lua_wrappers_scnmgr.h ../src/Grit.h
GritObject.o: ../src/lua_userdata_dependency_tracker.h
GritObjectManager.o: ../src/GritObjectManager.h ../src/GritObject.h
GritObjectManager.o: ../src/PhysicsWorld.h ../src/CollisionMesh.h
GritObjectManager.o: ../src/LooseEnd.h ../src/BackgroundMeshLoader.h
GritObjectManager.o: ../src/app_error.h
GritObjectManager.o: ../src/lua_wrappers_primitives.h
GritObjectManager.o: ../src/lua_wrappers_common.h ../src/lua_util.h
GritObjectManager.o: ../src/CacheFriendlyRangeSpaceSIMD.h
GritObjectManager.o: ../src/SSEAllocator.h ../src/Grit.h
GritObjectManager.o: ../src/lua_userdata_dependency_tracker.h
HUD.o: ../src/HUD.h ../src/TextListOverlayElement.h
HUD.o: ../src/app_error.h
lua_util.o: ../src/lua_util.h ../src/app_error.h
lua_wrappers_core.o: ../src/Grit.h
lua_wrappers_core.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_core.o: ../src/Keyboard.h ../src/Mouse.h
lua_wrappers_core.o: ../src/BackgroundMeshLoader.h ../src/app_error.h
lua_wrappers_core.o: ../src/matbin.h ../src/sleep.h ../src/HUD.h
lua_wrappers_core.o: ../src/TextListOverlayElement.h
lua_wrappers_core.o: ../src/lua_wrappers_primitives.h
lua_wrappers_core.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_core.o: ../src/lua_wrappers_physics.h
lua_wrappers_core.o: ../src/PhysicsWorld.h ../src/CollisionMesh.h
lua_wrappers_core.o: ../src/LooseEnd.h ../src/GritObject.h
lua_wrappers_core.o: ../src/GritObjectManager.h
lua_wrappers_core.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_core.o: ../src/SSEAllocator.h ../src/lua_wrappers_mobj.h
lua_wrappers_core.o: ../src/lua_wrappers_scnmgr.h
lua_wrappers_core.o: ../src/lua_wrappers_material.h
lua_wrappers_core.o: ../src/lua_wrappers_mesh.h
lua_wrappers_core.o: ../src/lua_wrappers_gpuprog.h
lua_wrappers_core.o: ../src/lua_wrappers_tex.h
lua_wrappers_core.o: ../src/lua_wrappers_render.h
lua_wrappers_core.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_core.o: ../src/lua_wrappers_hud.h
lua_wrappers_gpuprog.o: ../src/lua_wrappers_gpuprog.h
lua_wrappers_gpuprog.o: ../src/lua_wrappers_common.h
lua_wrappers_gpuprog.o: ../src/lua_util.h ../src/app_error.h
lua_wrappers_gritobj.o: ../src/GritObjectManager.h
lua_wrappers_gritobj.o: ../src/GritObject.h ../src/PhysicsWorld.h
lua_wrappers_gritobj.o: ../src/CollisionMesh.h ../src/LooseEnd.h
lua_wrappers_gritobj.o: ../src/BackgroundMeshLoader.h
lua_wrappers_gritobj.o: ../src/app_error.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_primitives.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_common.h
lua_wrappers_gritobj.o: ../src/lua_util.h
lua_wrappers_gritobj.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_gritobj.o: ../src/SSEAllocator.h ../src/Grit.h
lua_wrappers_gritobj.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_physics.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_scnmgr.h
lua_wrappers_hud.o: ../src/HUD.h ../src/TextListOverlayElement.h
lua_wrappers_hud.o: ../src/lua_wrappers_hud.h
lua_wrappers_hud.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_hud.o: ../src/app_error.h ../src/lua_wrappers_material.h
lua_wrappers_material.o: ../src/lua_wrappers_material.h
lua_wrappers_material.o: ../src/lua_wrappers_common.h
lua_wrappers_material.o: ../src/lua_util.h ../src/app_error.h
lua_wrappers_mesh.o: ../src/lua_wrappers_mesh.h
lua_wrappers_mesh.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_mesh.o: ../src/app_error.h
lua_wrappers_mesh.o: ../src/lua_wrappers_material.h
lua_wrappers_mobj.o: ../src/Grit.h
lua_wrappers_mobj.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_mobj.o: ../src/lua_wrappers_mobj.h
lua_wrappers_mobj.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_mobj.o: ../src/app_error.h ../src/lua_wrappers_scnmgr.h
lua_wrappers_mobj.o: ../src/lua_wrappers_primitives.h
lua_wrappers_mobj.o: ../src/lua_wrappers_material.h
lua_wrappers_mobj.o: ../src/lua_wrappers_mesh.h
lua_wrappers_mobj.o: ../src/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_physics.o: ../src/PhysicsWorld.h ../src/CollisionMesh.h
lua_wrappers_physics.o: ../src/LooseEnd.h ../src/GritObject.h
lua_wrappers_physics.o: ../src/GritObjectManager.h
lua_wrappers_physics.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_physics.o: ../src/SSEAllocator.h ../src/app_error.h
lua_wrappers_physics.o: ../src/BackgroundMeshLoader.h
lua_wrappers_physics.o: ../src/lua_wrappers_primitives.h
lua_wrappers_physics.o: ../src/lua_wrappers_common.h
lua_wrappers_physics.o: ../src/lua_util.h
lua_wrappers_physics.o: ../src/lua_wrappers_physics.h
lua_wrappers_primitives.o: ../src/lua_wrappers_primitives.h
lua_wrappers_primitives.o: ../src/lua_wrappers_common.h
lua_wrappers_primitives.o: ../src/lua_util.h ../src/app_error.h
lua_wrappers_render.o: ../src/Grit.h
lua_wrappers_render.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_render.o: ../src/lua_wrappers_render.h
lua_wrappers_render.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_render.o: ../src/app_error.h ../src/lua_wrappers_tex.h
lua_wrappers_render.o: ../src/lua_wrappers_mobj.h
lua_wrappers_render.o: ../src/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_scnmgr.o: ../src/Grit.h
lua_wrappers_scnmgr.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_scnmgr.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_scnmgr.o: ../src/app_error.h ../src/lua_wrappers_mobj.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_primitives.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_material.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_tex.h
lua_wrappers_scnmgr.o: ../src/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_tex.o: ../src/lua_wrappers_tex.h
lua_wrappers_tex.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_tex.o: ../src/app_error.h ../src/lua_wrappers_render.h
main.o: ../src/linux/KeyboardX11.h ../src/Keyboard.h
main.o: ../src/linux/MouseX11.h ../src/Mouse.h ../src/Grit.h
main.o: ../src/lua_userdata_dependency_tracker.h
main.o: ../src/BackgroundMeshLoader.h ../src/app_error.h
main.o: ../src/console_colour.h
matbin.o: ../src/matbin.h ../src/ogre_datastream_util.h
PhysicsWorld.o: ../src/PhysicsWorld.h ../src/CollisionMesh.h
PhysicsWorld.o: ../src/LooseEnd.h ../src/GritObject.h
PhysicsWorld.o: ../src/GritObjectManager.h
PhysicsWorld.o: ../src/CacheFriendlyRangeSpaceSIMD.h
PhysicsWorld.o: ../src/SSEAllocator.h ../src/app_error.h
PhysicsWorld.o: ../src/BackgroundMeshLoader.h
PhysicsWorld.o: ../src/lua_wrappers_primitives.h
PhysicsWorld.o: ../src/lua_wrappers_common.h ../src/lua_util.h
PhysicsWorld.o: ../src/Grit.h
PhysicsWorld.o: ../src/lua_userdata_dependency_tracker.h
PhysicsWorld.o: ../src/lua_wrappers_physics.h
ray.o: ../src/ray.h
TColParser.o: ../src/TColLexer ../src/TColLexer-token_ids
TColParser.o: ../src/TColParser.h
TextListOverlayElement.o: ../src/TextListOverlayElement.h
TextListOverlayElement.o: ../src/app_error.h ../src/Grit.h
TextListOverlayElement.o: ../src/lua_userdata_dependency_tracker.h
KeyboardX11.o: ../src/linux/KeyboardX11.h ../src/Keyboard.h
KeyboardX11.o: ../src/app_error.h
MouseX11.o: ../src/linux/MouseX11.h ../src/Mouse.h
MouseX11.o: ../src/app_error.h
KeyboardDirectInput8.o: ../src/win32/KeyboardDirectInput8.h
KeyboardDirectInput8.o: ../src/Keyboard.h ../src/app_error.h
MouseDirectInput8.o: ../src/win32/MouseDirectInput8.h
MouseDirectInput8.o: ../src/Mouse.h ../src/app_error.h
