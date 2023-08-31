#include "wayfire/core.hpp"
#include "../core/core-impl.hpp"
#include "../output/gtk-shell.hpp"
#include "view-impl.hpp"
#include "wayfire/decorator.hpp"
#include "wayfire/signal-definitions.hpp"
#include "wayfire/workspace-manager.hpp"
#include <wayfire/util/log.hpp>

#include "xdg-shell.hpp"

wf::wlr_view_t::wlr_view_t() :
    wf::wlr_surface_base_t(this), wf::view_interface_t()
{}

void wf::wlr_view_t::set_role(view_role_t new_role)
{
    view_interface_t::set_role(new_role);
    if (new_role != wf::VIEW_ROLE_TOPLEVEL)
    {
        destroy_toplevel();
    }
}

void wf::wlr_view_t::handle_app_id_changed(std::string new_app_id)
{
    this->app_id = new_app_id;
    toplevel_send_app_id();

    app_id_changed_signal data;
    data.view = self();
    emit_signal("app-id-changed", &data);
}

std::string wf::wlr_view_t::get_app_id()
{
    return this->app_id;
}

void wf::wlr_view_t::handle_title_changed(std::string new_title)
{
    this->title = new_title;
    toplevel_send_title();

    title_changed_signal data;
    data.view = self();
    emit_signal("title-changed", &data);
}

std::string wf::wlr_view_t::get_title()
{
    return this->title;
}

void wf::wlr_view_t::handle_minimize_hint(wf::surface_interface_t *relative_to,
    const wlr_box & hint)
{
    auto relative_to_view =
        dynamic_cast<view_interface_t*>(relative_to);
    if (!relative_to_view)
    {
        LOGE("Setting minimize hint to unknown surface. Wayfire currently"
             "supports only setting hints relative to views.");

        return;
    }

    if (relative_to_view->get_output() != get_output())
    {
        LOGE("Minimize hint set to surface on a different output, "
             "problems might arise");
        /* TODO: translate coordinates in case minimize hint is on another output */
    }

    auto box = relative_to_view->get_output_geometry();
    box.x    += hint.x;
    box.y    += hint.y;
    box.width = hint.width;
    box.height = hint.height;

    set_minimize_hint(box);
}

wf::region_t wf::wlr_view_t::get_transformed_opaque_region()
{
    auto& maximal_shrink_constraint =
        wf::surface_interface_t::impl::active_shrink_constraint;
    int saved_shrink_constraint = maximal_shrink_constraint;

    /* Fullscreen views take up the whole screen, so plugins can't request
     * padding for them (nothing below is visible).
     *
     * In this case, we hijack the maximal_shrink_constraint, but we must
     * restore it immediately after subtracting the opaque region */
    if (this->fullscreen)
    {
        maximal_shrink_constraint = 0;
    }

    auto region = wf::view_interface_t::get_transformed_opaque_region();
    maximal_shrink_constraint = saved_shrink_constraint;

    return region;
}

void wf::wlr_view_t::set_position(int x, int y,
    wf::geometry_t old_geometry, bool send_signal)
{
    auto obox = get_output_geometry();
    auto wm   = get_wm_geometry();

    view_geometry_changed_signal data;
    data.view = self();
    data.old_geometry = old_geometry;

    view_damage_raw(self(), last_bounding_box);
    /* obox.x - wm.x is the current difference in the output and wm geometry */
    geometry.x = x + obox.x - wm.x;
    geometry.y = y + obox.y - wm.y;

    /* Make sure that if we move the view while it is unmapped, its snapshot
     * is still valid coordinates */
    if (view_impl->offscreen_buffer.valid())
    {
        view_impl->offscreen_buffer.geometry.x += x - data.old_geometry.x;
        view_impl->offscreen_buffer.geometry.y += y - data.old_geometry.y;
    }

    damage();

    if (send_signal)
    {
        emit_signal("geometry-changed", &data);
        wf::get_core().emit_signal("view-geometry-changed", &data);
        if (get_output())
        {
            get_output()->emit_signal("view-geometry-changed", &data);
        }
    }

    last_bounding_box = get_bounding_box();
}

void wf::wlr_view_t::move(int x, int y)
{
    set_position(x, y, get_wm_geometry(), true);
}

void wf::wlr_view_t::adjust_anchored_edge(wf::dimensions_t new_size)
{
    if (view_impl->edges)
    {
        auto wm = get_wm_geometry();
        if (view_impl->edges & WLR_EDGE_LEFT)
        {
            wm.x += geometry.width - new_size.width;
        }

        if (view_impl->edges & WLR_EDGE_TOP)
        {
            wm.y += geometry.height - new_size.height;
        }

        set_position(wm.x, wm.y,
            get_wm_geometry(), false);
    }
}

