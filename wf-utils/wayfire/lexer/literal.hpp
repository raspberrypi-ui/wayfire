#ifndef LITERAL_HPP
#define LITERAL_HPP

#include "wayfire/variant.hpp"
#include <string>

namespace wf
{

/**
 * @brief parse_literal Static helper method to parse a literal from a text fragment.
 *
 * @param[in] s The text fragment to convert to a literal value.
 *
 * @return The converted literal value.
 */
variant_t parse_literal(const std::string &s);

} // End namespace wf.

#endif // LITERAL_HPP
