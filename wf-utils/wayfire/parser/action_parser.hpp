#ifndef ACTION_PARSER_HPP
#define ACTION_PARSER_HPP

#include <memory>

namespace wf
{

class action_t;
class lexer_t;

/**
 * @brief The action_parser_t class uses the lexer_t instance to parse an action from a text.
 */
class action_parser_t {
public:
    /**
     * @brief parse Parses an action from the text in the lexer instance.
     *
     * throws std::runtime_error in case of a parser error.
     *
     * @param[in] lexer The lexer instance to use to parse an action from.
     *
     * @return Shared pointer to the parsed action instance.
     */
    std::shared_ptr<action_t> parse(lexer_t &lexer);
};

} // End namespace wf.

#endif // ACTION_PARSER_HPP
