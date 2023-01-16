#include "wayfire/condition/condition.hpp"
#include "wayfire/condition/logic_condition.hpp"
#include "wayfire/condition/test_condition.hpp"
#include "wayfire/lexer/lexer.hpp"
#include "wayfire/lexer/symbol.hpp"
#include "wayfire/parser/condition_parser.hpp"
#include "wayfire/variant.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace wf
{

condition_parser_t::condition_parser_t()
{
}

std::shared_ptr<condition_t> condition_parser_t::parse(lexer_t &lexer)
{
    // Peek at the first symbol to see if we consider a catch-all or catch-none condition.
    auto symbol = lexer.parse_symbol();
    lexer.reverse();

    if (symbol.type == symbol_t::type_t::KEYWORD)
    {
        auto keyword = get_string(symbol.value);
        if (keyword == "all")
        {
            _root = std::make_shared<true_condition_t>();
        }
        else if (keyword == "none")
        {
            _root = std::make_shared<false_condition_t>();
        }
        else
        {
            throw std::runtime_error("Condition parser error. Single keyword rules must be 'all' or 'none'.");
        }
    }
    else
    {
        _expression(lexer);
        lexer.reverse();
    }

    std::cout << _root->to_string() << std::endl;

    return _root;
}

void condition_parser_t::_expression(lexer_t &lexer)
{
    _term(lexer);
    while ((_symbol.type == symbol_t::type_t::OPERATOR) && (get_string(_symbol.value) == "|"))
    {
        auto or_condition = std::make_shared<or_condition_t>();
        or_condition->left = _root;
        _term(lexer);
        or_condition->right = _root;
        _root = or_condition;
    }
}

void condition_parser_t::_term(lexer_t &lexer)
{
    _factor(lexer);
    while ((_symbol.type == symbol_t::type_t::OPERATOR) && (get_string(_symbol.value) == "&"))
    {
        auto and_condition = std::make_shared<and_condition_t>();
        and_condition->left = _root;
        _factor(lexer);
        and_condition->right = _root;
        _root = and_condition;
    }
}

void condition_parser_t::_factor(lexer_t &lexer)
{
    _symbol = lexer.parse_symbol();

    if (_symbol.type == symbol_t::type_t::IDENTIFIER)
    {
        // Identifier.
        auto identifier = get_string(_symbol.value);

        // Keyword.
        _symbol = lexer.parse_symbol();
        if (_symbol.type != symbol_t::type_t::KEYWORD)
        {
            throw std::runtime_error("Condition parser error. Expected keyword.");
        }
        auto keyword = get_string(_symbol.value);
        if ((keyword != "equals") && (keyword != "contains") && (keyword != "is"))
        {
            std::string error = "Condition parser error. Unsupported keyword. keyword: ";
            error.append(keyword);
            throw std::runtime_error(error);
        }

        // Value.
        _symbol = lexer.parse_symbol();
        if (_symbol.type != symbol_t::type_t::LITERAL)
        {
            throw std::runtime_error("Condition parser error. Expected literal.");
        }

        // Create condition.
        if (keyword == "equals" || keyword == "is")
        {
            _root = std::make_shared<equals_condition_t>(identifier, _symbol.value);
        }
        if (keyword == "contains")
        {
            _root = std::make_shared<contains_condition_t>(identifier, _symbol.value);
        }

        _symbol = lexer.parse_symbol();
    }
    else if ((_symbol.type == symbol_t::type_t::OPERATOR) && (get_string(_symbol.value) == "!"))
    {
        auto not_condition = std::make_shared<not_condition_t>();
        _factor(lexer);
        not_condition->child = _root;
        _root = not_condition;
    }
    else if ((_symbol.type == symbol_t::type_t::STRUCTURAL) && (get_string(_symbol.value) == "("))
    {
        _expression(lexer);
        // Expect ) and discard it
        if (_symbol.type != symbol_t::type_t::STRUCTURAL || (get_string(_symbol.value) != ")"))
        {
            throw std::runtime_error("Condition parser error. Expected \')\'");
        }
        _symbol = lexer.parse_symbol();
    }
    else {
        throw std::runtime_error("Condition parser error. Unexpected symbol.");
    }
}

} // End namespace wf.
