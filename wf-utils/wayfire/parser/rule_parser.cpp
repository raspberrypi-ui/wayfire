#include "wayfire/condition/test_condition.hpp"
#include "wayfire/lexer/lexer.hpp"
#include "wayfire/lexer/symbol.hpp"
#include "wayfire/parser/action_parser.hpp"
#include "wayfire/parser/condition_parser.hpp"
#include "wayfire/parser/rule_parser.hpp"
#include "wayfire/rule/rule.hpp"
#include "wayfire/variant.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace wf
{

class action_t;
class condition_t;

std::shared_ptr<rule_t> rule_parser_t::parse(const std::string &text)
{
    lexer_t lexer(text);
    return parse(lexer);
}

std::shared_ptr<rule_t> rule_parser_t::parse(lexer_t &lexer)
{
    std::string signal;
    std::shared_ptr<condition_t> condition;
    std::shared_ptr<action_t> if_action;
    std::shared_ptr<action_t> else_action;

    try
    {
        // Expect the first symbol to be the 'on' keyword.
        auto symbol = lexer.parse_symbol();
        if ((symbol.type != symbol_t::type_t::KEYWORD) || (get_string(symbol.value) != "on"))
        {
            throw std::runtime_error("Rule parser error. Expected 'on' keyword.");
        }

        // Expect a signal next.
        symbol = lexer.parse_symbol();
        if (symbol.type != symbol_t::type_t::SIGNAL)
        {
            throw std::runtime_error("Rule parser error. Expected signal.");
        }
        signal = get_string(symbol.value);

        // Expect the 'if' or 'then' keyword next.
        symbol = lexer.parse_symbol();
        if ((symbol.type != symbol_t::type_t::KEYWORD) || ((get_string(symbol.value) != "if") && (get_string(symbol.value) != "then")))
        {
            throw std::runtime_error("Rule parser error. Expected 'if' or 'then' keyword.");
        }

        // Check if keyword was if -> condition parser or else -> true condition and no else allowed.
        auto else_allowed = true;
        auto keyword = get_string(symbol.value);
        if (keyword == "if")
        {
            // Delegate to logical condition parser.
            condition = condition_parser_t().parse(lexer);

            // Expect the next symbol to be the 'then' keyword.
            symbol = lexer.parse_symbol();
            if ((symbol.type != symbol_t::type_t::KEYWORD) || (get_string(symbol.value) != "then"))
            {
                throw std::runtime_error("Rule parser error. Expected 'then' keyword.");
            }
        }
        else
        {
            condition = std::make_shared<true_condition_t>();
            else_allowed = false;
        }

        // Delegate to action parser.
        if_action = action_parser_t().parse(lexer);

        // Check if there is an else symbol.
        symbol = lexer.parse_symbol();
        if (symbol.type == symbol_t::type_t::KEYWORD)
        {
            if (get_string(symbol.value) != "else")
            {
                throw std::runtime_error("Rule parser error. Expected 'else' keyword.");
            }
            else
            {
                if (!else_allowed)
                {
                    throw std::runtime_error("Rule parser error. Else action is not allowed if no condition set.");
                }
            }

            // Delegate to action parser.
            else_action = action_parser_t().parse(lexer);

            // Advance to next symbol. Expect EOL.
            symbol = lexer.parse_symbol();
        }

        // Expect END.
        if (symbol.type != symbol_t::type_t::END)
        {
            std::string error = "Rule parser error. Unexpected symbol: ";
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

    return std::make_shared<rule_t>(signal, condition, if_action, else_action);
}

} // End namespace wf.
