shadows = shared_module('winshadows', 
	['shadows.cpp', 'shadow-surface.cpp', 'deco-shadow.cpp', 'deco-shadow.glsl.cpp'],
    	include_directories: [wayfire_api_inc, wayfire_conf_inc, plugins_common_inc],
    	dependencies: [wlroots, pixman, wf_protos, wfconfig, cairo, pango, pangocairo],
    	install: true,
    	install_dir: join_paths( get_option( 'libdir' ), 'wayfire' ))
