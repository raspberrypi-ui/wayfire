#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include "wayfire/variant.hpp"
#include <set>
#include <string>
#include <string_view>

namespace wf
{

/**
 * @brief Set of all the signals recognized by the lexer.
 */
static const std::set<std::string_view> SIGNALS = {"created", "maximized", "unmaximized", "minimized", "fullscreened"};

/**
 * @brief Set of all the keywords recognized by the lexer.
 */
static const std::set<std::string_view> KEYWORDS = {"is", "equals", "contains", "if", "else", "then", "on", "all", "none"};

/**
 * @brief Set of all the operators recognized by the lexer.
 */
static const std::set<std::string_view> OPERATORS = {"&", "|", "!"};

/**
 * @brief Set of all the structural characters recognized by the lexer.
 */
static const std::set<std::string_view> STRUCTURES = {"(", ")"};

/**
 * @brief The symbol_t class holds information on a symbol parsed by the lexer.
 *
 * A parser or other user of the lexer can use this to do things with what was in the supplied parse text.
 */
class symbol_t
{
public:
    /**
     * @brief The type_t enum lists the different symbol types that the lexer_t can recognize.
     */
    enum class type_t
    {
        /**
         * @brief Symbol type returned when the lexer is at the end of the parse text.
         */
        END,

        /**
         * @brief Symbol type returned if an identifier was parsed.
         *
         * An identifier is a name of a property or variable. It follows the C/C++ naming rules for variables.
         */
        IDENTIFIER,

        /**
         * @brief Symbol type returned if a literal value was parsed.
         *
         * Can be String, int, char, bool, float or double.
         */
        LITERAL,

        /**
         * @brief Symbol type returned if an operator was found.
         *
         * Operators are in the OPERATORS set.
         */
        OPERATOR,

        /**
         * @brief Symbol type returned if a keyword was found.
         *
         * Keywords are in the KEYWORDS set.
         */
        KEYWORD,

        /**
         * @brief Symbol type returned if a structural character was found.
         *
         * Structural characters are in the STRUCTURES set.
         */
        STRUCTURAL,

        /**
         * @brief Symbol type returned if a signal was found.
         *
         * Signals are in the SIGNALS set.
         */
        SIGNAL,
    };

    /**
     * @brief symbol_t Constructor.
     *
     * Initializes a new symbol as END symbol and an empty string as value.
     */
    symbol_t();

    /**
     * @brief symbol_t Constructor.
     *
     * Initializes a new symbol with the given type and an empty string as value.
     *
     * @param[in] t Type of the symbol to create.
     */
    symbol_t(type_t t);

    /**
     * @brief symbol_t Constructor.
     *
     * Initializes a new symbol with the given value and LITERAL type.
     *
     * @param[in] v The value of the LITERAL typed symbol.
     */
    symbol_t(const variant_t &v);

    /**
     * @brief type Type of the symbol.
     */
    type_t type;

    /**
     * @brief value Value of the symbol.
     */
    variant_t value;
};

/**
 * @brief to_string Convenience method to be able to output a symbol to a text base debug facility or log.
 *
 * @param[in] s The symbol to convert.
 *
 * @return The string representation of the symbol.
 */
std::string to_string(const symbol_t &s);

} // End namespace wf.

#endif // SYMBOL_HPP
