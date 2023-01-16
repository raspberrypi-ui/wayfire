#include "wayfire/parser/lambda_rule_parser.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "wayfire/lexer/lexer.hpp"
#include "wayfire/parser/condition_parser.hpp"
#include "wayfire/rule/lambda_rule.hpp"

namespace wf
{

std::shared_ptr<lambda_rule_t> lambda_rule_parser_t::parse(const std::string &text, lambda_t if_lambda, lambda_t else_lambda)
{
    lexer_t lexer(text);
    return parse(lexer, if_lambda, else_lambda);
}

std::shared_ptr<lambda_rule_t> lambda_rule_parser_t::parse(lexer_t &lexer, lambda_t if_lambda, lambda_t else_lambda)
{
    std::string signal;
    std::shared_ptr<condition_t> condition;

    try
    {
        // Expect the first symbol to be the 'on' keyword.
        auto symbol = lexer.parse_symbol();
        if ((symbol.type != symbol_t::type_t::KEYWORD) || (get_string(symbol.value) != "on"))
        {
            throw std::runtime_error("Lambda rule parser error. Expected 'on' keyword.");
        }

        // Expect a signal next.
        symbol = lexer.parse_symbol();
        if (symbol.type != symbol_t::type_t::SIGNAL)
        {
            throw std::runtime_error("Lambda rule parser error. Expected signal.");
        }
        signal = get_string(symbol.value);

        // Expect the 'if' keyword next.
        symbol = lexer.parse_symbol();
        if ((symbol.type != symbol_t::type_t::KEYWORD) || (get_string(symbol.value) != "if"))
        {
            throw std::runtime_error("Lambda rule parser error. Expected 'if' keyword.");
        }

        // Delegate to logical condition parser.
        condition = condition_parser_t().parse(lexer);

        // Expect the lexer to be at the end - 1.
        lexer.parse_symbol();
        symbol = lexer.parse_symbol();
        if (symbol.type != symbol_t::type_t::END)
        {
            std::string error = "Lambda rule parser error. Unexpected symbol: ";
            error.append(to_string(symbol));
            throw std::runtime_error(error);
        }
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Malformed input:" << std::endl;
        std::cout << lexer.text() << std::endl;
        std::string pad;
        auto pos = lexer.current_symbol_position();
        for (std::size_t i = 0; i < pos; ++i)
        {
            pad.append(" ");
        }
        pad.append("^ ");
        std::cout << pad << e.what() << std::endl;
    }

    auto rule = std::make_shared<lambda_rule_t>(signal, condition);
    if (rule != nullptr)
    {
        rule->setIfLambda(if_lambda);
        rule->setElseLambda(else_lambda);
    }
    return rule;
}

} // End namespace wf.
