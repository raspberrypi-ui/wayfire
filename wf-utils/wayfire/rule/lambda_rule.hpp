#ifndef LAMBDA_RULE_HPP
#define LAMBDA_RULE_HPP

#include <functional>
#include <memory>
#include <string>

namespace wf
{

class condition_t;
class access_interface_t;

/**
 * @note The lambda function executed by this rule as if or else case returns a boolean which is
 *       <code>true</code> if there was an error. <code>False</code> if no error.
 */
using lambda_t = std::function<bool()>;

/**
 * @brief The lambda_rule_t class is a rule (combination of condition and action lambdass).
 */
class lambda_rule_t
{
  public:
    /**
     * @brief lambda_rule_t Constructor. Creates a new lambda_rule_t.
     *
     * @param[in] signal The signal which should trigger the evaluation of the rule_t instance.
     * @param[in] condition The condition to be tested when triggered.
     */
    lambda_rule_t(const std::string &signal, std::shared_ptr<condition_t> condition);

    /**
     * @brief setIfLambda Sets the lambda to be executed on the signal if the condition holds.
     *
     * @param[in] if_lambda The lambda to execute.
     */
    void setIfLambda(lambda_t if_lambda);

    /**
     * @brief setElseLambda Sets the lambda to be executed on the signal if the condition does not hold.
     *
     * @param[in] else_lambda The lambda to execute.
     */
    void setElseLambda(lambda_t else_lambda);

    /**
     * @brief apply Applies the rule to the given trigger signal. The interface access_interface_t is used to
     *              obtain data.
     *
     * The rule will not do anything if the signal does not match the value of _signal. It is implemented this
     * way to allow a set of rules to be 'applied' on all signals, and only have the relevant rules execute.
     * Think of this as a built in signal filter.
     *
     * @param[in] signal The signal to apply the rule to.
     * @param[in] access Acessor interface for the condition checks.
     *
     * @return <code>True</code> if there was an error, <code>false</code> if not.
     */
    bool apply(const std::string &signal, access_interface_t &access);

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
     * @brief _if_lambda The lambda to execute if the condition holds.
     */
    lambda_t _if_lambda;

    /**
     * @brief _else_lambda The lambda to execute if the condition does not hold.
     */
    lambda_t _else_lambda;
};

} // End namespace wf.

#endif // LAMBDA_RULE_HPP
