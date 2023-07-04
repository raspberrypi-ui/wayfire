#pragma once
#include <gio/gio.h>
#include <wayfire/render-manager.hpp>
#include "deco-button.hpp"

#define LARGE_ICON_THRESHOLD 20
#define MIN_BAR_HEIGHT 20
#define BUTTON_W_PAD 2

namespace wf
{
namespace decor
{
/**
 * A  class which manages the outlook of decorations.
 * It is responsible for determining the background colors, sizes, etc.
 */
class decoration_theme_t
{
  public:
    /** Create a new theme with the default parameters */
    decoration_theme_t(bool state);

    /** @return The height of the system font in pixels */
    int get_font_height_px() const;
    /** @return The available height for displaying the title */
    int get_title_height() const;
    /** @return The available border for resizing */
    int get_border_size() const;

    bool get_decorated () const;

    /**
     * Fill the given rectangle with the background color(s).
     *
     * @param fb The target framebuffer, must have been bound already.
     * @param rectangle The rectangle to redraw.
     * @param scissor The GL scissor rectangle to use.
     * @param active Whether to use active or inactive colors
     */
    void render_background(const wf::framebuffer_t& fb, wf::geometry_t rectangle,
        const wf::geometry_t& scissor, bool active) const;

    /**
     * Render the given text on a cairo_surface_t with the given size.
     * The caller is responsible for freeing the memory afterwards.
     */
    cairo_surface_t *render_text(std::string text, int width, int height, int t_width, bool active) const;

    struct button_state_t
    {
        /** Button width */
        double width;
        /** Button height */
        double height;
        /** Button outline size */
        double border;
        /* Hovering... */
        bool hover;
    };

    /**
     * Get the icon for the given button.
     * The caller is responsible for freeing the memory afterwards.
     *
     * @param button The button type.
     * @param state The button state.
     */
    cairo_surface_t *get_button_surface(button_type_t button,
        const button_state_t& state, bool active) const;

    void set_maximize (bool state);

  private:
    wf::option_wrapper_t<int> border_size{"pixdecor/border_size"};

    GSettings *gs;
	wf::color_t fg;
	wf::color_t bg;
	wf::color_t fg_text;
	wf::color_t bg_text;
	bool maximized;
    bool dec;
};
}
}
