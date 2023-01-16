#ifndef CONDITION_PARSER_HPP
#define CONDITION_PARSER_HPP

#include "wayfire/lexer/symbol.hpp"
#include <memory>

namespace wf
{

class condition_t;
class lexer_t;

/**
 * @brief The condition_parser_t class uses the lexer_t instance to parse a condition rult from a text.
 *
 * This is a complicated class. It is a recursive descent parser that is capable of parsing boolean expressions
 * with correct semantics. It supports the use of brackets to control evaluation order. The supported operators
 * are, in order of precedence, NOT, AND and OR.
 *
 * The decision was made to not support the XOR operator because it needlessly made the condition parser more
 * complex. Boolean algebra teaches us that XOR can be expressed using only the NOT, AND and OR operators.
 *
 * a XOR b = (a OR b) AND (NOT(a AND B))
 * a XOR b = (a AND (NOT b)) OR ((NOT a) AND B)
 */
class condition_parser_t {
public:
    /**
     * @brief condition_parser_t Constructor.
     */
    condition_parser_t();

    /**
     * @brief parse Parses a condition from the text in the lexer instance.
     *
     * throws std::runtime_error in case of a parser error.
     *
     * @param[in] lexer The lexer instance to use to parse a condition from.
     *
     * @return Shared pointer to the parsed condition instance.
     */
    std::shared_ptr<condition_t> parse(lexer_t &lexer);

private:
    /**
     * @brief _expression Parses an expression for the condition.
     *
     * @param[in] lexer The lexer_t instance to parse from.
     */
    void _expression(lexer_t &lexer);

    /**
     * @brief _expression Parses a term for the condition.
     *
     * @param[in] lexer The lexer_t instance to parse from.
     */
    void _term(lexer_t &lexer);

    /**
     * @brief _expression Parses a factor for the condition.
     *
     * @param[in] lexer The lexer_t instance to parse from.
     */
    void _factor(lexer_t &lexer);

    /**
     * @brief _root Shared pointer to the (current) root of the parsed condition.
     *
     * The root will point to different items during the parsing of the logical expression.
     */
    std::shared_ptr<condition_t> _root;

    /**
     * @brief _symbol Current symbol under consideration from the lexer.
     */
    symbol_t _symbol;
};

} // End namespace wf.

#endif // CONDITION_PARSER_HPP
