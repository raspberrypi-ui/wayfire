TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    wayfire/lexer/lexer.cpp \
    wayfire/lexer/literal.cpp \
    wayfire/lexer/symbol.cpp \
    wayfire/parser/lambda_rule_parser.cpp \
    wayfire/parser/rule_parser.cpp \
    wayfire/parser/action_parser.cpp \
    wayfire/parser/condition_parser.cpp \
    wayfire/action/action.cpp \
    wayfire/rule/lambda_rule.cpp \
    wayfire/rule/rule.cpp \
    wayfire/condition/condition.cpp \
    wayfire/condition/logic_condition.cpp \
    wayfire/condition/test_condition.cpp \
    wayfire/variant.cpp \
    main.cpp \

HEADERS += \
    wayfire/action/action.hpp \
    wayfire/action/action_interface.hpp \
    wayfire/lexer/lexer.hpp \
    wayfire/lexer/literal.hpp \
    wayfire/lexer/symbol.hpp \
    wayfire/parser/lambda_rule_parser.hpp \
    wayfire/parser/rule_parser.hpp \
    wayfire/parser/condition_parser.hpp \
    wayfire/parser/action_parser.hpp \
    wayfire/rule/lambda_rule.hpp \
    wayfire/utils.hpp \
    wayfire/condition/access_interface.hpp \
    wayfire/condition/condition.hpp \
    wayfire/rule/rule.hpp \
    wayfire/condition/logic_condition.hpp \
    wayfire/condition/test_condition.hpp \
    wayfire/variant.hpp

DISTFILES += \
    meson.build \
    Makefile \
    README.md \
    Doxyfile.in
