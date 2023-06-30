#pragma once

#include <string>
#include <wayfire/plugins/common/simple-texture.hpp>
#include <wayfire/config/types.hpp>
#include <cairo.h>
#include <stdlib.h>
#include <wayfire/util/log.hpp>
#include <drm_fourcc.h>

namespace wf
{
struct simple_texture_t;
}

/**
 * Upload the data from the cairo surface to the OpenGL texture.
 *
 * @param surface The source cairo surface.
 * @param buffer  The buffer to upload data to.
 */
static void cairo_surface_upload_to_texture(
    cairo_surface_t *surface, wf::simple_texture_t& buffer)
{
    buffer.width  = cairo_image_surface_get_width(surface);
    buffer.height = cairo_image_surface_get_height(surface);

    auto src = cairo_image_surface_get_data(surface);

   /* NB: We have to use the getenv version of this test as various
    * plugins include this file and trying to include ../main.hpp
    * causes compile errors for those plugins */
    if (!getenv("WAYFIRE_USE_PIXMAN"))
     {
        if (buffer.tex == (GLuint) - 1)
          {
             GL_CALL(glGenTextures(1, &buffer.tex));
          }

        GL_CALL(glBindTexture(GL_TEXTURE_2D, buffer.tex));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, src));
     }
   else
     {
        void *data;
        uint32_t format;
        size_t stride;
        auto renderer = wf::get_core().renderer;

        if (!buffer.buffer)
          {
             auto allocator = wf::get_core().allocator;
             const struct wlr_drm_format_set *fset;
             const struct wlr_drm_format *dformat;

             fset = wlr_renderer_get_render_formats(renderer);
             if (!fset)
               {
                  wlr_log(WLR_DEBUG, "Cannot get render foramts");
                  return;
               }
             dformat = wlr_drm_format_set_get(fset, DRM_FORMAT_ARGB8888);
             if (!dformat)
               {
                  wlr_log(WLR_DEBUG, "Cannot get drm format");
                  return;
               }

             buffer.buffer =
               wlr_allocator_create_buffer(allocator, buffer.width,
                                           buffer.height, dformat);
             if (!buffer.buffer)
               {
                  wlr_log(WLR_DEBUG, "Cannot create texture buffer");
                  return;
               }
          }

        /* we should have a buffer by now, copy cairo_surface pixels */
        if (!wlr_buffer_begin_data_ptr_access(buffer.buffer,
                                              WLR_BUFFER_DATA_PTR_ACCESS_WRITE,
                                              &data, &format, &stride))
          {
             wlr_log(WLR_DEBUG, "Cannot access buffer data ptr");
             return;
          }

        memcpy(data, src, stride * buffer.height);
        wlr_buffer_end_data_ptr_access(buffer.buffer);

        buffer.texture = wlr_texture_from_buffer(renderer, buffer.buffer);
     }
}

namespace wf
{
/**
 * Simple wrapper around rendering text with Cairo. This object can be
 * kept around to avoid reallocation of the cairo surface and OpenGL
 * texture on repeated renders.
 */
struct cairo_text_t
{
    wf::simple_texture_t tex;

    /* parameters used for rendering */
    struct params
    {
        /* font size */
        int font_size = 12;
        /* color for background rectangle (only used if bg_rect == true) */
        wf::color_t bg_color;
        /* text color */
        wf::color_t text_color;
        /* scale everything by this amount */
        float output_scale = 1.f;
        /* crop result to this size (if nonzero);
         * note that this is multiplied by output_scale */
        wf::dimensions_t max_size{0, 0};
        /* draw a rectangle in the background with bg_color */
        bool bg_rect = true;
        /* round the corners of the background rectangle */
        bool rounded_rect = true;
        /* if true, the resulting surface will be cropped to the
         * minimum size necessary to fit the text; otherwise, the
         * resulting surface might be bigger than necessary and the
         * text is centered in it */
        bool exact_size = false;

        params()
        {}
        params(int font_size_, const wf::color_t& bg_color_,
            const wf::color_t& text_color_, float output_scale_ = 1.f,
            const wf::dimensions_t& max_size_ = {0, 0},
            bool bg_rect_ = true, bool exact_size_ = false) :
            font_size(font_size_), bg_color(bg_color_),
            text_color(text_color_), output_scale(output_scale_),
            max_size(max_size_), bg_rect(bg_rect_),
            exact_size(exact_size_)
        {}
    };

    /**
     * Render the given text in the texture tex.
     *
     * @param text         text to render
     * @param par          parameters for rendering
     *
     * @return The size needed to render in scaled coordinates. If this is larger
     *   than the size of tex, it means the result was cropped (due to the constraint
     *   given in par.max_size). If it is smaller, than the result is centered along
     *   that dimension.
     */
    wf::dimensions_t render_text(const std::string& text, const params& par)
    {
        if (!cr)
        {
            /* create with default size */
            cairo_create_surface();
        }

        cairo_text_extents_t extents;
        cairo_font_extents_t font_extents;
        /* TODO: font properties could be made parameters! */
        cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, par.font_size * par.output_scale);
        cairo_text_extents(cr, text.c_str(), &extents);
        cairo_font_extents(cr, &font_extents);

