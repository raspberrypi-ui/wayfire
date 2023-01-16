#ifndef LEXER_HPP
#define LEXER_HPP

#include "wayfire/lexer/symbol.hpp"
#include "wayfire/variant.hpp"
#include <cstddef>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace wf
{

/**
 * @brief List of delimiters. The lexer will look for these to determine beginning/end of symbols.
 */
static const std::set<std::string_view> DELIMITERS = {" ", "\'", "\""};

/**
 * @brief The lexer_t class takes the given text and parses this in symbols.
 *
 * @note Lexer is short for Lexical Parser.
 *
 * The lexer is a smart tokenizer that will present the given text as a set of symbols that can be
 * consumed by a structural parser to do exciting things with.
 *
 * It is reversible to the beginning to allow a parser to peek at values ahead of the parse position.
 * This is helpful when working with a hierarchy of parsers. The inner parser may recognize a symbol
 * that is not meant for it, reverse the lexer and hand it back to the outer parser.
 *
 * To allow fast parsing when doing multipass parsing, all parsed symbols are added to a std::vector.
 * Reversing the lexer will just go back one value in the std::vector of symbols. This means that the
 * lexer will only do the parsing once. Even if it is reversed all the way back to the beginning,
 * followed by new calls to parse_symbol. This is an optimization, making the parser a bit more memory
 * heavy, but saving time on the string manipulations on the second pass.
 *
 * The lexer supports a set of methods that can be used to give accurate error reporting. It is
 * possible to obtain the lexer text and the position of the current symbol. In case the parser throws
 * due to an error in the supplied string, the caller can figure out exactly where in the string the
 * mistake occurred.
 */
class lexer_t
{
public:
    /**
     * @brief lexer_t Constructor.
     */
    lexer_t();

    /**
     * @brief lexer_t Constructor.
     *
     * @param[in] text The text for the lexer to tokenize.
     */
    lexer_t(const std::string &text);

    /**
     * @brief reset Resets the lexer to the start position.
     *
     * Calling reset will also clear the std::vector. If you want to bring back the lexer to the
     * beginning and force it to re-parse the string, this is the method you want.
     */
    void reset();

    /**
     * @brief reset Resets the lexer and introduces a new text to parse.
     *
     * @param[in] text The text for the lexer to tokenize.
     */
    void reset(const std::string &text);

    /**
     * @brief parse_symbol Advances 1 symbol through the text and returns it.
     *
     * If the lexer has been reversed, this may return from cache rather then parse in the string.
     *
     * This method throws std::runtime_error in case a symbol cannot be parsed due to malformed text.
     *
     * @return The parser symbol.
     */
    symbol_t parse_symbol();

    /**
     * @brief reverse Reverses 1 symbol in the lexer text.
     *
     * If the lexer is reversed to the beginning (before first symbol), calling reverse again has
     * no effect.
     */
    void reverse();

    /**
     * @brief text Gets a reference to the text the lexer is currently parsing.
     *
     * Usefull for finding out what is wrong with the parse string.
     *
     * @return The current lexer text.
     */
    const std::string &text() const;

    /**
     * @brief current_symbol_position Gets the character position of the first character of the
     *                                current symbol.
     *
     * @return Character position in the text of the lexer to the beginning of the current symbol.
     */
    std::size_t current_symbol_position() const;

private:
    /**
     * @brief _parse_literal Parses a literal from the current parse position.
     *
     * This method throws std::runtime_error in case a literal cannot be parsed due to malformed text.
     *
     * @return The parsed literal.
     */
    variant_t _parse_literal();

    /**
     * @brief _parse_encapsulated_literal Parses a literal that is encapsulated by the given start
     *                                    and end boundary character.
     *
     * e.g. A string literal would be bounded by " at beginning and " at the end.
     *
     * e.g. (bis) an xml tag would be bounded by < at beginning and > at the end. This is the reason
     * why there is a start and end boundary.
     *
     * @param[in] s_bound Start boundary character.
     * @param[in] e_bound End boundary character.
     *
     * @return The parsed literal.
     */
    variant_t _parse_encapsulated_literal(const std::string &s_bound, const std::string &e_bound);

    /**
     * @brief _parse_comment_literal Parses a comment literal.
     *
     * A comment literal starts at the # symbol and runs until the first encountered \n character.
     * @return
     */
    variant_t _parse_comment_literal();

    /**
     * @brief _size Length of the current parse text.
     */
    std::size_t _size;

    /**
     * @brief _text The current parse text.
     */
    std::string _text;

    /**
     * @brief _parse_position The current parse character position.
     *
     * This is the index of the parse position in the _text string.
     */
    std::size_t _parse_position;

    /**
     * @brief _symbol_position The character position of the first character of the current symbol.
     *
     * This is the index of the current symbol position in the _text string.
     */
    std::size_t _symbol_position;

    /**
     * @brief _reversed Counter to indicate how many times reverse has been called from the current
     *                  parse position.
     */
    std::size_t _reversed;

    /**
     * @brief _history The std::vector with the already parsed symbols.
     *
     * Each symbol that is encountered while servicing the parse_symbol() method is stored here to
     * support reverse() calls and multipass/peeking.
     */
    std::vector<std::pair<std::size_t, symbol_t>> _history;
};

} // End namespace wf.

#endif // LEXER_HPP
