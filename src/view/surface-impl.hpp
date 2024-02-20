#ifndef SURFACE_IMPL_HPP
#define SURFACE_IMPL_HPP

#include <wayfire/opengl.hpp>
#include <wayfire/surface.hpp>
#include <wayfire/util.hpp>

namespace wf
{
class surface_interface_t::impl
{
  public:
    surface_interface_t *parent_surface;
    std::vector<std::unique_ptr<surface_interface_t>> surface_children_above;
    std::vector<std::unique_ptr<surface_interface_t>> surface_children_below;
    size_t last_cnt_surfaces = 0;

    /**
     * Remove all subsurfaces and emit signals for them.
     */
    void clear_subsurfaces(surface_interface_t *self);

    wf::output_t *output = nullptr;
    static int active_shrink_constraint;

    /**
     * Most surfaces don't have a wlr_surface. However, internal surface
     * implementations can set the underlying surface so that functions like
     *
     * subtract_opaque(), send_frame_done(), etc. work for the surface
     */
    wlr_surface *wsurface = nullptr;
    bool _closing = false;
};

/**
 * A base class for views and surfaces which are based on a wlr_surface
 * Any class that derives from wlr_surface_base_t must also derive from
 * surface_interface_t!
 */
class wlr_surface_base_t
{
  protected:
    wf::wl_listener_wrapper::callback_t handle_new_subsurface;
    wf::wl_listener_wrapper on_commit, on_destroy, on_new_subsurface;

    void apply_surface_damage();
    wlr_surface_base_t(wf::surface_interface_t *self);
    /* Pointer to this as surface_interface, see requirement above */
    wf::surface_interface_t *_as_si = nullptr;

  public:
    /* if surface != nullptr, then the surface is mapped */
    wlr_surface *surface = nullptr;

    virtual ~wlr_surface_base_t();

    /** @return The offset from the surface coordinates to the actual geometry */
    virtual wf::point_t get_window_offset();

    /** Update the surface output */
    virtual void update_output(wf::output_t *old_output,
        wf::output_t *new_output);

    /*
     * Functions that need to be implemented/overridden from the
     * surface_implementation_t
     */
    virtual bool _is_mapped() const;
    virtual wf::dimensions_t _get_size() const;
    virtual void _simple_render(const wf::framebuffer_t& fb, int x, int y,
        const wf::region_t& damage);

  protected:
    virtual void map(wlr_surface *surface);
    virtual void unmap();
    virtual void commit();

    virtual wlr_buffer *get_buffer();
};

/**
 * wlr_child_surface_base_t is a base class for wlr-surface based child
 * surfaces, i.e subsurfaces.
 *
 * However, they can still exist without a parent, for ex. drag icons.
 */
class wlr_child_surface_base_t :
    public surface_interface_t, public wlr_surface_base_t
{
  public:
    wlr_child_surface_base_t(surface_interface_t *self);
    virtual ~wlr_child_surface_base_t();

    /* Just pass to the default wlr surface implementation */
    virtual bool is_mapped() const override
    {
        return _is_mapped();
    }

    virtual wf::dimensions_t get_size() const override
    {
        return _get_size();
    }

    virtual void simple_render(const wf::framebuffer_t& fb, int x, int y,
        const wf::region_t& damage) override
    {
        _simple_render(fb, x, y, damage);
    }

    virtual void set_output(wf::output_t *output) override
    {
        update_output(get_output(), output);
        surface_interface_t::set_output(output);
    }
};
}

#endif /* end of include guard: SURFACE_IMPL_HPP */
