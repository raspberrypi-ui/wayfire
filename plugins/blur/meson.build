blur = shared_module('blur',
                       ['blur.cpp', 'blur-base.cpp', 'box.cpp', 'gaussian.cpp',
                         'kawase.cpp', 'bokeh.cpp'],
                       include_directories: [wayfire_api_inc, wayfire_conf_inc],
                       dependencies: [wlroots, pixman, wfconfig],
                       install: true,
                       install_dir: join_paths(get_option('libdir'), 'wayfire'))
