#include "wayfire/rule/lambda_rule.hpp"
#include "wayfire/condition/condition.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

namespace wf
{

lambda_rule_t::lambda_rule_t(const std::string &signal, std::shared_ptr<condition_t> condition) :
    _signal(signal), _condition(condition)
{
}

void lambda_rule_t::setIfLambda(lambda_t if_lambda)
{
    _if_lambda = if_lambda;
}

void lambda_rule_t::setElseLambda(lambda_t else_lambda)
{
    _else_lambda = else_lambda;
}

bool lambda_rule_t::apply(const std::string &signal, access_interface_t &access)
{
    if ((signal.empty()) || (_condition == nullptr) || (_if_lambda == nullptr))
    {
        return true;
    }

    bool error = false;
    if (signal == _signal)
    {
        auto check_result = _condition->evaluate(access, error);
        if (!error)
        {
            if (check_result)
            {
                error = _if_lambda();
            }
            else
            {
                if (_else_lambda != nullptr)
                {
                    error = _else_lambda();
                }
            }

        }
    }

    return error;
}

std::string lambda_rule_t::to_string() const
{
    std::string out = "lambda rule: [signal: ";
    out.append(_signal).append(", condition: ");
    if (_condition)
    {
        out.append(_condition->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(", if_lambda: ");
    if (_if_lambda)
    {
        auto uint_ptr = reinterpret_cast<std::uintptr_t>(&_if_lambda);
        std::stringstream ss;
        ss << "0x" << std::hex << uint_ptr;
        out.append(ss.str());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(", else_lambda: ");
    if (_else_lambda)
    {
        auto uint_ptr = reinterpret_cast<std::uintptr_t>(&_else_lambda);
        std::stringstream ss;
        ss << "0x" << std::hex << uint_ptr;
        out.append(ss.str());
    }
    else
    {
        out.append("nullptr");
    }
    out.append("]");
    return out;
}

} // End namespace wf.
