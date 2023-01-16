#ifndef ACTION_INTERFACE_HPP
#define ACTION_INTERFACE_HPP

#include "wayfire/variant.hpp"
#include <string>
#include <vector>

namespace wf
{

/**
 * @brief The action_interface_t class acts as the gateway for the actions to act on an object of a given type.
 *
 * The intent of this interface is that it would be implemented with support for a single object type.
 */
class action_interface_t
{
public:
    /**
     * @brief ~action_interface_t Destructor.
     */
    virtual ~action_interface_t() = default;

    /**
     * @brief execute Executes the named action with the supplied arguments.
     *
     * @param[in] name Name of the action to execute.
     * @param[in] args Arguments for the action's execution.
     *
     * @return <code>True</code> if there was an error, <code>false</code> if not.
     */
    virtual bool execute(const std::string &name, const std::vector<variant_t> &args) = 0;
};

} // End namespace wf.

#endif // ACTION_INTERFACE_HPP
