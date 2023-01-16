#include "wayfire/rule/rule.hpp"
#include "wayfire/action/action.hpp"
#include "wayfire/condition/condition.hpp"
#include <memory>
#include <string>

namespace wf
{

rule_t::rule_t(const std::string &signal,
               std::shared_ptr<condition_t> condition,
               std::shared_ptr<action_t> if_action,
               std::shared_ptr<action_t> else_action) :
    _signal(signal),
    _condition(condition),
    _if_action(if_action),
    _else_action(else_action)
{
}

bool rule_t::apply(const std::string &signal, access_interface_t &access, action_interface_t &action)
{
    if ((signal.empty()) || (_condition == nullptr) || (_if_action == nullptr))
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
                error = _if_action->execute(action);
            }
            else
            {
                if (_else_action != nullptr)
                {
                    error = _else_action->execute(action);
                }
            }
        }
    }

    return error;
}

std::string rule_t::to_string() const
{
    std::string out = "rule: [signal: ";
    out.append(_signal).append(", condition: ");
    if (_condition)
    {
        out.append(_condition->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(", if_action: ");
    if (_if_action)
    {
        out.append(_if_action->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append(", else_action: ");
    if (_else_action)
    {
        out.append(_else_action->to_string());
    }
    else
    {
        out.append("nullptr");
    }
    out.append("]");
    return out;
}

} // End namespace wf.
