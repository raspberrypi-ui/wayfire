#ifndef TEST_CONDITION_HPP
#define TEST_CONDITION_HPP

#include "wayfire/condition/condition.hpp"
#include "wayfire/variant.hpp"
#include <string>

namespace wf
{

class access_interface_t;

/**
 * @brief The test_condition_t class is the acstract superclass in the test condition hierarchy. A test_condition_t will compare a property
 *        value agains a set value and return <code>true</code> if the value of the named property compares favorably to the supplied value.
 */
class test_condition_t : public condition_t
{
public:
    /**
     * @brief test_condition_t Constructor.
     *
     * @param[in] identifier of the property to check against.
     * @param[in] value to chech the property against.
     */
    test_condition_t(const std::string &identifier, const variant_t &value);

    /**
     * @brief ~test_condition_t Destructor.
     */
    virtual ~test_condition_t() override;

    // Inherits docs.
    virtual bool evaluate(access_interface_t &interface, bool &error) override = 0;

    // Inherits docs.
    virtual std::string to_string() const override = 0;
protected:
    /**
     * @brief _identifier of the property to check in the evaluate() method.
     */
    std::string _identifier;

    /**
     * @brief _value to check the named property against.
     */
    variant_t _value;
};

/**
 * @brief The true_condition_t class is a placeholder condition that will always return <code>true</code>. This is useful to be able to
 *        define catch-all rules.
 */
class true_condition_t : public condition_t
{
public:
    /**
     * @brief ~true_condition_t Destructor.
     */
    virtual ~true_condition_t() override;

    // Inherits docs.
    virtual bool evaluate(access_interface_t &interface, bool &error) override;

    // Inherits docs.
    virtual std::string to_string() const override;
};

/**
 * @brief The false_condition_t class is a placeholder condition that will always return <code>false</code>. This is useful to be able to
 *        define catch-none rules.
 */
class false_condition_t : public condition_t
{
public:
    /**
     * @brief ~false_condition_t Destructor.
     */
    virtual ~false_condition_t() override;

    // Inherits docs.
    virtual bool evaluate(access_interface_t &interface, bool &error) override;

    // Inherits docs.
    virtual std::string to_string() const override;
};

/**
 * @brief The equals_condition_t class will test if the value of the property named by _identifier is the same as the supplied _value.
 */
class equals_condition_t : public test_condition_t
{
public:
    // Inherits docs.
    equals_condition_t(const std::string &identifier, const variant_t &value);

    /**
     * @brief ~equals_condition_t Destructor.
     */
    virtual ~equals_condition_t() override;

    // Inherits docs.
    virtual bool evaluate(access_interface_t &interface, bool &error) override;

    // Inherits docs.
    virtual std::string to_string() const override;
};

/**
 * @brief The contains_condition_t class will test if the value of the property named by _identifier contains the supplied _value.
 *
 * This could be useful to check for a substring in a string, or for a member in an array of values.
 */
class contains_condition_t : public test_condition_t
{
public:
    // Inherits docs.
    contains_condition_t(const std::string &identifier, const variant_t &value);

    /**
     * @brief ~contains_condition_t Destructor.
     */
    virtual ~contains_condition_t() override;

    // Inherits docs.
    virtual bool evaluate(access_interface_t &interface, bool &error) override;

    // Inherits docs.
    virtual std::string to_string() const override;
};

} // End namespace wf.

#endif // TEST_CONDITION_HPP
