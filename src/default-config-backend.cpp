#include <vector>
#include "wayfire/debug.hpp"
#include <string>
#include <cstring>
#include <wayfire/config/file.hpp>
#include <wayfire/config-backend.hpp>
#include <wayfire/plugin.hpp>
#include <wayfire/core.hpp>

#include <sys/inotify.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INOT_BUF_SIZE (sizeof(inotify_event) + NAME_MAX + 1)

static std::string config_dir, config_file;
wf::config::config_manager_t *cfg_manager;

static int wd_cfg_file;

static void readd_watch(int fd)
{
    inotify_add_watch(fd, config_dir.c_str(), IN_CREATE);
    wd_cfg_file = inotify_add_watch(fd, config_file.c_str(), IN_MODIFY);
}

static void reload_config(int fd)
{
    wf::config::load_configuration_options_from_file(*cfg_manager, config_file);
    readd_watch(fd);
}

static int handle_config_updated(int fd, uint32_t mask, void *data)
{
    if ((mask & WL_EVENT_READABLE) == 0)
    {
        return 0;
    }

    char buf[INOT_BUF_SIZE] __attribute__((aligned(alignof(inotify_event))));

    bool should_reload = false;
    inotify_event *event;

    // Reading from the inotify FD is guaranteed to not read partial events.
    // From inotify(7):
    // Each successful read(2) returns a buffer containing
    // one or more [..] structures
    auto len = read(fd, buf, INOT_BUF_SIZE);
    if (len < 0)
    {
        return 0;
    }

    const auto last_slash = config_file.find_last_of('/');
    const auto cfg_file_basename = (last_slash == std::string::npos) ?
        config_file : config_file.substr(last_slash);

    for (char *ptr = buf;
         ptr < (buf + len);
         ptr += sizeof(inotify_event) + event->len)
    {
        event = reinterpret_cast<inotify_event*>(ptr);
        // We reload in two main cases:
        //
        // - Config file itself was modified
        // - Config file was created inside parent directory
        should_reload |=
            (event->wd == wd_cfg_file) || (cfg_file_basename == event->name);
    }

    if (should_reload)
    {
        LOGD("Reloading configuration file");

        reload_config(fd);
        wf::get_core().emit_signal("reload-config", nullptr);
    } else
    {
        readd_watch(fd);
    }

    return 0;
}

static const char *CONFIG_FILE_ENV = "WAYFIRE_CONFIG_FILE";

namespace wf
{
class dynamic_ini_config_t : public wf::config_backend_t
{
  public:
    void init(wl_display *display, config::config_manager_t& config,
        const std::string& cfg_file) override
    {
        cfg_manager = &config;

        config_file = choose_cfg_file(cfg_file);
        LOGI("Using config file: ", config_file.c_str());
        setenv(CONFIG_FILE_ENV, config_file.c_str(), 1);

        std::string defs, tmpl;
        if (config_file.find ("greeter") != std::string::npos)
        {
            defs = SYSCONFDIR "/wayfire/greeter.ini";
            // emergency fallback in case the user greeter file has been deleted
            if (access (config_file.c_str (), F_OK))
                config_file = SYSCONFDIR "/wayfire/gtemplate.ini";
        }
        else
        {
            defs = SYSCONFDIR "/wayfire/defaults.ini";
            tmpl = SYSCONFDIR "/wayfire/template.ini";

            /* check if the config file exists - if not, copy the defaults to it */
            if (access (config_file.c_str (), F_OK) && !access (tmpl.c_str (), F_OK))
            {
                int fs = open (tmpl.c_str (), O_RDONLY);
                if (fs >= 0)
                {
                    struct stat stat_buf;
                    fstat (fs, &stat_buf);
                    int fd = open (config_file.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if (fd >= 0)
                    {
                        sendfile (fd, fs, NULL, stat_buf.st_size);
                        close (fd);
                    }
                    close (fs);
                }
            }
        }

        config = wf::config::build_configuration(
            get_xml_dirs(), defs, config_file);

        int inotify_fd = inotify_init1(IN_CLOEXEC);
        reload_config(inotify_fd);

        wl_event_loop_add_fd(wl_display_get_event_loop(display),
            inotify_fd, WL_EVENT_READABLE, handle_config_updated, NULL);
    }

    std::string choose_cfg_file(const std::string& cmdline_cfg_file)
    {
        std::string env_cfg_file = nonull(getenv(CONFIG_FILE_ENV));
        if (!cmdline_cfg_file.empty())
        {
            if ((env_cfg_file != nonull(NULL)) &&
                (cmdline_cfg_file != env_cfg_file))
            {
                LOGW("Wayfire config file specified in the environment is ",
                    "overridden by the command line arguments!");
            }

            return cmdline_cfg_file;
        }

        if (env_cfg_file != nonull(NULL))
        {
            return env_cfg_file;
        }

        // Fallback, default config file
        config_dir = nonull(getenv("XDG_CONFIG_HOME"));
        if (!config_dir.compare("nil"))
        {
            config_dir = std::string(nonull(getenv("HOME"))) + "/.config";
        }

        return config_dir + "/wayfire.ini";
    }
};
}

DECLARE_WAYFIRE_CONFIG_BACKEND(wf::dynamic_ini_config_t);
