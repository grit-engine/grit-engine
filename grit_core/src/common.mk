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

../src/TColLexer: ../src/TColLexer.qx
	cd ../src && $(QUEX_PATH)/quex-exe.py -i $< --engine TColLexer

%.o: ../src/%.cpp
	@$(COMPILING)
	@$(WEAKCOMPILER) -c $< -o $@ $(CFLAGS)



COMMON_OBJ=\
        BackgroundMeshLoader.o \
        BulletDebugDrawer.o \
        CollisionMesh.o \
        ExternalTable.o \
        Grit.o \
        GritClass.o \
        GritObject.o \
        Streamer.o \
        HUD.o \
        ldbglue.o \
        lua_util.o \
        lua_wrappers_core.o \
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
        LuaParticleSystem.o \
        main.o \
        path_util.o \
        PhysicsWorld.o \
        ray.o \
        TColLexer.o \
        TColLexer-core-engine.o \
        TColParser.o \
        TextListOverlayElement.o \
        unicode_util.o \

LINUX_OBJ=$(COMMON_OBJ) MouseX11.o KeyboardX11.o posix_sleep.o x11_clipboard.o
WIN32_OBJ=$(COMMON_OBJ) MouseDirectInput8.o KeyboardDirectInput8.o KeyboardWinAPI.o win32_clipboard.o win32_sleep.o

grit.x11: $(LINUX_OBJ) $(DEPENDENT_LIBS)
	@$(LINKING)
	@$(COMPILER) $(LINUX_OBJ) -o $@ $(LDFLAGS)

grit.exe: $(WIN32_OBJ) $(DEPENDENT_LIBS)
	@$(LINKING)
	@$(COMPILER) $(WIN32_OBJ) -o $@ $(LDFLAGS)

clean:
	rm -fv $(TARGETS) *.o




# DO NOT DELETE

