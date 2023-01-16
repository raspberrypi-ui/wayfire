#include "wayfire/condition/condition.hpp"
#include "wayfire/condition/logic_condition.hpp"
#include <memory>

namespace wf
{

class access_interface_t;

or_condition_t::or_condition_t() : condition_t()
{
}

or_condition_t::~or_condition_t()
{
}

bool or_condition_t::evaluate(access_interface_t &interface, bool &error)
{
    if ((error) || (left == nullptr) || (right == nullptr))
    {
        error = true;
        return false;
    }

    return left->evaluate(interface, error) | right->evaluate(interface, error);
}

std::string or_condition_t::to_string() const
{
    std::string out = "(";
    if (left != nullptr)
    {
        out.append(left->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(" | ");
    if (right != nullptr)
    {
        out.append(right->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(")");
    return out;
}

and_condition_t::and_condition_t() : condition_t()
{
}

and_condition_t::~and_condition_t()
{
}

bool and_condition_t::evaluate(access_interface_t &interface, bool &error)
{
    if ((error) || (left == nullptr) || (right == nullptr))
    {
        error = true;
        return false;
    }

    return left->evaluate(interface, error) & right->evaluate(interface, error);
}

std::string and_condition_t::to_string() const
{
    std::string out = "(";
    if (left != nullptr)
    {
        out.append(left->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(" & ");
    if (right != nullptr)
    {
        out.append(right->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(")");
    return out;
}

not_condition_t::not_condition_t() : condition_t()
{
}

not_condition_t::~not_condition_t()
{
}

bool not_condition_t::evaluate(access_interface_t &interface, bool &error)
{
    if ((error) || (child == nullptr))
    {
        error = true;
        return false;
    }

    return !child->evaluate(interface, error);
}

std::string not_condition_t::to_string() const
{
    std::string out = "(!";
    if (child != nullptr)
    {
        out.append(child->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(")");
    return out;
}

} // End namespace wf.
