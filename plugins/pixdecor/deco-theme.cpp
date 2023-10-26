#include "deco-theme.hpp"
#include <wayfire/core.hpp>
#include <wayfire/opengl.hpp>
#include <config.h>
#include <map>

gboolean read_colour (char *file, const char *name, float *r, float *g, float *b)
{
    char *cmd = g_strdup_printf ("sed -n -e \"s/@define-color[ \t]*%s[ \t]*//p\" %s", name, file);
    char *line = NULL;
    size_t len = 0;
    int n = 0, ir, ig, ib;
    FILE *fp = popen (cmd, "r");

    if (fp)
    {
        if (getline (&line, &len, fp) > 0)
        {
            n = sscanf (line, "#%02x%02x%02x", &ir, &ig, &ib);
            g_free (line);
        }
        pclose (fp);
    }
    g_free (cmd);
    if (n == 3)
    {
        *r = ir / 255.0;
        *g = ig / 255.0;
        *b = ib / 255.0;
        return TRUE;
    }
    return FALSE;
}

namespace wf
{
namespace decor
{
/** Create a new theme with the default parameters */
decoration_theme_t::decoration_theme_t()
{
    float r, g, b;
    gs = g_settings_new ("org.gnome.desktop.interface");
    char *theme = g_settings_get_string (gs, "gtk-theme");

    // read the current colour scheme
    char *userconf = g_build_filename (g_get_user_data_dir (), "themes/", theme, "/gtk-3.0/gtk.css", NULL);
    char *sysconf = g_build_filename ("/usr/share/themes/", theme, "/gtk-3.0/gtk-colours.css", NULL);

    if (read_colour (userconf, "theme_selected_bg_color", &r, &g, &b)
        || read_colour (sysconf, "theme_selected_bg_color", &r, &g, &b))
            fg = {r, g, b, 1.0};
    else fg = {0.13, 0.13, 0.13, 0.67};

    if (read_colour (userconf, "theme_selected_fg_color", &r, &g, &b)
        || read_colour (sysconf, "theme_selected_fg_color", &r, &g, &b))
            fg_text = {r, g, b, 1.0};
    else fg_text = {1.0, 1.0, 1.0, 1.0};

    if (read_colour (userconf, "theme_unfocused_bg_color", &r, &g, &b)
        || read_colour (sysconf, "theme_unfocused_bg_color", &r, &g, &b))
            bg = {r, g, b, 1.0};
    else bg = {0.2, 0.2, 0.2, 0.87};

    if (read_colour (userconf, "theme_unfocused_fg_color", &r, &g, &b)
        || read_colour (sysconf, "theme_unfocused_fg_color", &r, &g, &b))
            bg_text = {r, g, b, 1.0};
    else bg_text = {1.0, 1.0, 1.0, 1.0};

    g_free (sysconf);
    g_free (userconf);
    g_free (theme);
}

/** @return The available height for displaying the title */
int decoration_theme_t::get_font_height_px() const
{
    char *font = g_settings_get_string (gs, "font-name");

    PangoFontDescription *font_desc = pango_font_description_from_string (font);
    int font_height = pango_font_description_get_size (font_desc);
    g_free (font);
    if (!pango_font_description_get_size_is_absolute (font_desc))
    {
        font_height *= 4;
        font_height /= 3;
    }
    return font_height / PANGO_SCALE;
}

int decoration_theme_t::get_title_height() const
{
    int height = get_font_height_px ();
    height *= 3;
    height /= 2;
    height += 8;
    if (height < MIN_BAR_HEIGHT) return MIN_BAR_HEIGHT;
    else return height;
}

/** @return The available border for resizing */
int decoration_theme_t::get_border_size() const
{
    return border_size;
}

void decoration_theme_t::set_maximize (bool state)
{
    maximized = state;
}

/**
 * Fill the given rectangle with the background color(s).
 *
 * @param fb The target framebuffer, must have been bound already
 * @param rectangle The rectangle to redraw.
 * @param scissor The GL scissor rectangle to use.
 * @param active Whether to use active or inactive colors
 */
void decoration_theme_t::render_background(const wf::framebuffer_t& fb,
    wf::geometry_t rectangle, const wf::geometry_t& scissor, bool active) const
{
    wf::color_t color = active ? fg : bg;
    OpenGL::render_begin (fb);
    fb.logic_scissor (scissor);
    int border = maximized ? 0 : get_border_size ();

    // adjust for invisible border
    rectangle.x += border;
    rectangle.y += border;
    rectangle.width -= 2 * border;

    // draw background
    rectangle.height = get_title_height ();
    OpenGL::render_rectangle (rectangle, color, fb.get_orthographic_projection());

    OpenGL::render_end ();
}

/**
 * Render the given text on a cairo_surface_t with the given size.
 * The caller is responsible for freeing the memory afterwards.
 */
cairo_surface_t*decoration_theme_t::render_text(std::string text,
    int width, int height, int t_width, bool active) const
{
    const auto format = CAIRO_FORMAT_ARGB32;
    auto surface = cairo_image_surface_create(format, width, height);

    if (height == 0)
    {
        return surface;
    }

    auto cr = cairo_create(surface);

    PangoFontDescription *font_desc;
    PangoLayout *layout;
    char *font = g_settings_get_string (gs, "font-name");
    int w, h;

    // render text
    font_desc = pango_font_description_from_string (font);

    layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(layout, font_desc);
    pango_layout_set_text(layout, text.c_str(), text.size());
    cairo_set_source_rgba(cr, active ? fg_text.r : bg_text.r, active ? fg_text.g : bg_text.g, active ? fg_text.b : bg_text.b, 1);
    pango_layout_get_pixel_size (layout, &w, &h);
    cairo_translate (cr, (t_width - w) / 2, (height - h) / 2);
    pango_cairo_show_layout(cr, layout);
    pango_font_description_free(font_desc);
    g_object_unref(layout);
    cairo_destroy(cr);
    g_free (font);

    return surface;
}

cairo_surface_t *decoration_theme_t::get_button_surface(button_type_t button,
    const button_state_t& state, bool active) const
{
    cairo_surface_t *cspng, *csout;
    unsigned char *sdata, *tdata;
    const char *icon_name;
    char *iconfile;
    int sh, sw, th, tw, pad, r, g, b;
    float fr, fg, fb;

    // get the current text colour
    fr = (active ? fg_text.r : bg_text.r) * 255.0;
    fg = (active ? fg_text.g : bg_text.g) * 255.0;
    fb = (active ? fg_text.b : bg_text.b) * 255.0;
    r = fr;
    g = fg;
    b = fb;

    switch (button)
    {
        case BUTTON_CLOSE :             icon_name = "close";
                                        break;
        case BUTTON_TOGGLE_MAXIMIZE :   if (maximized)
                                            icon_name = "restore";
                                        else
                                            icon_name = "maximize";
                                        break;
        case BUTTON_MINIMIZE :          icon_name = "minimize";
                                        break;
    }

    // these get recoloured according to theme, so just use the light theme version
    iconfile = g_strdup_printf ("/usr/share/themes/PiXflat/gtk-3.0/assets/window-%s%s%s.symbolic.png",
         icon_name, state.hover ? "-hover" : "", get_font_height_px () >= LARGE_ICON_THRESHOLD ? "-large" : "");

    // read the icon into a surface
    cspng = cairo_image_surface_create_from_png (iconfile);
    sdata = cairo_image_surface_get_data (cspng);
    sh = cairo_image_surface_get_height (cspng);
    sw = cairo_image_surface_get_width (cspng);

    // create a larger surface
    tw = state.width;
    th = state.height;
    csout = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, tw, th);
    tdata = cairo_image_surface_get_data (csout);

    // centre and re-colour the bitmap in the new surface
    pad = (th - sh) / 2;
    for (int i = 0; i < th; i++)
    {
        for (int j = 0; j < tw; j++)
        {
            if (i < pad || i >= sh + pad || j < pad || j >= sw + pad) tdata += 4;
            else
            {
                sdata += 3;
                *tdata++ = (*sdata == 0xff) ? b : 0;
                *tdata++ = (*sdata == 0xff) ? g : 0;
                *tdata++ = (*sdata == 0xff) ? r : 0;
                *tdata++ = *sdata++;
            }
        }
    }

    cairo_surface_destroy (cspng);
    g_free (iconfile);

    return csout;
}
}
}
