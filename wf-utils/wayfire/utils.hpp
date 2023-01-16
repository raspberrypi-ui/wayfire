#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <iterator>
#include <string>

namespace wf
{

/**
 * @brief contains Utility method to see if an element is contained in the given container.
 *
 * @tparam T_CONTAINER The template type name of the container to check for the item.
 * @tparam T_ITEM The type of the item to check.
 * @param[in] container The container to check.
 * @param[in] item The item to find in the container.
 *
 * @return <code>True</code> if the item is in the container, <code>false</code> if not.
 */
template <typename T_CONTAINER, typename T_ITEM>
bool contains(const T_CONTAINER &container, const T_ITEM &item)
{
    auto begin = std::cbegin(container);
    auto end = std::cend(container);
    return std::find(begin, end, item) != end;
}

/**
 * @brief ltrim Trims whitespace characters from the left side of the string.
 *
 * This is an in-place trim method. It will change the supplied argument.
 *
 * @param[in] s Reference to the string to trim.
 */
inline void ltrim(std::string &s)
{
    auto lambda = [] (int ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), lambda));
}

/**
 * @brief rtrim Trims whitespace characters from the right side of the string.
 *
 * This is an in-place trim method. It will change the supplied argument.
 *
 * @param[in] s Reference to the string to trim.
 */
inline void rtrim(std::string &s)
{
    auto lambda = [] (int ch) { return !std::isspace(ch); };
    s.erase(std::find_if(s.rbegin(), s.rend(), lambda).base(), s.end());
}

/**
 * @brief trim Trims whitespace characters from both sides of the string.
 *
 * This is an in-plce trim method. It will change the supplied argument.
 *
 * @param[in] s Reference to the string to trim.
 */
inline void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

} // End namespace wf.

#endif // UTILS_HPP
