animiate = shared_module('animate',
                         ['animate.cpp',
                          'fire/particle.cpp',
                          'fire/fire.cpp'],
                         include_directories: [wayfire_api_inc, wayfire_conf_inc],
                         dependencies: [wlroots, pixman, wfconfig],
                         install: true,
                         install_dir: join_paths(get_option('libdir'), 'wayfire'))
