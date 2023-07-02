#include "deco-button.hpp"
#include "deco-theme.hpp"
#include <wayfire/opengl.hpp>
#include <wayfire/pixman.hpp>
#include <wayfire/plugins/common/cairo-util.hpp>
#include <stdlib.h>

namespace wf
{
namespace decor
{
button_t::button_t(const decoration_theme_t& t, wf::geometry_t geom, std::function<void()> damage) :
    theme(t), geometry(geom), damage_callback(damage)
{}

void button_t::set_button_type(button_type_t type)
{
    this->type = type;
    update_texture();
    add_idle_damage();
}

button_type_t button_t::get_button_type() const
{
    return this->type;
}

void button_t::set_hover(bool is_hovered)
{
    this->is_hovered = is_hovered;
    add_idle_damage();
}

/**
 * Set whether the button is pressed or not.
 * Affects appearance.
 */
void button_t::set_pressed(bool is_pressed)
{
    this->is_pressed = is_pressed;
    add_idle_damage();
}

void button_t::render(const wf::framebuffer_t& fb, wf::geometry_t geometry,
    wf::geometry_t scissor, bool active)
{
    if (this->active != active)
    {
        this->active = active;
        update_texture ();
        add_idle_damage ();
    }

   if (!getenv("WAYFIRE_USE_PIXMAN"))
//    if (!runtime_config.use_pixman)
     {
        OpenGL::render_begin(fb);
        fb.logic_scissor(scissor);
        OpenGL::render_texture(button_texture.tex, fb, geometry, {1, 1, 1, 1},
                               OpenGL::TEXTURE_TRANSFORM_INVERT_Y);
        OpenGL::render_end();
     }
   else
     {
        Pixman::render_begin(fb);
        fb.logic_scissor(scissor);
        Pixman::render_texture(button_texture.texture, fb, geometry, {1, 1, 1, 1});
        Pixman::render_end();
     }
}

void button_t::update_texture()
{
    decoration_theme_t::button_state_t state = {
        .width  = 1.0 * this->geometry.width,
        .height = 1.0 * this->geometry.width,
        .border = 1.0,
        .hover = this->is_hovered,
    };

    auto surface = theme.get_button_surface(type, state, this->active);

   if (!getenv("WAYFIRE_USE_PIXMAN"))
//    if (!runtime_config.use_pixman)
     {
        OpenGL::render_begin();
        cairo_surface_upload_to_texture(surface, this->button_texture);
        OpenGL::render_end();
     }
   else
     {
        cairo_surface_upload_to_texture(surface, this->button_texture);
     }

   cairo_surface_destroy(surface);
}

void button_t::add_idle_damage()
{
    this->idle_damage.run_once([=] ()
    {
        this->damage_callback();
        update_texture();
    });
}
}
}
