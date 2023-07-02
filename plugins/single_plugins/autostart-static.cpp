#include <wayfire/singleton-plugin.hpp>
#include <wayfire/core.hpp>
#include <wayfire/util/log.hpp>
#include <wayfire/option-wrapper.hpp>
#include <config.h>

class wayfire_autostart_static
{
    wf::option_wrapper_t<std::string> autostart0{"autostart-static/autostart0"};
    wf::option_wrapper_t<std::string> autostart1{"autostart-static/autostart1"};
    wf::option_wrapper_t<std::string> autostart2{"autostart-static/autostart2"};
    wf::option_wrapper_t<std::string> autostart3{"autostart-static/autostart3"};
    wf::option_wrapper_t<std::string> autostart4{"autostart-static/autostart4"};

  public:
    wayfire_autostart_static()
    {
        if (!((std::string) autostart0).empty ()) wf::get_core ().run (autostart0);
        if (!((std::string) autostart1).empty ()) wf::get_core ().run (autostart1);
        if (!((std::string) autostart2).empty ()) wf::get_core ().run (autostart2);
        if (!((std::string) autostart3).empty ()) wf::get_core ().run (autostart3);
        if (!((std::string) autostart4).empty ()) wf::get_core ().run (autostart4);
    }
};

DECLARE_WAYFIRE_PLUGIN((wf::singleton_plugin_t<wayfire_autostart_static, false>));
