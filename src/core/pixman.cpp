#include <wayfire/util/log.hpp>
#include <map>
#include "pixman-priv.hpp"
#include "wayfire/output.hpp"
#include "core-impl.hpp"
#include "config.h"
#include <wayfire/nonstd/wlroots-full.hpp>
#include <sys/mman.h>
#include <drm_fourcc.h>
#include <wlr/types/wlr_matrix.h>

namespace Pixman
{
   struct wlr_buffer *current_output_fb = NULL;

   void init()
     {
        wlr_log(WLR_DEBUG, "Pixman Render Init");
        /* render_begin(); */
        /* render_end(); */
     }

   void fini()
     {
        wlr_log(WLR_DEBUG, "Pixman Render Finish");
        /* render_begin(); */
        /* render_end(); */
     }

   void bind_output(struct wlr_buffer *fb)
     {
        wlr_log(WLR_DEBUG, "Pixman Render Bind Output %p", fb);
        current_output_fb = fb;
     }

   void unbind_output()
     {
        wlr_log(WLR_DEBUG, "Pixman Render Unbind Output");
        current_output_fb = NULL;
     }

   void render_begin()
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render Begin"); */
        /* auto renderer = wf::get_core().renderer; */
        /* wlr_renderer_begin(renderer); */
     }

   void render_begin(const wf::framebuffer_base_t& fb)
     {
        wlr_log(WLR_DEBUG, "Pixman Render Begin With FB: %p", fb.buffer);
        fb.bind();
     }

   void render_begin(int32_t width, int32_t height)
     {
        wlr_log(WLR_DEBUG, "Pixman Render Begin With Size %d %d",
                width, height);
        auto renderer = wf::get_core().renderer;
        wlr_renderer_begin(renderer, width, height);
     }

   void render_begin(int32_t width, int32_t height, uint32_t fb)
     {
        wlr_log(WLR_DEBUG, "Pixman Render Begin With Size %d %d and FB %d",
                width, height, fb);
        auto renderer = wf::get_core().renderer;
        wlr_renderer_begin(renderer, width, height);
     }

   void render_rectangle(wf::geometry_t box, wf::color_t color, glm::mat4 matrix)
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render Rectangle"); */
        /* wlr_log(WLR_DEBUG, "\tColor: %9.6f %9.6f %9.6f %9.6f", */
        /*         (float)color.r, (float)color.g, (float)color.b, (float)color.a); */
        /* wlr_log(WLR_DEBUG, "\tBox: %d %d %d %d", box.x, box.y, box.width, box.height); */

        auto renderer = wf::get_core().renderer;
        const struct wlr_box wbox =
          {
             .x = box.x,
             .y = box.y,
             .width = box.width,
             .height = box.height,
          };
        /* FIXME: Are the passed in values already premultiplied ?? */
        const float c[4] =
          {
             (float)color.r, (float)color.g, (float)color.b, (float)color.a
          };

        float mat[9];
        wlr_matrix_identity(mat);
        wlr_render_rect(renderer, &wbox, c, mat);
     }

   void render_texture(wf::texture_t tex, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color)
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render WF::Texture %p Framebuffer", tex); */
        /* wlr_log(WLR_DEBUG, "\tGeometry: %d %d %d %d", */
        /*         geometry.x, geometry.y, geometry.width, geometry.height); */

        if (tex.texture)
          render_transformed_texture(tex.texture, geometry,
                                     framebuffer.get_orthographic_projection(),
                                     color);
        else if (tex.surface)
          {
             auto texture = wlr_surface_get_texture(tex.surface);
             if (texture)
               render_transformed_texture(texture, geometry,
                                          framebuffer.get_orthographic_projection(),
                                          color);
          }
     }

   void render_texture(struct wlr_texture *texture, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color)
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render Texture %p Framebuffer", texture); */

        render_transformed_texture(texture, geometry,
                                   framebuffer.get_orthographic_projection(),
                                   color);
     }

   void render_transformed_texture(struct wlr_texture *tex, const gl_geometry& g, const gl_geometry& texg, glm::mat4 transform, glm::vec4 color)
     {
        float mat[9];

        /* wlr_log(WLR_DEBUG, "Pixman Render Transformed Texture %p with gl_geometry", */
        /*         tex); */
        /* wlr_log(WLR_DEBUG, "\tgl_geometry: %9.6f %9.6f %9.6f %9.6f", */
        /*         g.x1, g.y1, g.x2, g.y2); */

        if (!tex) return;
        auto renderer = wf::get_core().renderer;
        auto output = wf::get_core().get_active_output();
        const struct wlr_box wbox =
          {
             .x = (int)g.x1,
             .y = (int)g.y1,
             .width = (int)g.x2,
             .height = (int)g.y2,
          };

        wlr_matrix_project_box(mat, &wbox, WL_OUTPUT_TRANSFORM_NORMAL, 0,
                               output->handle->transform_matrix);
        wlr_render_texture_with_matrix(renderer, tex, mat, (float)color.a);
     }

   void render_transformed_texture(struct wlr_texture *tex, const wf::geometry_t& geometry, glm::mat4 transform, glm::vec4 color)
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render Transformed Texture %p with wf::geometry_t", */
        /*         tex); */
        /* wlr_log(WLR_DEBUG, "\twf:geometry: %d %d %d %d", */
        /*         geometry.x, geometry.y, geometry.width, geometry.height); */

        if (!tex) return;

        gl_geometry gg;

        /*
         * x2, y1 == top right
         * x1, y1 == top left
         * x2, y2 == bottom right
         * x1, y2 == bottom left
         */

        gg.x1 = geometry.x;
        gg.y1 = geometry.y;
        gg.x2 = geometry.width;
        gg.y2 = geometry.height;

        render_transformed_texture(tex, gg, {}, transform, color);
     }

   void render_end()
     {
        wlr_log(WLR_DEBUG, "Pixman Render End");
        auto renderer = wf::get_core().renderer;
        wlr_renderer_scissor(renderer, NULL);
        wlr_renderer_end(renderer);
     }

   void clear(wf::color_t color)
     {
        wlr_log(WLR_DEBUG, "Pixman Render Clear");
        const float col[4] =
          {
             (float)color.r, (float)color.g, (float)color.b, (float)color.a
          };
        auto renderer = wf::get_core().renderer;
        wlr_renderer_clear(renderer, col);
     }

   bool fb_alloc(wf::framebuffer_base_t *fb, int width, int height)
     {
        auto renderer = wf::get_core().renderer;
        bool first_allocate = false;
        bool is_resize = false;

        /* wlr_log(WLR_DEBUG, "Pixman FB %p Alloc: %d %d", fb, width, height); */
        /* wlr_log(WLR_DEBUG, "\tBuffer: %p", fb->buffer); */

        if (!fb->buffer)
          {
             auto allocator = wf::get_core().allocator;
             const struct wlr_drm_format_set *formats;
             const struct wlr_drm_format *format;

             first_allocate = true;
             formats = wlr_renderer_get_render_formats(renderer);
             if (!formats)
               {
                  wlr_log(WLR_DEBUG, "Cannot get render formats");
                  return false;
               }
             format = wlr_drm_format_set_get(formats, DRM_FORMAT_ARGB8888);
             if (!format)
               {
                  wlr_log(WLR_DEBUG, "Cannot get format");
                  return false;
               }
             fb->buffer =
               wlr_allocator_create_buffer(allocator, width, height, format);
             if (!fb->buffer)
               {
                  wlr_log(WLR_DEBUG, "Cannot create wlr_buffer");
                  return false;
               }

             /* wlr_log(WLR_DEBUG, "\tAllocated new buffer %p", fb->buffer); */
          }

        if (!fb->texture)
          fb->texture = wlr_texture_from_buffer(renderer, fb->buffer);

        if (fb->buffer != Pixman::current_output_fb)
          {
             if (first_allocate || (width != fb->viewport_width) ||
                 (height != fb->viewport_height))
               {
                  is_resize = true;
               }
          }

        return is_resize || first_allocate;
     }
}
