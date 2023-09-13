#include <wayfire/plugin.hpp>
#include <wayfire/opengl.hpp>
#include <wayfire/pixman.hpp>
#include <wayfire/core.hpp>
#include <wayfire/view.hpp>
#include <wayfire/output.hpp>
#include <wayfire/render-manager.hpp>
#include <wayfire/workspace-manager.hpp>
#include <wayfire/signal-definitions.hpp>
#include <wayfire/nonstd/reverse.hpp>
#include <wayfire/util/duration.hpp>
#include <wayfire/view-transform.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <set>

#include "../main.hpp"

constexpr const char *pixswitcher_transformer = "pixswitcher-2d";

using namespace wf::animation;

class PixSwitcherPaintAttribs
{
  public:
    PixSwitcherPaintAttribs(const duration_t& duration) :
         scale_x(duration, 1, 1), scale_y(duration, 1, 1),
         translation_x(duration, 0, 0), translation_y(duration, 0, 0)
    {}

    timed_transition_t scale_x, scale_y;
    timed_transition_t translation_x, translation_y;
};

enum PixSwitcherDirection
{
    PIXSWITCHER_DIRECTION_FORWARD = 1,
    PIXSWITCHER_DIRECTION_BACKWARD = -1,
};

struct PixSwitcherView
{
    wayfire_view view;
    PixSwitcherPaintAttribs attribs;

    int index;
    PixSwitcherView(duration_t& duration) : attribs(duration)
    {}
};

class PixSwitcher : public wf::plugin_interface_t
{
    wf::option_wrapper_t<int> grid_columns{"pixswitcher/grid_columns"};
    wf::option_wrapper_t<double> grid_margin{"pixswitcher/grid_margin"};
    wf::option_wrapper_t<int> speed{"pixswitcher/speed"};
    wf::option_wrapper_t<double> thumbnail_selected_scale{"pixswitcher/thumbnail_selected_scale"};
    wf::option_wrapper_t<double> thumbnail_unselected_scale{"pixswitcher/thumbnail_unselected_scale"};

    duration_t duration{speed};

    std::vector<PixSwitcherView> views;

    // the modifiers which were used to activate switcher
    uint32_t activating_modifiers = 0;
    // if the plugin is active or not
    bool active = false;
    // the number of views
    int32_t count = 0;
    // the selected view
    int32_t selected = 0;

  public:

    void init() override
    {
        grab_interface->name = "pixswitcher";
        grab_interface->capabilities = wf::CAPABILITY_MANAGE_COMPOSITOR;

        output->add_key(
            wf::option_wrapper_t<wf::keybinding_t>{"pixswitcher/next_view"},
            &next_view_binding);
         output->add_key(wf::option_wrapper_t<wf::keybinding_t>{"pixswitcher/prev_view"},
             &prev_view_binding);
         output->connect_signal("view-mapped", &view_added);
         output->connect_signal("view-detached", &view_removed);

         grab_interface->callbacks.keyboard.mod = [=] (uint32_t mod, uint32_t state)
         {
            if ((state == WLR_KEY_RELEASED) && (mod & activating_modifiers))
            {
                handle_done();
            }
         };

         grab_interface->callbacks.cancel = [=] () { deinit_switcher(); };
    }

    void fini() override
    {
         if (output->is_plugin_active(grab_interface->name))
         {
            deinit_switcher();
         }

         output->rem_binding(&next_view_binding);
         output->rem_binding(&prev_view_binding);
         output->disconnect_signal("view-mapped", &view_added);
         output->disconnect_signal("view-detached", &view_removed);
    }     

    /* keybinding callbacks for cycling between views */
    wf::key_callback next_view_binding = [=] (auto)
    {
        return handle_switch_request(PIXSWITCHER_DIRECTION_FORWARD);
    };

    wf::key_callback prev_view_binding = [=] (auto)
    {
         return handle_switch_request(PIXSWITCHER_DIRECTION_BACKWARD);
    };

    /* output signal callback when a view gets added */
    wf::signal_callback_t view_added = [=] (wf::signal_data_t *data)
    {
        /* if we are not running, then nothing to do */
        if (!output->is_plugin_active(grab_interface->name))
        {
            return;
        }

        bool need_action = false;
        wayfire_view view = get_signaled_view(data);

        for (auto& sv : get_workspace_views())
        {
            need_action |= (sv == view);
        }

        if (!need_action) return;

        /* Push the new view to the beginning */
        views.push_back(create_view(view));
        count++;
        selected++;
        arrange();
    };