void wf::wlr_view_t::update_size()
{
    if (!is_mapped())
    {
        return;
    }

    auto current_size = get_size();
    if ((current_size.width == geometry.width) &&
        (current_size.height == geometry.height))
    {
        return;
    }

    /* Damage current size */
    view_damage_raw(self(), last_bounding_box);
    adjust_anchored_edge(current_size);

    view_geometry_changed_signal data;
    data.view = self();
    data.old_geometry = get_wm_geometry();

    geometry.width  = current_size.width;
    geometry.height = current_size.height;

    /* Damage new size */
    last_bounding_box = get_bounding_box();
    view_damage_raw(self(), last_bounding_box);
    emit_signal("geometry-changed", &data);
    wf::get_core().emit_signal("view-geometry-changed", &data);
    if (get_output())
    {
        get_output()->emit_signal("view-geometry-changed", &data);
    }

    if (view_impl->frame)
    {
        view_impl->frame->notify_view_resized(get_wm_geometry());
    }
}

bool wf::wlr_view_t::should_resize_client(
    wf::dimensions_t request, wf::dimensions_t current_geometry)
{
    /*
     * Do not send a configure if the client will retain its size.
     * This is needed if a client starts with one size and immediately resizes
     * again.
     *
     * If we do configure it with the given size, then it will think that we
     * are requesting the given size, and won't resize itself again.
     */
    if (this->last_size_request == wf::dimensions_t{0, 0})
    {
        return request != current_geometry;
    } else
    {
        return request != last_size_request;
    }
}

wf::geometry_t wf::wlr_view_t::get_output_geometry()
{
    return geometry;
}

wf::geometry_t wf::wlr_view_t::get_wm_geometry()
{
    if (view_impl->frame)
    {
        return view_impl->frame->expand_wm_geometry(geometry);
    } else
    {
        return geometry;
    }
}

wlr_surface*wf::wlr_view_t::get_keyboard_focus_surface()
{
    if (is_mapped() && view_impl->keyboard_focus_enabled)
    {
        return surface;
    }

    return NULL;
}

bool wf::wlr_view_t::should_be_decorated()
{
    return role == wf::VIEW_ROLE_TOPLEVEL && !has_client_decoration;
}

void wf::wlr_view_t::set_decoration_mode(bool use_csd)
{
    bool was_decorated = should_be_decorated();
    this->has_client_decoration = use_csd;
    /* This is a moderately kludgey hack to get consistent decorations.
     * This function is not called by native GTK Wayland windows, but is called by 
     * everything else.
     * Xwayland windows are always decorated by the plugin anyway, unless they have a 
     * custom GtkHeaderBar, which the use_csd argument correctly shows - so for
     * Xwayland windows, we don't need to hack anything.
     * We detect Xwayland windows by the absence of a toplevel_handle.
     * If we have a non-Xwayland, non-GTK window, and if the option to only decorate
     * GTK windows is set, we force the has_client_decoration flag to false.
     * This marks the window as not having client-side decorations, and hence the plugin
     * will decorate it. In order for this to work properly, non-GTK windows must have
     * client-side decorations deactivated elsewhere.
     */
    if (this->toplevel_handle)
    {
        wf::option_wrapper_t<bool> only_gtk ("core/only_decorate_gtk");
        if (only_gtk == 1) this->has_client_decoration = false;
    }
    if ((was_decorated != should_be_decorated()) && is_mapped())
    {
        wf::view_decoration_state_updated_signal data;
        data.view = self();

        this->emit_signal("decoration-state-updated", &data);
        if (get_output())
        {
            get_output()->emit_signal("view-decoration-state-updated", &data);
        }
    }
}

void wf::wlr_view_t::set_output(wf::output_t *wo)
{
    auto old_output = get_output();
    toplevel_update_output(old_output, false);
    view_interface_t::set_output(wo);
    toplevel_update_output(wo, true);

    /* send enter/leave events */
    if (this->is_mapped())
    {
        update_output(old_output, wo);
    }
}

void wf::wlr_view_t::commit()
{
    wlr_surface_base_t::commit();
    update_size();

    /* Clear the resize edges.
     * This is must be done here because if the user(or plugin) resizes too fast,
     * the shell client might still haven't configured the surface, and in this
     * case the next commit(here) needs to still have access to the gravity */
    if (!view_impl->in_continuous_resize)
    {
        view_impl->edges = 0;
    }

    this->last_bounding_box = get_bounding_box();

    /* Set non-blending area to render with pixman SRC operator */
    if (wlr_texture_is_pixman(wlr_surface_get_texture(surface)))
    {
        auto wm_geom = get_wm_geometry();
        auto out_geom = get_output_geometry();
	if ((this->last_bounding_box.x == out_geom.x && wm_geom.x > out_geom.x) ||
	    (this->last_bounding_box.y == out_geom.y && wm_geom.y > out_geom.y))
	  wlr_pixman_texture_set_src_op_area(wlr_surface_get_texture(surface),
					     &wm_geom);

	if ((this->last_bounding_box.x == wm_geom.x && out_geom.x > wm_geom.x) ||
	    (this->last_bounding_box.y == wm_geom.y && out_geom.y > wm_geom.y))
	  wlr_pixman_texture_set_src_op_area(wlr_surface_get_texture(surface),
					     &out_geom);
    }
}

