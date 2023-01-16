#ifndef LAMBDA_RULE_PARSER_HPP
#define LAMBDA_RULE_PARSER_HPP

#include <memory>
#include <string>

#include "wayfire/rule/lambda_rule.hpp"

namespace wf
{

class lexer_t;

/**
 * @brief The lambda_rule_parser_t class will parse a rule_t from a lexer_t instance.
 */
class lambda_rule_parser_t
{
  public:
    /**
     * @brief parse Convenience method. Takes the text and makes a lexer_t instance which is then used to parse the lambda_rule_t.
     *
     * This method is convenient, but not recommended if you have multiple rules to parse. In the multiple rules case,
     * it is smarter to make 1 lexer_t instance outside and reset it to a new text, thus reusing the lexer_t instance.
     * Reuse is good for the environment!
     *
     * This method will throw std::runtime_error in case the text is malformed.
     *
     * @param[in] text The text to parse a lambda_rule_t from.
     * @param[in] if_lambda The lambda method to execute if the condition holds.
     * @param[in] else_lambda The lambda method to execute if the condition does not hold.
     *
     * @return The parsed rule.
     */
    std::shared_ptr<lambda_rule_t> parse(const std::string &text, lambda_t if_lambda, lambda_t else_lambda);

    /**
     * @brief parse Takes the lexer_t instance and parses a lambda_rule_t from it.
     *
     * This method will throw std::runtime_error incase the lexer_t instance has malformed text in it.
     *
     * @param[in] lexer The lexer_t instance to parse the lambda_rule_t from.
     * @param[in] if_lambda The lambda method to execute if the condition holds.
     * @param[in] else_lambda The lambda method to execute if the condition does not hold.
     *
     * @return Shared pointer to the lambda_rule_t instance parsed from the text.
     */
    std::shared_ptr<lambda_rule_t> parse(lexer_t &lexer, lambda_t if_lambda, lambda_t else_lambda);
};

} // End namespace wf,

#endif // LAMBDA_RULE_PARSER_HPP