    /* output signal callback when a view gets removed */
    wf::signal_callback_t view_removed = [=] (wf::signal_data_t *data)
    {
        /* if we are not running, then nothing to do */
        if (!output->is_plugin_active(grab_interface->name))
        {
            return;
        }

        bool need_action = false;
        wayfire_view view = get_signaled_view(data);

        for (auto& sv : views)
        {
            need_action |= (sv.view == view);
        }

        /* don't do anything if we're not using this view */
        if (!need_action) return;

        /* if the removed view was the last one and selected, select
           the previous view */
        if (selected == (int)views.size() - 1)
        {
            selected--;
        }

        if (active)
        {
            arrange();
        }

        if (!views.size())
        {
            active = false;
            deinit_switcher();
        }
    };


    wf::effect_hook_t damage = [=] ()
    {
        output->render->damage_whole();
    };

    PixSwitcherView create_view(wayfire_view view)
    {
        if (!view->get_transformer(pixswitcher_transformer))
        {
            view->add_transformer(std::make_unique<wf::view_2D>(view),
                                  pixswitcher_transformer);
        }

        PixSwitcherView pv{duration};
        pv.view = view;
        pv.index = count++;

        return pv;
    }

    std::vector<wayfire_view> get_workspace_views() const
    {
        auto all = output->workspace->get_views_on_workspace(
            output->workspace->get_current_workspace(),
            wf::WM_LAYERS | wf::LAYER_MINIMIZED);

        decltype(all) mapped_views;
        for (auto view : all)
        {
            if (view->is_mapped())
            {
                mapped_views.push_back(view);
            }
        }

        return mapped_views;
    }

    void arrange()
    {
        views.clear();
        count = 0;

        duration.start();

        auto wviews = get_workspace_views();
        for (auto v : wviews)
        {
            views.push_back(create_view(v));
        }

        for (int i = 0; i < (int)views.size(); i++)
        {
            arrange_view(views[i]);
        }
    }

    void dearrange()
    {
        for (auto& pv : views)
        {
            pv.attribs.translation_x.restart_with_end(0);
            pv.attribs.translation_y.restart_with_end(0);

            pv.attribs.scale_x.restart_with_end(1.0);
            pv.attribs.scale_y.restart_with_end(1.0);
        }

        duration.start();
        active = false;

        if (views.size())
        {
            output->focus_view(views[selected].view, true);
        }
    }

    std::vector<wayfire_view> get_bg_views() const
    {
        auto ws = output->workspace->get_current_workspace();
        return output->workspace->get_views_on_workspace(ws, wf::BELOW_LAYERS);
    }

    std::vector<wayfire_view> get_overlay_views() const
    { 
        return output->workspace->get_views_on_workspace(
            output->workspace->get_current_workspace(),
            wf::ABOVE_LAYERS);
    }

    bool handle_switch_request(int dir)
    {
        if (get_workspace_views().empty())
        {
            return false;
        }

        if (!output->is_plugin_active(grab_interface->name))
        {
            if (!init_switcher())
            {
                return false;
            }
        }

        if (!active)
        {
            active = true;
            selected = 0;

            auto grab = grab_interface->grab();
            assert(grab);

            arrange();
            focus_next(dir);
            activating_modifiers = wf::get_core().get_keyboard_modifiers();
        } else
        {
            focus_next(dir);
        }

        return true;
    }

    void handle_done()
    {
        dearrange();
        grab_interface->ungrab();
    }

    bool init_switcher()
    {
        if (!output->activate_plugin(grab_interface))
        {
            return false;
        }

        output->render->add_effect(&damage, wf::OUTPUT_EFFECT_PRE);
        output->render->set_renderer(pixswitcher_renderer);
        if (!runtime_config.use_pixman)
        {
            output->render->set_redraw_always();
        }

        return true;
    }

    void deinit_switcher()
    {
        output->deactivate_plugin(grab_interface);

        output->render->rem_effect(&damage);
        output->render->set_renderer(nullptr);
        if (!runtime_config.use_pixman)
        {
            output->render->set_redraw_always(false);
        }

        for (auto& view : output->workspace->get_views_in_layer(wf::ALL_LAYERS))
        {
            view->pop_transformer(pixswitcher_transformer);
        }

        views.clear();
        count = 0;

        wf::stack_order_changed_signal data;
        data.output = output;
        wf::get_core().emit_signal("output-stack-order-changed", &data);
    }

    /* gets the number of colums of the grid */
    int get_grid_width() const
    {
        return std::min((int)views.size(), (int)grid_columns);
    }

