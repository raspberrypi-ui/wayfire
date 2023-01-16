#include "wayfire/lexer/symbol.hpp"
#include "wayfire/utils.hpp"
#include "wayfire/variant.hpp"
#include <ctype.h>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

namespace wf
{

symbol_t::symbol_t() : type(type_t::END), value(std::string(""))
{
}

symbol_t::symbol_t(type_t t) : type(t), value(std::string(""))
{
}

symbol_t::symbol_t(const variant_t &v) : type(type_t::LITERAL), value(v)
{
    // String literals can also be keywords, comments or identifiers. Check for this.
    if (is_string(value))
    {
        auto literal_value = get_string(value);

        if ((literal_value.front() == '\"') && (literal_value.back() == '\"'))
        {
            value = literal_value.substr(1, literal_value.size() - 2); // Strip quotes here.
        }
        else if (contains(SIGNALS, literal_value))
        {
            type = type_t::SIGNAL;
        }
        else if (contains(KEYWORDS, literal_value))
        {
            type = type_t::KEYWORD;
        }
        else if (contains(STRUCTURES, literal_value))
        {
            type = type_t::STRUCTURAL;
        }
        else if (contains(OPERATORS, literal_value))
        {
            type = type_t::OPERATOR;
        }
        else
        {
            bool first = true;
            for (const auto &c : literal_value)
            {
                auto digit = std::isdigit(c);
                auto letter = std::isalpha(c);
                auto underscore = (c == '_');

                if (first)
                {
                    first = false;
                    if (digit)
                    {
                        std::string error = "Symbol parser error. Identifier should start with _ or letter. text:";
                        error.append(literal_value);
                        throw std::runtime_error(error);
                    }
                }

                if (!(digit | letter | underscore))
                {
                    std::string error = "Symbol parser error. Identifier contains invalid character. text:";
                    error.append(literal_value);
                    throw std::runtime_error(error);
                }
            }
            type = type_t::IDENTIFIER;
        }
    }
}

std::string to_string(const symbol_t &s)
{
    std::string out = "Symbol [";
    switch (s.type)
    {
    case symbol_t::type_t::END:
        out.append("END");
        break;
    case symbol_t::type_t::IDENTIFIER:
        out.append("Identifier: ").append(get_string(s.value));
        break;
    case symbol_t::type_t::LITERAL:
        out.append("Literal: [");
        out.append(to_string(s.value));
        out.append("]");
        break;
    case symbol_t::type_t::OPERATOR:
        out.append("Operator: ").append(get_string(s.value));
        break;
    case symbol_t::type_t::KEYWORD:
        out.append("Keyword: ").append(get_string(s.value));
        break;
    case symbol_t::type_t::STRUCTURAL:
        out.append("Structural: ").append(get_string(s.value));
        break;
    case symbol_t::type_t::SIGNAL:
        out.append("Signal: ").append(get_string(s.value));
        break;
    }
    out.append("]");
    return out;
}

} // End namespace wf.
