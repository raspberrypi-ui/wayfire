#ifndef CONDITION_HPP
#define CONDITION_HPP

#include <string>

namespace wf
{

class access_interface_t;

/**
 * @brief The condition_t class is the superclass in the condition hierarchy.
 */
class condition_t
{
public:
    /**
     * @brief condition_t Constructor.
     */
    condition_t();

    /**
     * @brief ~condition_t Destructor.
     */
    virtual ~condition_t();

    /**
     * @brief evaluate uses the interface to evaluate the condition and return the result.
     *
     * @param[in] interface The interface to use to check if the condition is met.
     * @param[out] error Pointer to a boolean value that will be set to <code>true</code> if an error occurred.
     *
     * @return <code>True</code> if the condition is satisfied, <code>false</code> if not.
     */
    virtual bool evaluate(access_interface_t &interface, bool &error) = 0;

    /**
     * @brief to_string Turns the condtion into a printable string. Useful for debugging.
     *
     * @return String representation of the condition.
     */
    virtual std::string to_string() const = 0;
};

} // End namespace wf.

#endif // CONDITION_HPP