        double xpad = par.bg_rect ? 10.0 * par.output_scale : 0.0;
        double ypad = par.bg_rect ? 0.2 * (font_extents.ascent +
            font_extents.descent) : 0.0;
        int w = (int)(extents.width + 2 * xpad);
        int h = (int)(font_extents.ascent + font_extents.descent + 2 * ypad);
        wf::dimensions_t ret = {w, h};
        if (par.max_size.width && (w > par.max_size.width * par.output_scale))
        {
            w = (int)std::floor(par.max_size.width * par.output_scale);
        }

        if (par.max_size.height && (h > par.max_size.height * par.output_scale))
        {
            h = (int)std::floor(par.max_size.height * par.output_scale);
        }

        if ((w != surface_size.width) || (h != surface_size.height))
        {
            if (par.exact_size || (w > surface_size.width) ||
                (h > surface_size.height))
            {
                surface_size.width  = w;
                surface_size.height = h;
                cairo_create_surface();
            }
        }

        cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cr);

        int x = (surface_size.width - w) / 2;
        int y = (surface_size.height - h) / 2;

        if (par.bg_rect)
        {
            int min_r = (int)(20 * par.output_scale);
            int r     = par.rounded_rect ? (h > min_r ? min_r : (h - 2) / 2) : 0;

            cairo_move_to(cr, x + r, y);
            cairo_line_to(cr, x + w - r, y);
            if (par.rounded_rect)
            {
                cairo_curve_to(cr, x + w, y, x + w, y, x + w, y + r);
            }

            cairo_line_to(cr, x + w, y + h - r);
            if (par.rounded_rect)
            {
                cairo_curve_to(cr, x + w, y + h, x + w, y + h, x + w - r, y + h);
            }

            cairo_line_to(cr, x + r, y + h);
            if (par.rounded_rect)
            {
                cairo_curve_to(cr, x, y + h, x, y + h, x, y + h - r);
            }

            cairo_line_to(cr, x, y + r);
            if (par.rounded_rect)
            {
                cairo_curve_to(cr, x, y, x, y, x + r, y);
            }

            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_set_source_rgba(cr, par.bg_color.r, par.bg_color.g,
                par.bg_color.b, par.bg_color.a);
            cairo_fill(cr);
        }

        x += xpad;
        y += ypad + font_extents.ascent;
        cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, par.font_size * par.output_scale);
        cairo_move_to(cr, x - extents.x_bearing, y);
        cairo_set_source_rgba(cr, par.text_color.r, par.text_color.g,
            par.text_color.b, par.text_color.a);
        cairo_show_text(cr, text.c_str());

        cairo_surface_flush(surface);

       /* NB: We have to use the getenv version of this test as various
        * plugins include this file and trying to include ../main.hpp
        * causes compile errors for those plugins */
       if (!getenv("WAYFIRE_USE_PIXMAN"))
         {
            OpenGL::render_begin();
            cairo_surface_upload_to_texture(surface, tex);
            OpenGL::render_end();
         }
       else
         {
            cairo_surface_upload_to_texture(surface, tex);
         }

        return ret;
    }

    /**
     * Standalone function version to render text to an OpenGL texture
     */
    static wf::dimensions_t cairo_render_text_to_texture(const std::string& text,
        const wf::cairo_text_t::params& par, wf::simple_texture_t& tex)
    {
        wf::cairo_text_t ct;
        /* note: we "borrow" the texture from what was supplied (if any) */
        ct.tex.tex = tex.tex;
        auto ret = ct.render_text(text, par);
        if (tex.tex == (GLuint) - 1)
        {
            tex.tex = ct.tex.tex;
        }

        tex.width  = ct.tex.width;
        tex.height = ct.tex.height;
        ct.tex.tex = -1;
        return ret;
    }

    ~cairo_text_t()
    {
        cairo_free();
    }

    /**
     * Calculate the height of text rendered with a given font size.
     *
     * @param font_size  Desired font size.
     * @param bg_rect    Whether a background rectangle should be taken into account.
     *
     * @returns Required height of the surface.
     */
    static unsigned int measure_height(int font_size, bool bg_rect = true)
    {
        cairo_text_t dummy;
        dummy.surface_size.width  = 1;
        dummy.surface_size.height = 1;
        dummy.cairo_create_surface();

        cairo_font_extents_t font_extents;
        /* TODO: font properties could be made parameters! */
        cairo_select_font_face(dummy.cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(dummy.cr, font_size);
        cairo_font_extents(dummy.cr, &font_extents);

        double ypad = bg_rect ? 0.2 * (font_extents.ascent +
            font_extents.descent) : 0.0;
        unsigned int h = (unsigned int)std::ceil(font_extents.ascent +
            font_extents.descent + 2 * ypad);
        return h;
    }

  protected:
    /* cairo context and surface for the text */
    cairo_t *cr = nullptr;
    cairo_surface_t *surface = nullptr;
    /* current width and height of the above surface */
    wf::dimensions_t surface_size = {400, 100};


    void cairo_free()
    {
        if (cr)
        {
            cairo_destroy(cr);
        }

        if (surface)
        {
            cairo_surface_destroy(surface);
        }

        cr = nullptr;
        surface = nullptr;
    }

    void cairo_create_surface()
    {
        cairo_free();
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surface_size.width,
            surface_size.height);
        cr = cairo_create(surface);
    }
};
}