void wf::wlr_view_t::map(wlr_surface *surface)
{
    wlr_surface_base_t::map(surface);
    if (wf::get_core_impl().uses_csd.count(surface))
    {
        this->has_client_decoration = wf::get_core_impl().uses_csd[surface];
    }

    update_size();

    if (role == VIEW_ROLE_TOPLEVEL)
    {
        if (!parent)
        {
            get_output()->workspace->add_view(self(), wf::LAYER_WORKSPACE);
        }

        get_output()->focus_view(self(), true);
    }

    damage();
    emit_view_map();
    /* Might trigger repositioning */
    set_toplevel_parent(this->parent);
}

void wf::wlr_view_t::unmap()
{
    damage();
    emit_view_pre_unmap();

    destroy_toplevel();

    // Unset decoration when unmapping, since the policy is to always remove
    // all subsurfaces when a view is unmapped.
    set_decoration(nullptr);

    wlr_surface_base_t::unmap();
    emit_view_unmap();
}

void wf::emit_view_map_signal(wayfire_view view, bool has_position)
{
    wf::view_mapped_signal data;
    data.view = view;
    data.is_positioned = has_position;
    view->get_output()->emit_signal("view-mapped", &data);
    view->emit_signal("mapped", &data);
}

void wf::emit_ping_timeout_signal(wayfire_view view)
{
    wf::view_ping_timeout_signal data;
    data.view = view;
    view->emit_signal("ping-timeout", &data);
}

void wf::view_interface_t::emit_view_map()
{
    emit_view_map_signal(self(), false);
}

void wf::view_interface_t::emit_view_unmap()
{
    view_unmapped_signal data;
    data.view = self();

    if (get_output())
    {
        get_output()->emit_signal("view-unmapped", &data);
        get_output()->emit_signal("view-disappeared", &data);
    }

    emit_signal("unmapped", &data);
}

void wf::view_interface_t::emit_view_pre_unmap()
{
    view_pre_unmap_signal data;
    data.view = self();

    if (get_output())
    {
        get_output()->emit_signal("view-pre-unmapped", &data);
    }

    emit_signal("pre-unmapped", &data);
}

void wf::wlr_view_t::destroy()
{
    view_impl->is_alive = false;
    /* Drop the internal reference created in surface_interface_t */
    unref();
}

void wf::wlr_view_t::create_toplevel()
{
    if (toplevel_handle)
    {
        return;
    }

    /* We don't want to create toplevels for shell views or xwayland menus */
    if (role != VIEW_ROLE_TOPLEVEL)
    {
        return;
    }

    toplevel_handle = wlr_foreign_toplevel_handle_v1_create(
        wf::get_core().protocols.toplevel_manager);

    toplevel_handle_v1_maximize_request.set_callback([&] (void *data)
    {
        auto ev =
            static_cast<wlr_foreign_toplevel_handle_v1_maximized_event*>(data);
        tile_request(ev->maximized ? wf::TILED_EDGES_ALL : 0);
    });
    toplevel_handle_v1_minimize_request.set_callback([&] (void *data)
    {
        auto ev =
            static_cast<wlr_foreign_toplevel_handle_v1_minimized_event*>(data);
        minimize_request(ev->minimized);
    });
    toplevel_handle_v1_activate_request.set_callback(
        [&] (void*) { focus_request(); });
    toplevel_handle_v1_close_request.set_callback([&] (void*) { close(); });

    toplevel_handle_v1_set_rectangle_request.set_callback([&] (void *data)
    {
        auto ev = static_cast<
            wlr_foreign_toplevel_handle_v1_set_rectangle_event*>(data);
        auto surface = wf_view_from_void(ev->surface->data);
        handle_minimize_hint(surface, {ev->x, ev->y, ev->width, ev->height});
    });

    toplevel_handle_v1_maximize_request.connect(
        &toplevel_handle->events.request_maximize);
    toplevel_handle_v1_minimize_request.connect(
        &toplevel_handle->events.request_minimize);
    toplevel_handle_v1_activate_request.connect(
        &toplevel_handle->events.request_activate);
    toplevel_handle_v1_set_rectangle_request.connect(
        &toplevel_handle->events.set_rectangle);
    toplevel_handle_v1_close_request.connect(
        &toplevel_handle->events.request_close);

    toplevel_send_title();
    toplevel_send_app_id();
    toplevel_send_state();
    toplevel_update_output(get_output(), true);
}

