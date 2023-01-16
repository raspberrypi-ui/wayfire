#include "wayfire/action/action.hpp"
#include "wayfire/lexer/lexer.hpp"
#include "wayfire/lexer/symbol.hpp"
#include "wayfire/parser/action_parser.hpp"
#include "wayfire/variant.hpp"
#include <memory>
#include <stdexcept>
#include <vector>

namespace wf
{

std::shared_ptr<action_t> action_parser_t::parse(lexer_t &lexer)
{
    auto symbol = lexer.parse_symbol();
    if (symbol.type != symbol_t::type_t::IDENTIFIER)
    {
        throw std::runtime_error("Action parser error. Expected identifier.");
    }
    auto name = get_string(symbol.value);

    std::vector<variant_t> args;
    auto done = false;
    while (!done)
    {
        symbol = lexer.parse_symbol();
        if ((symbol.type == symbol_t::type_t::LITERAL) || (symbol.type == symbol_t::type_t::IDENTIFIER))
        {
            args.push_back(symbol.value);
        }
        else
        {
            done = true;
            if (symbol.type != symbol_t::type_t::END)
            {
                lexer.reverse();
            }
        }
    }

    return std::make_shared<action_t>(name, args);
}

} // End namespace wf.
