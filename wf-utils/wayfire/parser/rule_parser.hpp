#ifndef RULE_PARSER_HPP
#define RULE_PARSER_HPP

#include <memory>
#include <string>

namespace wf
{

class lexer_t;
class rule_t;

/**
 * @brief The rule_parser_t class will parse a rule_t from a lexer_t instance.
 */
class rule_parser_t {
public:
    /**
     * @brief parse Convenience method. Takes the text and makes a lexer_t instance which is then used to parse the rule_t.
     *
     * This method is convenient, but not recommended if you have multiple rules to parse. In the multiple rules case,
     * it is smarter to make 1 lexer_t instance outside and reset it to a new text, this reusing the lexer_t instance.
     * Reuse is good for the environment!
     *
     * This method will throw std::runtime_error in case the text is malformed.
     *
     * @param[in] text The text to parse a rule_t from.
     *
     * @return The parsed rule.
     */
    std::shared_ptr<rule_t> parse(const std::string &text);

    /**
     * @brief parse Takes the lexer_t instance and parses a rule_t from it.
     *
     * This method will throw std::runtime_error incase the lexer_t instance has malformed text in it.
     *
     * @param[in] lexer The lexer_t instance to parse the rule_t from.
     *
     * @return Shared pointer to the rule_t instance parsed from the text.
     */
    std::shared_ptr<rule_t> parse(lexer_t &lexer);
};

} // End namespace wf.

#endif // RULE_PARSER_HPP
