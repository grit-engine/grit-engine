

GSL_CPP_SRCS= \
	gfx/gfx_gasoline.cpp \
	gfx/gfx_gasoline_parser.cpp \
	gfx/gfx_gasoline_type_system.cpp \
	gfx/gfx_gasoline_backend.cpp \
	gfx/gfx_gasoline_backend_gsl.cpp \
	gfx/gfx_gasoline_backend_cg.cpp \
	gfx/gfx_gasoline_backend_glsl.cpp \


GSL_STANDALONE_CPP_SRCS= \
	gfx/gfx_gasoline_standalone.cpp \
	$(GSL_CPP_SRCS) \


COL_CONV_CPP_SRCS= \
	physics/bcol_parser.cpp \
	physics/tcol_lexer-core-engine.cpp \
	physics/tcol_lexer.cpp \
	physics/tcol_parser.cpp \


COL_CONV_STANDALONE_CPP_SRCS = \
	physics/grit_col_conv.cpp \
	$(COL_CONV_CPP_SRCS) \


ENGINE_CPP_SRCS=\
	background_loader.cpp \
	bullet_debug_drawer.cpp \
	core_option.cpp \
	dense_index_map.cpp \
	disk_resource.cpp \
	external_table.cpp \
	grit_class.cpp \
	grit_lua_util.cpp \
	grit_object.cpp \
	input_filter.cpp \
	ldbglue.cpp \
	lua_wrappers_core.cpp \
	lua_wrappers_disk_resource.cpp \
	lua_wrappers_gritobj.cpp \
	lua_wrappers_primitives.cpp \
	main.cpp \
	path_util.cpp \
	streamer.cpp \
	 \
	audio/audio.cpp \
	audio/lua_wrappers_audio.cpp \
	audio/audio_disk_resource.cpp \
	audio/ogg_vorbis_decoder.cpp \
	 \
	gfx/gfx_body.cpp \
	gfx/gfx.cpp \
	gfx/gfx_decal.cpp \
	gfx/gfx_disk_resource.cpp \
	gfx/gfx_fertile_node.cpp \
	gfx/gfx_font.cpp \
	gfx/gfx_hud.cpp \
	gfx/gfx_instances.cpp \
	gfx/gfx_light.cpp \
	gfx/gfx_material.cpp \
	gfx/gfx_node.cpp \
	gfx/gfx_option.cpp \
	gfx/gfx_particle_system.cpp \
	gfx/gfx_pipeline.cpp \
	gfx/gfx_ranged_instances.cpp \
	gfx/gfx_shader.cpp \
	gfx/gfx_sky_body.cpp \
	gfx/gfx_sky_material.cpp \
	gfx/gfx_text_buffer.cpp \
	gfx/lua_wrappers_gfx.cpp \
	gfx/lua_wrappers_gpuprog.cpp \
	gfx/lua_wrappers_material.cpp \
	 \
	navigation/chunky_tri_mesh.cpp \
	navigation/crowd_manager.cpp \
	navigation/fastlz.cpp \
	navigation/input_geom.cpp \
	navigation/lua_wrappers_navigation.cpp \
	navigation/mesh_loader_obj.cpp \
	navigation/navigation.cpp \
	navigation/navigation_interfaces.cpp \
	navigation/navigation_manager.cpp \
	 \
	net/lua_wrappers_net.cpp \
	net/net_address.cpp \
	net/net.cpp \
	net/net_manager.cpp \
	net/net_message.cpp \
	 \
	linux/keyboard_x11.cpp \
	linux/mouse_x11.cpp \
	linux/x11_clipboard.cpp \
	linux/joystick_devjs.cpp \
	 \
	physics/collision_mesh.cpp \
	physics/lua_wrappers_physics.cpp \
	physics/physical_material.cpp \
	physics/physics_world.cpp \
	$(COL_CONV_CPP_SRCS) \
	$(GSL_CPP_SRCS) \