BackgroundMeshLoader.o: ../src/sleep.h ../src/BackgroundMeshLoader.h
BackgroundMeshLoader.o: ../src/CentralisedLog.h
BackgroundMeshLoader.o: ../src/console_colour.h
BulletDebugDrawer.o: ../src/BulletDebugDrawer.h
BulletDebugDrawer.o: ../src/CentralisedLog.h ../src/console_colour.h
BulletDebugDrawer.o: ../src/PhysicsWorld.h ../src/TColParser.h
BulletDebugDrawer.o: ../src/TColLexer ../src/TColLexer-token_ids
BulletDebugDrawer.o: ../src/math_util.h ../src/CollisionMesh.h
BulletDebugDrawer.o: ../src/LooseEnd.h ../src/GritObject.h
BulletDebugDrawer.o: ../src/Streamer.h ../src/GritClass.h
BulletDebugDrawer.o: ../src/ExternalTable.h ../src/lua_util.h
BulletDebugDrawer.o: ../src/CacheFriendlyRangeSpaceSIMD.h
BulletDebugDrawer.o: ../src/SSEAllocator.h
BulletDebugDrawer.o: ../src/BackgroundMeshLoader.h
CollisionMesh.o: ../src/CollisionMesh.h ../src/TColParser.h
CollisionMesh.o: ../src/TColLexer ../src/TColLexer-token_ids
CollisionMesh.o: ../src/math_util.h ../src/CentralisedLog.h
CollisionMesh.o: ../src/console_colour.h ../src/LooseEnd.h
CollisionMesh.o: ../src/PhysicsWorld.h ../src/GritObject.h
CollisionMesh.o: ../src/Streamer.h ../src/GritClass.h
CollisionMesh.o: ../src/ExternalTable.h ../src/lua_util.h
CollisionMesh.o: ../src/CacheFriendlyRangeSpaceSIMD.h
CollisionMesh.o: ../src/SSEAllocator.h ../src/BackgroundMeshLoader.h
CollisionMesh.o: ../src/path_util.h
ExternalTable.o: ../src/ExternalTable.h ../src/math_util.h
ExternalTable.o: ../src/CentralisedLog.h ../src/console_colour.h
ExternalTable.o: ../src/lua_util.h ../src/lua_wrappers_primitives.h
ExternalTable.o: ../src/lua_wrappers_common.h
Grit.o: ../src/Grit.h ../src/BulletDebugDrawer.h
Grit.o: ../src/lua_userdata_dependency_tracker.h ../src/sleep.h
Grit.o: ../src/ray.h ../src/CentralisedLog.h ../src/console_colour.h
Grit.o: ../src/lua_util.h ../src/math_util.h ../src/Mouse.h
Grit.o: ../src/Keyboard.h ../src/lua_wrappers_core.h
Grit.o: ../src/GritObject.h ../src/PhysicsWorld.h ../src/TColParser.h
Grit.o: ../src/TColLexer ../src/TColLexer-token_ids
Grit.o: ../src/CollisionMesh.h ../src/LooseEnd.h ../src/Streamer.h
Grit.o: ../src/GritClass.h ../src/ExternalTable.h
Grit.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../src/SSEAllocator.h
Grit.o: ../src/BackgroundMeshLoader.h ../src/path_util.h ../src/HUD.h
Grit.o: ../src/TextListOverlayElement.h
GritClass.o: ../src/GritClass.h ../src/ExternalTable.h
GritClass.o: ../src/math_util.h ../src/CentralisedLog.h
GritClass.o: ../src/console_colour.h ../src/lua_util.h
GritObject.o: ../src/PhysicsWorld.h ../src/CentralisedLog.h
GritObject.o: ../src/console_colour.h ../src/TColParser.h
GritObject.o: ../src/TColLexer ../src/TColLexer-token_ids
GritObject.o: ../src/math_util.h ../src/CollisionMesh.h
GritObject.o: ../src/LooseEnd.h ../src/GritObject.h ../src/Streamer.h
GritObject.o: ../src/GritClass.h ../src/ExternalTable.h
GritObject.o: ../src/lua_util.h ../src/CacheFriendlyRangeSpaceSIMD.h
GritObject.o: ../src/SSEAllocator.h ../src/BackgroundMeshLoader.h
GritObject.o: ../src/lua_wrappers_gritobj.h
GritObject.o: ../src/lua_wrappers_common.h
GritObject.o: ../src/lua_wrappers_physics.h
GritObject.o: ../src/lua_wrappers_scnmgr.h ../src/Grit.h
GritObject.o: ../src/BulletDebugDrawer.h
GritObject.o: ../src/lua_userdata_dependency_tracker.h
HUD.o: ../src/HUD.h ../src/TextListOverlayElement.h
HUD.o: ../src/CentralisedLog.h ../src/console_colour.h
LuaParticleSystem.o: ../src/Grit.h ../src/BulletDebugDrawer.h
LuaParticleSystem.o: ../src/lua_userdata_dependency_tracker.h
LuaParticleSystem.o: ../src/CentralisedLog.h ../src/console_colour.h
LuaParticleSystem.o: ../src/LuaParticleSystem.h ../src/lua_util.h
LuaParticleSystem.o: ../src/math_util.h
PhysicsWorld.o: ../src/PhysicsWorld.h ../src/CentralisedLog.h
PhysicsWorld.o: ../src/console_colour.h ../src/TColParser.h
PhysicsWorld.o: ../src/TColLexer ../src/TColLexer-token_ids
PhysicsWorld.o: ../src/math_util.h ../src/CollisionMesh.h
PhysicsWorld.o: ../src/LooseEnd.h ../src/GritObject.h
PhysicsWorld.o: ../src/Streamer.h ../src/GritClass.h
PhysicsWorld.o: ../src/ExternalTable.h ../src/lua_util.h
PhysicsWorld.o: ../src/CacheFriendlyRangeSpaceSIMD.h
PhysicsWorld.o: ../src/SSEAllocator.h ../src/BackgroundMeshLoader.h
PhysicsWorld.o: ../src/Grit.h ../src/BulletDebugDrawer.h
PhysicsWorld.o: ../src/lua_userdata_dependency_tracker.h
PhysicsWorld.o: ../src/lua_wrappers_physics.h
PhysicsWorld.o: ../src/lua_wrappers_common.h
Streamer.o: ../src/Streamer.h ../src/GritClass.h
Streamer.o: ../src/ExternalTable.h ../src/math_util.h
Streamer.o: ../src/CentralisedLog.h ../src/console_colour.h
Streamer.o: ../src/lua_util.h ../src/GritObject.h
Streamer.o: ../src/PhysicsWorld.h ../src/TColParser.h ../src/TColLexer
Streamer.o: ../src/TColLexer-token_ids ../src/CollisionMesh.h
Streamer.o: ../src/LooseEnd.h ../src/BackgroundMeshLoader.h
Streamer.o: ../src/CacheFriendlyRangeSpaceSIMD.h ../src/SSEAllocator.h
Streamer.o: ../src/Grit.h ../src/BulletDebugDrawer.h
Streamer.o: ../src/lua_userdata_dependency_tracker.h
TColParser.o: ../src/TColLexer ../src/TColLexer-token_ids
TColParser.o: ../src/TColParser.h ../src/math_util.h
TColParser.o: ../src/CentralisedLog.h ../src/console_colour.h
TColParser.o: ../src/PhysicsWorld.h ../src/CollisionMesh.h
TColParser.o: ../src/LooseEnd.h ../src/GritObject.h ../src/Streamer.h
TColParser.o: ../src/GritClass.h ../src/ExternalTable.h
TColParser.o: ../src/lua_util.h ../src/CacheFriendlyRangeSpaceSIMD.h
TColParser.o: ../src/SSEAllocator.h ../src/BackgroundMeshLoader.h
TColParser.o: ../src/path_util.h
TextListOverlayElement.o: ../src/TextListOverlayElement.h
TextListOverlayElement.o: ../src/CentralisedLog.h
TextListOverlayElement.o: ../src/console_colour.h ../src/Grit.h
TextListOverlayElement.o: ../src/BulletDebugDrawer.h
TextListOverlayElement.o: ../src/lua_userdata_dependency_tracker.h
lua_utf8.o: ../src/lua_utf8.h ../src/lua_wrappers_common.h
lua_utf8.o: ../src/lua_util.h ../src/CentralisedLog.h
lua_utf8.o: ../src/console_colour.h ../src/math_util.h
lua_util.o: ../src/lua_util.h ../src/CentralisedLog.h
lua_util.o: ../src/console_colour.h ../src/math_util.h
lua_wrappers_core.o: ../src/Grit.h ../src/BulletDebugDrawer.h
lua_wrappers_core.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_core.o: ../src/Keyboard.h ../src/Mouse.h
lua_wrappers_core.o: ../src/BackgroundMeshLoader.h
lua_wrappers_core.o: ../src/CentralisedLog.h ../src/console_colour.h
lua_wrappers_core.o: ../src/sleep.h ../src/clipboard.h ../src/HUD.h
lua_wrappers_core.o: ../src/TextListOverlayElement.h
lua_wrappers_core.o: ../src/lua_wrappers_primitives.h
lua_wrappers_core.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_core.o: ../src/math_util.h ../src/lua_wrappers_physics.h
lua_wrappers_core.o: ../src/PhysicsWorld.h ../src/TColParser.h
lua_wrappers_core.o: ../src/TColLexer ../src/TColLexer-token_ids
lua_wrappers_core.o: ../src/CollisionMesh.h ../src/LooseEnd.h
lua_wrappers_core.o: ../src/GritObject.h ../src/Streamer.h
lua_wrappers_core.o: ../src/GritClass.h ../src/ExternalTable.h
lua_wrappers_core.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_core.o: ../src/SSEAllocator.h ../src/lua_wrappers_mobj.h
lua_wrappers_core.o: ../src/lua_wrappers_scnmgr.h
lua_wrappers_core.o: ../src/lua_wrappers_material.h
lua_wrappers_core.o: ../src/lua_wrappers_mesh.h
lua_wrappers_core.o: ../src/lua_wrappers_gpuprog.h
lua_wrappers_core.o: ../src/lua_wrappers_tex.h
lua_wrappers_core.o: ../src/lua_wrappers_render.h
lua_wrappers_core.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_core.o: ../src/lua_wrappers_hud.h ../src/path_util.h
lua_wrappers_core.o: ../src/lua_utf8.h
lua_wrappers_gpuprog.o: ../src/lua_wrappers_gpuprog.h
lua_wrappers_gpuprog.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_gpuprog.o: ../src/CentralisedLog.h
lua_wrappers_gpuprog.o: ../src/console_colour.h ../src/math_util.h
lua_wrappers_gritobj.o: ../src/GritObject.h ../src/PhysicsWorld.h
lua_wrappers_gritobj.o: ../src/CentralisedLog.h
lua_wrappers_gritobj.o: ../src/console_colour.h ../src/TColParser.h
lua_wrappers_gritobj.o: ../src/TColLexer ../src/TColLexer-token_ids
lua_wrappers_gritobj.o: ../src/math_util.h ../src/CollisionMesh.h
lua_wrappers_gritobj.o: ../src/LooseEnd.h ../src/Streamer.h
lua_wrappers_gritobj.o: ../src/GritClass.h ../src/ExternalTable.h
lua_wrappers_gritobj.o: ../src/lua_util.h
lua_wrappers_gritobj.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_gritobj.o: ../src/SSEAllocator.h
lua_wrappers_gritobj.o: ../src/BackgroundMeshLoader.h ../src/Grit.h
lua_wrappers_gritobj.o: ../src/BulletDebugDrawer.h
lua_wrappers_gritobj.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_common.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_physics.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_scnmgr.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_primitives.h
lua_wrappers_gritobj.o: ../src/lua_wrappers_core.h ../src/path_util.h
lua_wrappers_hud.o: ../src/HUD.h ../src/TextListOverlayElement.h
lua_wrappers_hud.o: ../src/lua_wrappers_hud.h
lua_wrappers_hud.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_hud.o: ../src/CentralisedLog.h ../src/console_colour.h
lua_wrappers_hud.o: ../src/math_util.h ../src/lua_wrappers_material.h
lua_wrappers_material.o: ../src/lua_wrappers_material.h
lua_wrappers_material.o: ../src/lua_wrappers_common.h
lua_wrappers_material.o: ../src/lua_util.h ../src/CentralisedLog.h
lua_wrappers_material.o: ../src/console_colour.h ../src/math_util.h
lua_wrappers_mesh.o: ../src/lua_wrappers_mesh.h
lua_wrappers_mesh.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_mesh.o: ../src/CentralisedLog.h ../src/console_colour.h
lua_wrappers_mesh.o: ../src/math_util.h ../src/lua_wrappers_material.h
lua_wrappers_mobj.o: ../src/Grit.h ../src/BulletDebugDrawer.h
lua_wrappers_mobj.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_mobj.o: ../src/LuaParticleSystem.h
lua_wrappers_mobj.o: ../src/lua_wrappers_mobj.h
lua_wrappers_mobj.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_mobj.o: ../src/CentralisedLog.h ../src/console_colour.h
lua_wrappers_mobj.o: ../src/math_util.h ../src/lua_wrappers_scnmgr.h
lua_wrappers_mobj.o: ../src/lua_wrappers_primitives.h
lua_wrappers_mobj.o: ../src/lua_wrappers_material.h
lua_wrappers_mobj.o: ../src/lua_wrappers_mesh.h
lua_wrappers_mobj.o: ../src/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_physics.o: ../src/PhysicsWorld.h ../src/CentralisedLog.h
lua_wrappers_physics.o: ../src/console_colour.h ../src/TColParser.h
lua_wrappers_physics.o: ../src/TColLexer ../src/TColLexer-token_ids
lua_wrappers_physics.o: ../src/math_util.h ../src/CollisionMesh.h
lua_wrappers_physics.o: ../src/LooseEnd.h ../src/GritObject.h
lua_wrappers_physics.o: ../src/Streamer.h ../src/GritClass.h
lua_wrappers_physics.o: ../src/ExternalTable.h ../src/lua_util.h
lua_wrappers_physics.o: ../src/CacheFriendlyRangeSpaceSIMD.h
lua_wrappers_physics.o: ../src/SSEAllocator.h
lua_wrappers_physics.o: ../src/BackgroundMeshLoader.h
lua_wrappers_physics.o: ../src/lua_wrappers_primitives.h
lua_wrappers_physics.o: ../src/lua_wrappers_common.h
lua_wrappers_physics.o: ../src/lua_wrappers_physics.h
lua_wrappers_physics.o: ../src/lua_wrappers_gritobj.h
lua_wrappers_primitives.o: ../src/lua_wrappers_primitives.h
lua_wrappers_primitives.o: ../src/lua_wrappers_common.h
lua_wrappers_primitives.o: ../src/lua_util.h ../src/CentralisedLog.h
lua_wrappers_primitives.o: ../src/console_colour.h ../src/math_util.h
lua_wrappers_primitives.o: ../src/SplineTable.h
lua_wrappers_render.o: ../src/Grit.h ../src/BulletDebugDrawer.h
lua_wrappers_render.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_render.o: ../src/lua_wrappers_render.h
lua_wrappers_render.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_render.o: ../src/CentralisedLog.h ../src/console_colour.h
lua_wrappers_render.o: ../src/math_util.h ../src/lua_wrappers_tex.h
lua_wrappers_render.o: ../src/lua_wrappers_mobj.h
lua_wrappers_render.o: ../src/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_scnmgr.o: ../src/Grit.h ../src/BulletDebugDrawer.h
lua_wrappers_scnmgr.o: ../src/lua_userdata_dependency_tracker.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_scnmgr.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_scnmgr.o: ../src/CentralisedLog.h ../src/console_colour.h
lua_wrappers_scnmgr.o: ../src/math_util.h ../src/lua_wrappers_mobj.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_primitives.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_material.h
lua_wrappers_scnmgr.o: ../src/lua_wrappers_tex.h
lua_wrappers_scnmgr.o: ../src/lua_userdata_dependency_tracker_funcs.h
lua_wrappers_tex.o: ../src/lua_wrappers_tex.h
lua_wrappers_tex.o: ../src/lua_wrappers_common.h ../src/lua_util.h
lua_wrappers_tex.o: ../src/CentralisedLog.h ../src/console_colour.h
lua_wrappers_tex.o: ../src/math_util.h ../src/lua_wrappers_render.h
lua_wrappers_tex.o: ../src/lua_wrappers_scnmgr.h
main.o: ../src/linux/KeyboardX11.h ../src/Keyboard.h
main.o: ../src/linux/MouseX11.h ../src/Mouse.h ../src/Grit.h
main.o: ../src/BulletDebugDrawer.h
main.o: ../src/lua_userdata_dependency_tracker.h
main.o: ../src/BackgroundMeshLoader.h ../src/CentralisedLog.h
main.o: ../src/console_colour.h ../src/LuaParticleSystem.h
path_util.o: ../src/CentralisedLog.h ../src/console_colour.h
path_util.o: ../src/path_util.h ../src/lua_util.h ../src/math_util.h
ray.o: ../src/ray.h
unicode_util.o: ../src/unicode_util.h
KeyboardX11.o: ../src/linux/KeyboardX11.h ../src/Keyboard.h
KeyboardX11.o: ../src/CentralisedLog.h ../src/console_colour.h
KeyboardX11.o: ../src/unicode_util.h
MouseX11.o: ../src/linux/MouseX11.h ../src/Mouse.h
MouseX11.o: ../src/CentralisedLog.h ../src/console_colour.h
KeyboardDirectInput8.o: ../src/win32/KeyboardDirectInput8.h
KeyboardDirectInput8.o: ../src/Keyboard.h
KeyboardDirectInput8.o: ../src/CentralisedLog.h
KeyboardDirectInput8.o: ../src/console_colour.h
KeyboardWinAPI.o: ../src/win32/KeyboardWinAPI.h
KeyboardWinAPI.o: ../src/Keyboard.h ../src/CentralisedLog.h
KeyboardWinAPI.o: ../src/console_colour.h ../src/unicode_util.h
MouseDirectInput8.o: ../src/win32/MouseDirectInput8.h
MouseDirectInput8.o: ../src/Mouse.h ../src/CentralisedLog.h
MouseDirectInput8.o: ../src/console_colour.h
win32_clipboard.o: ../src/unicode_util.h