void wf::wlr_view_t::destroy_toplevel()
{
    if (!toplevel_handle)
    {
        return;
    }

    toplevel_handle_v1_maximize_request.disconnect();
    toplevel_handle_v1_activate_request.disconnect();
    toplevel_handle_v1_minimize_request.disconnect();
    toplevel_handle_v1_set_rectangle_request.disconnect();
    toplevel_handle_v1_close_request.disconnect();

    wlr_foreign_toplevel_handle_v1_destroy(toplevel_handle);
    toplevel_handle = nullptr;
}

void wf::wlr_view_t::toplevel_send_title()
{
    if (!toplevel_handle)
    {
        return;
    }

    wlr_foreign_toplevel_handle_v1_set_title(toplevel_handle,
        get_title().c_str());
}

void wf::wlr_view_t::toplevel_send_app_id()
{
    if (!toplevel_handle)
    {
        return;
    }

    std::string app_id;

    auto default_app_id   = get_app_id();
    auto gtk_shell_app_id = wf_gtk_shell_get_custom_app_id(
        wf::get_core_impl().gtk_shell, surface->resource);

    std::string app_id_mode =
        wf::option_wrapper_t<std::string>("workarounds/app_id_mode");

    if ((app_id_mode == "gtk-shell") && (gtk_shell_app_id.length() > 0))
    {
        app_id = gtk_shell_app_id;
    } else if (app_id_mode == "full")
    {
        app_id = default_app_id + " " + gtk_shell_app_id;
    } else
    {
        app_id = default_app_id;
    }

    wlr_foreign_toplevel_handle_v1_set_app_id(toplevel_handle, app_id.c_str());
}

void wf::wlr_view_t::toplevel_send_state()
{
    if (!toplevel_handle)
    {
        return;
    }

    wlr_foreign_toplevel_handle_v1_set_maximized(toplevel_handle,
        tiled_edges == TILED_EDGES_ALL);
    wlr_foreign_toplevel_handle_v1_set_activated(toplevel_handle, activated);
    wlr_foreign_toplevel_handle_v1_set_minimized(toplevel_handle, minimized);

    /* update parent as well */
    wf::wlr_view_t *parent_ptr = dynamic_cast<wf::wlr_view_t*>(parent.get());
    wlr_foreign_toplevel_handle_v1_set_parent(toplevel_handle,
        parent_ptr ? parent_ptr->toplevel_handle : nullptr);
}

void wf::wlr_view_t::toplevel_update_output(wf::output_t *wo, bool enter)
{
    if (!wo || !toplevel_handle)
    {
        return;
    }

    if (enter)
    {
        wlr_foreign_toplevel_handle_v1_output_enter(
            toplevel_handle, wo->handle);
    } else
    {
        wlr_foreign_toplevel_handle_v1_output_leave(
            toplevel_handle, wo->handle);
    }
}

void wf::wlr_view_t::desktop_state_updated()
{
    toplevel_send_state();
}

void wf::init_desktop_apis()
{
    init_xdg_shell();
    init_layer_shell();

    wf::option_wrapper_t<bool> xwayland_enabled("core/xwayland");
    if (xwayland_enabled == 1)
    {
        init_xwayland();
    }
}

wf::surface_interface_t*wf::wf_surface_from_void(void *handle)
{
    return static_cast<wf::surface_interface_t*>(handle);
}

wf::view_interface_t*wf::wf_view_from_void(void *handle)
{
    return static_cast<wf::view_interface_t*>(handle);
}

wf::compositor_surface_t*wf::compositor_surface_from_surface(
    wf::surface_interface_t *surface)
{
    return dynamic_cast<wf::compositor_surface_t*>(surface);
}

wf::compositor_interactive_view_t*wf::interactive_view_from_view(
    wf::view_interface_t *view)
{
    return dynamic_cast<wf::compositor_interactive_view_t*>(view);
}

wayfire_view wf::wl_surface_to_wayfire_view(wl_resource *resource)
{
    auto surface = (wlr_surface*)wl_resource_get_user_data(resource);

    void *handle = NULL;
    if (wlr_surface_is_xdg_surface(surface))
    {
        handle = wlr_xdg_surface_from_wlr_surface(surface)->data;
    }

    if (wlr_surface_is_layer_surface(surface))
    {
        handle = wlr_layer_surface_v1_from_wlr_surface(surface)->data;
    }

#if WF_HAS_XWAYLAND
    if (wlr_surface_is_xwayland_surface(surface))
    {
        handle = wlr_xwayland_surface_from_wlr_surface(surface)->data;
    }

#endif

    wf::view_interface_t *view = wf::wf_view_from_void(handle);

    return view ? view->self() : nullptr;
}
