#ifndef RULE_HPP
#define RULE_HPP

#include <memory>
#include <string>

namespace wf
{

class action_t;
class condition_t;
class access_interface_t;
class action_interface_t;

/**
 * @brief The rule_t class is a rule (combination of condition and actions) that can be executed against a
 *        rule_interface_t instance.
 */
class rule_t
{
public:
    /**
     * @brief rule_t Constructor. Creates a new rule_t.
     *
     * The else_action argument may be nullptr in case the user wants a rule with only an if action.
     *
     * @param[in] signal The signal which should trigger the evaluation of the rule_t instance.
     * @param[in] condition The condition to be tested when triggered.
     * @param[in] if_action The action to execute in case the condition check returns <code>true</code>.
     * @param[in] else_action The action to execute in case the confition check returns <code>false</code>.
     */
    rule_t(const std::string &signal,
           std::shared_ptr<condition_t> condition,
           std::shared_ptr<action_t> if_action,
           std::shared_ptr<action_t> else_action);

    /**
     * @brief apply Applies the rule to the given trigger signal. The interface rule_interface_t is used to
     *              obtain data and execute actions.
     *
     * The rule will not do anything if the signal does not match the value of _signal. It is implemented this
     * way to allow a set of rules to be 'applied' on all signals, and only have the relevant rules execute.
     * Think of this as a built in signal filter.
     *
     * @param[in] signal The signal to apply the rule to.
     * @param[in] access Acessor interface for the condition checks.
     * @param[in] action Action interface for supporting the execution of actions.
     *
     * @return <code>True</code> if there was an error, <code>false</code> if not.
     */
    bool apply(const std::string &signal, access_interface_t &access, action_interface_t &action);

    /**
     * @brief to_string Generates a string representation of the rule. Useful for debugging.
     *
     * @return The string representation of the rule.
     */
    std::string to_string() const;
private:
    /**
     * @brief _signal The signal that should trigger the application of this rule.
     */
    std::string _signal;

    /**
     * @brief _condition The condition to check to decide which action to execute.
     */
    std::shared_ptr<condition_t> _condition;

    /**
     * @brief _if_action The action to execute if the condition holds.
     */
    std::shared_ptr<action_t> _if_action;

    /**
     * @brief _else_action The action to execute if the condition does not hold.
     */
    std::shared_ptr<action_t> _else_action;
};

} // End namespace wf.

#endif // RULE_HPP
