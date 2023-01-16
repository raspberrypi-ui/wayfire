#include "wayfire/lexer/literal.hpp"
#include "wayfire/variant.hpp"
#include <ctype.h>
#include <stdexcept>
#include <string>

namespace wf
{

variant_t parse_literal(const std::string &s)
{
    // Deal with char literal
    if ((s.front() == '\'') && (s.back() == '\''))
    {
        char c = s.at(1);
        if (c == '\\')
        {
            if (s.size() != 4)
            {
                std::string error = "Literal parser error. Text could not be converted to char. text:";
                error.append(s);
                throw std::runtime_error(error);
            }

            switch (s.at(2))
            {
            case 'a': // bell
                c = '\a';
                break;
            case 'b': // backspace
                c = '\b';
                break;
            case 'e': // escape
                c = 0x1b;
                break;
            case 'f': // form feed
                c = '\f';
                break;
            case 'n': // newline
                c = '\n';
                break;
            case 'r': // carriage return
                c = '\r';
                break;
            case 't': // tab
                c = '\t';
                break;
            case 'v': // vertical tab
                c = '\v';
                break;
            case '\\': // backslash
                c = '\\';
                break;
            case '\'': // apostrophe
                c = '\'';
                break;
            case '\"': // quotation mark
                c = '\"';
                break;
            case '\?': // question mark
                c = '\?';
                break;
            }
        }
        else
        {
            if (s.size() != 3)
            {
                std::string error = "Literal parser error. Text could not be converted to char. text:";
                error.append(s);
                throw std::runtime_error(error);
            }
        }

        return variant_t(c);
    }

    // Deal with boolean literal.
    if ((s == "true") || (s == "TRUE") || (s == "True"))
    {
        return variant_t(true);
    }
    if ((s == "false") || (s == "FALSE") || (s == "False"))
    {
        return variant_t(false);
    }

    // Deal with float or double here.
    if (s.find('.') != std::string::npos)
    {
        // Float or Double.
        if ((s.find('f') != std::string::npos) || (s.find('F') != std::string::npos))
        {
            // Float.
            try
            {
                return variant_t(std::stof(s));
            }
            catch (std::logic_error &)
            {
                std::string error = "Literal parser error. Text could not be converted to float. text:";
                error.append(s);
                throw std::runtime_error(error);
            }
        }
        else
        {
            // Double.
            try
            {
                return variant_t(std::stod(s));
            }
            catch (std::logic_error &)
            {
                std::string error = "Literal parser error. Text could not be converted to double. text:";
                error.append(s);
                throw std::runtime_error(error);
            }
        }
    }

    // Assume literal will be an integer and test the assumption by looking at all characters.
    auto integer = true;
    auto first = true;
    for (const auto &c : s)
    {
        auto digit = std::isdigit(c);
        auto hyphen = (c == '-');

        if (first)
        {
            first = false;
            if (!(digit | hyphen))
            {
                integer = false;
                break;
            }
        }
        else
        {
            if (!(digit))
            {
                integer = false;
                break;
            }
        }
    }
    if (integer)
    {
        try
        {
            return variant_t(std::stoi(s));
        }
        catch (std::logic_error &)
        {
            std::string error = "Literal parser error. Text could not be converted to int. text:";
            error.append(s);
            throw std::runtime_error(error);
        }
    }

    // If we get here, the text is a string.
    return variant_t(s);
}

} // End namespace wf.
