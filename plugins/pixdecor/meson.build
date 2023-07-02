pixdecor = shared_module('pixdecor',
        ['decoration.cpp', 'deco-subsurface.cpp', 'deco-button.cpp', 'deco-layout.cpp', 'deco-theme.cpp'],
        include_directories: [wayfire_api_inc, wayfire_conf_inc, plugins_common_inc],
        dependencies: [wlroots, pixman, wfconfig, cairo, pango, pangocairo],
        install: true,
        install_dir: join_paths(get_option('libdir'), 'wayfire'))
