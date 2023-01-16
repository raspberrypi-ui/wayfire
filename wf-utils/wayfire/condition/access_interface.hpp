#ifndef ACCESS_INTERFACE_HPP
#define ACCESS_INTERFACE_HPP

#include "wayfire/variant.hpp"
#include <string>

namespace wf
{

/**
 * @brief The access_interface_t class describes an interface for a property accessor that can be used by the condition
 *        system.
 *
 * The intent of this interface is that it would be subclassed for an object type that can then be used with the
 * condition system.
 *
 * @attention A good implementation of this interface ensures that the object being interrogated is NOT changed.
 */
class access_interface_t
{
public:
    /**
     * @brief ~access_interface_t Destructor.
     */
    virtual ~access_interface_t() = default;

    /**
     * @brief get Retrieves the value of a named property.
     *
     * @param[in] identifier The name of the property to get the value for.
     * @param[out] error Reference to a boolean value that will receve the error state in case something goes wrong.
     *
     * @return The value of the property. May be invalid if the error reference was set to <code>true</code>.
     */
    virtual variant_t get(const std::string &identifier, bool &error) = 0;
};

} // End namespace wf.

#endif // ACCESS_INTERFACE_HPP
