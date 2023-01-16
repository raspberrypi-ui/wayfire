project(
        'wf-utils',
        'cpp',
        version: '0.4.0',
        license: 'MIT',
        meson_version: '>=0.50.0',
        default_options: [
        'cpp_std=c++17',
                'warning_level=2',
                'werror=false',
        ],
)

add_project_arguments(['-Wno-deprecated-declarations'], language: ['cpp'])

sources = [
'wayfire/action/action.cpp',
'wayfire/condition/condition.cpp',
'wayfire/condition/logic_condition.cpp',
'wayfire/condition/test_condition.cpp',
'wayfire/lexer/lexer.cpp',
'wayfire/lexer/literal.cpp',
'wayfire/lexer/symbol.cpp',
'wayfire/parser/action_parser.cpp',
'wayfire/parser/condition_parser.cpp',
'wayfire/parser/lambda_rule_parser.cpp',
'wayfire/parser/rule_parser.cpp',
'wayfire/rule/lambda_rule.cpp',
'wayfire/rule/rule.cpp',
'wayfire/variant.cpp',
]

wfutils_inc = include_directories('.')

lib_wfutils = library('wf-utils',
    sources,
    dependencies: [],
    include_directories: wfutils_inc,
    install: true,
    version: meson.project_version())

pkgconfig = import('pkgconfig')
pkgconfig.generate(
    libraries:    lib_wfutils,
    version:      meson.project_version(),
    filebase:     meson.project_name(),
    name:         meson.project_name(),
    description: 'Utilities library for Wayfire')

install_headers([], subdir: 'wayfire/utils')

wfutils = declare_dependency(link_with: lib_wfutils,
    include_directories: wfutils_inc)

# -------------------------------------------------------------------------------------------------
# Install headers
headers_action = [
'wayfire/action/action.hpp',
'wayfire/action/action_interface.hpp',
]

headers_condition = [
'wayfire/condition/access_interface.hpp',
'wayfire/condition/condition.hpp',
'wayfire/condition/logic_condition.hpp',
'wayfire/condition/test_condition.hpp',
]

headers_lexer = [
'wayfire/lexer/lexer.hpp',
'wayfire/lexer/literal.hpp',
'wayfire/lexer/symbol.hpp',
]

headers_parser = [
'wayfire/parser/action_parser.hpp',
'wayfire/parser/condition_parser.hpp',
'wayfire/parser/rule_parser.hpp',
]

headers_rule = [
'wayfire/rule/lambda_rule.hpp',
'wayfire/rule/rule.hpp',
]

headers_root = [
'wayfire/utils.hpp',
'wayfire/variant.hpp',
]

install_headers(headers_action, subdir: 'wayfire/action')
install_headers(headers_condition, subdir: 'wayfire/condition')
install_headers(headers_lexer, subdir: 'wayfire/lexer')
install_headers(headers_parser, subdir: 'wayfire/parser')
install_headers(headers_rule, subdir: 'wayfire/rule')
install_headers(headers_root, subdir: 'wayfire')

# -------------------------------------------------------------------------------------------------
# Unit tests
#subdir('test')

# -------------------------------------------------------------------------------------------------
# Documentation
# TODO: Figure out how to install the doc package with meson.

doxygen = find_program('doxygen', required: false)

if doxygen.found()
  cdata = configuration_data()
  cdata.set('TOP_SRCDIR', meson.source_root())
  cdata.set('TOP_BUILDDIR', meson.build_root())
  cdata.set('OUTPUT_DIR', join_paths(meson.build_root(),'docs'))
  cdata.set('README_PATH', join_paths(meson.source_root(),'README.md'))
  cdata.set('VERSION', meson.project_version())
  cdata.set('PROJECT_NAME', meson.project_name())

  doxyfile = configure_file(input: 'Doxyfile.in',
                            output: 'Doxyfile',
                            configuration: cdata,
                            install: false)

  doc_target = run_target('doc',command: [doxygen, doxyfile])
endif