    /* gets the number of rows of the grid */
    int get_grid_height() const
    {
        return 1 + (views.size() - 1) / grid_columns;
    }

    /* gets the width in pixels of a cell */
    int get_grid_cell_width() const
    {
        auto fullw = output->get_relative_geometry().width;

        return (fullw - (2 * grid_margin * fullw)) / get_grid_width();
    }

    /* gets the height in pixels of a cell */
    int get_grid_cell_height() const
    {
        auto fullh = output->get_relative_geometry().height;

        return (fullh - (2 * grid_margin * fullh)) / get_grid_height();
    }

    /* gets the scale required to apply to box to fit in the cell */
    float get_scale_factor(const wf::geometry_t& box) const
    {
       return std::min(
            get_grid_cell_width() / (float)box.width,
            get_grid_cell_height() / (float)box.height);
    }

    /* gets the X position in the screen to place the box in the
       proper grid cell, based on the index */
    float get_grid_cell_x_offset(struct wlr_box bbox, int index) const
    {
        auto margin_offset = grid_margin * output->get_relative_geometry().width;
        auto cellw = get_grid_cell_width();

        auto offset = margin_offset + (cellw/2) + cellw * (index % get_grid_width());

        return offset - bbox.x - (bbox.width/2);
    }

    /* gets the Y position in the screen to place the box in the
       proper grid cell, based on the index */
    float get_grid_cell_y_offset(struct wlr_box bbox, int index) const
    {
        auto margin_offset = grid_margin * output->get_relative_geometry().height;
        auto cellh = get_grid_cell_height();

        auto offset = margin_offset + (cellh/2) + cellh * (index / get_grid_width());

        return offset - bbox.y - (bbox.height/2);
    }

    /* Highlight/unhighlight the view if it is selected/unselected */
    void highlight_view(PixSwitcherView& pv)
    {
        auto bbox = pv.view->get_bounding_box(pixswitcher_transformer);
        float scale = get_scale_factor(bbox);

        pv.attribs.scale_x.restart_with_end(
            scale * (pv.index == selected ? thumbnail_selected_scale : thumbnail_unselected_scale));
        pv.attribs.scale_y.restart_with_end(
            scale * (pv.index == selected ? thumbnail_selected_scale : thumbnail_unselected_scale));
    }

    /* Moves the view to the proper cell in the switcher grid */
    void arrange_view(PixSwitcherView& pv)
    {
        auto bbox = pv.view->get_bounding_box(pixswitcher_transformer);

        pv.attribs.translation_x.restart_with_end(get_grid_cell_x_offset(bbox, pv.index));
        pv.attribs.translation_y.restart_with_end(get_grid_cell_y_offset(bbox, pv.index));

        highlight_view(pv);
    }

    /* Moves the focus to the next/prev view, hightlighting the new one */
    void focus_next(int dir)
    {
        int size = views.size();
        int previous = selected;
        selected = (selected + dir) % size;
        if (selected < 0)
        {
            selected = size - selected;
        }
        highlight_view(views[previous]);
        highlight_view(views[selected]);
    }

    void render_view(PixSwitcherView& pv, const wf::framebuffer_t& fb)
    {
        auto transform = dynamic_cast<wf::view_2D*>(
            pv.view->get_transformer(pixswitcher_transformer).get());
        assert(transform);

        transform->translation_x = (double)pv.attribs.translation_x;
        transform->translation_y = (double)pv.attribs.translation_y;

        transform->scale_x = (double)pv.attribs.scale_x;
        transform->scale_y = (double)pv.attribs.scale_y;

        pv.view->render_transformed(fb, fb.geometry);
    }

    wf::render_hook_t pixswitcher_renderer = [=] (const wf::framebuffer_t& fb)
    {
        if (!runtime_config.use_pixman)
        {
            OpenGL::render_begin(fb);
            OpenGL::clear({0, 0, 0, 1});
            OpenGL::render_end();
        } else
        {
            Pixman::render_begin(fb);
            Pixman::clear({0, 0, 0, 1});
            Pixman::render_end();
        }

        for (auto view : get_bg_views())
        {
            view->render_transformed(fb, fb.geometry);
        }

        for (auto& view : wf::reverse(views))
        {
            if (view.index != selected)
            {
                render_view(view, fb);
            }
        }

        render_view(views[selected], fb);

        for (auto& view : get_overlay_views())
        {
            view->render_transformed(fb, fb.geometry);
        }

        if (!duration.running())
        {
            if (!active)
            {
                deinit_switcher();
            }
        }
    };
};

DECLARE_WAYFIRE_PLUGIN(PixSwitcher);
