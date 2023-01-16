#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <string>
#include <variant>

namespace wf
{

/**
 * @brief Alias for std::variant to reduce the amount of typing to do for the poor developers.
 */
using variant_t = std::variant<int, char, bool, float, double, std::string>;

/**
 * @brief The variant_type_t enum Enumerates the types of data that can be in a variant_t.
 */
enum class variant_type_t
{
    /**
     * @brief Variant contains integer data.
     */
    INT,
    /**
     * @brief Variant contains character data.
     */
    CHAR,
    /**
     * @brief Variant contains boolean data.
     */
    BOOL,
    /**
     * @brief Variant contains floating point data.
     */
    FLOAT,
    /**
     * @brief Variant contains double-precision floating point data.
     */
    DOUBLE,
    /**
     * @brief Variant contains string data.
     */
    STRING,
};

/**
 * @brief is_int Checks if the variant contains integer data.
 *
 * @param[in] v The variant to check.
 *
 * @return <code>True</code> if the variant contains integer data, <code>false</code> if not.
 */
bool is_int(const variant_t &v);

/**
 * @brief is_int Checks if the variant contains character data.
 *
 * @param[in] v The variant to check.
 *
 * @return <code>True</code> if the variant contains character data, <code>false</code> if not.
 */
bool is_char(const variant_t &v);

/**
 * @brief is_int Checks if the variant contains boolean data.
 *
 * @param[in] v The variant to check.
 *
 * @return <code>True</code> if the variant contains boolean data, <code>false</code> if not.
 */
bool is_bool(const variant_t &v);

/**
 * @brief is_int Checks if the variant contains floating point data.
 *
 * @param[in] v The variant to check.
 *
 * @return <code>True</code> if the variant contains floating point data, <code>false</code> if not.
 */
bool is_float(const variant_t &v);

/**
 * @brief is_int Checks if the variant contains double precision floating-point data.
 *
 * @param[in] v The variant to check.
 *
 * @return <code>True</code> if the variant contains double precision floating-point data, <code>false</code> if not.
 */
bool is_double(const variant_t &v);

/**
 * @brief is_int Checks if the variant contains string data.
 *
 * @param[in] v The variant to check.
 *
 * @return <code>True</code> if the variant contains string data, <code>false</code> if not.
 */
bool is_string(const variant_t &v);

/**
 * @brief type Gets the type of data contained in the supplied variant.
 *
 * @param[in] v The variant to check.
 *
 * @return The data type.
 */
variant_type_t type(const variant_t v);

/**
 * @brief get_int Extracts integer data from the given variant.
 *
 * @param[in] v The variant.
 *
 * @return The extracted data.
 */
int get_int(const variant_t &v);

/**
 * @brief get_int Extracts character data from the given variant.
 *
 * @param[in] v The variant.
 *
 * @return The extracted data.
 */
char get_char(const variant_t &v);

/**
 * @brief get_int Extracts boolean data from the given variant.
 *
 * @param[in] v The variant.
 *
 * @return The extracted data.
 */
bool get_bool(const variant_t &v);

/**
 * @brief get_int Extracts floating-point data from the given variant.
 *
 * @param[in] v The variant.
 *
 * @return The extracted data.
 */
float get_float(const variant_t &v);

/**
 * @brief get_int Extracts double precision floating-point data from the given variant.
 *
 * @param[in] v The variant.
 *
 * @return The extracted data.
 */
double get_double(const variant_t &v);

/**
 * @brief get_int Extracts string data from the given variant.
 *
 * @param[in] v The variant.
 *
 * @return The extracted data.
 */
std::string get_string(const variant_t &v);

/**
 * @brief to_string Cnvenience method to create a string representation of the supplied variant.
 *
 * This is useful in text based logging or debug facilities.
 *
 * @param[in] v The variant to get a string representation for.
 *
 * @return The string representation.
 */
std::string to_string(const variant_t &v);

} // End namespace wf.

#endif // VARIANT_HPP
