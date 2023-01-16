#include "wayfire/variant.hpp"
#include <string>
#include <variant>

namespace wf
{

bool is_int(const variant_t &v)
{
    return std::holds_alternative<int>(v);
}

bool is_char(const variant_t &v)
{
    return std::holds_alternative<char>(v);
}

bool is_bool(const variant_t &v)
{
    return std::holds_alternative<bool>(v);
}

bool is_float(const variant_t &v)
{
    return std::holds_alternative<float>(v);
}

bool is_double(const variant_t &v)
{
    return std::holds_alternative<double>(v);
}

bool is_string(const variant_t &v)
{
    return std::holds_alternative<std::string>(v);
}

variant_type_t type(const variant_t v)
{
    if (is_int(v))
    {
        return variant_type_t::INT;
    }
    if (is_char(v))
    {
        return variant_type_t::CHAR;
    }
    if (is_bool(v))
    {
        return variant_type_t::BOOL;
    }
    if (is_float(v))
    {
        return variant_type_t::FLOAT;
    }
    if (is_double(v))
    {
        return variant_type_t::DOUBLE;
    }

    return variant_type_t::STRING;
}

int get_int(const variant_t &v)
{
    return std::get<int>(v);
}

char get_char(const variant_t &v)
{
    return std::get<char>(v);
}

bool get_bool(const variant_t &v)
{
    return std::get<bool>(v);
}

float get_float(const variant_t &v)
{
    return std::get<float>(v);
}

double get_double(const variant_t &v)
{
    return std::get<double>(v);
}

std::string get_string(const variant_t &v)
{
    return std::get<std::string>(v);
}

std::string to_string(const variant_t &v)
{
    std::string out = "variant: [type: ";
    if (is_int(v))
    {
        out.append("int, value: ").append(std::to_string(get_int(v)));
    }
    else if (is_char(v))
    {
        out.append("char, value: ").append(std::to_string(get_char(v)));
    }
    else if (is_bool(v))
    {
        out.append("bool, value: ").append(std::to_string(static_cast<unsigned int>(get_bool(v))));
    }
    else if (is_float(v))
    {
        out.append("float, value: ").append(std::to_string(get_float(v)));
    }
    else if (is_double(v))
    {
        out.append("double, value: ").append(std::to_string(get_double(v)));
    }
    else if (is_string(v))
    {
        out.append("string, value: ").append(get_string(v));
    }
    else
    {
        out.append("invalid. WTF?!?");
    }

    out.append("]");
    return out;
}

} // End namespace wf.
