#include "wayfire/action/action.hpp"
#include "wayfire/action/action_interface.hpp"
#include "wayfire/variant.hpp"

namespace wf
{

action_t::action_t(const std::string &name, const std::vector<variant_t> &args) : _name(name), _args(args)
{
}

bool action_t::execute(action_interface_t &interface)
{
    return interface.execute(_name, _args);
}

std::string action_t::to_string() const
{
    std::string out = "action: [name: ";
    out.append(_name).append(" args: [");
    for (const auto &arg : _args)
    {
        out.append(wf::to_string(arg)).append(", ");
    }
    out.append("]]");
    return out;
}

} // End namespace wf.
