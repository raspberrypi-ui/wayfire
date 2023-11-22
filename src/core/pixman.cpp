#include <wayfire/util/log.hpp>
#include <map>
#include "pixman-priv.hpp"
//#include "wayfire/render-manager.hpp"
#include "wayfire/output.hpp"
#include "core-impl.hpp"
#include "config.h"
#include <wayfire/nonstd/wlroots-full.hpp>
#include <sys/mman.h>
#include <drm_fourcc.h>
#include <wlr/types/wlr_matrix.h>
#include <glm/gtc/type_ptr.hpp>

namespace Pixman
{
   struct wlr_buffer *current_output_fb = NULL;

   void mat4_to_mat3(glm::mat4 matrix, float mat[9])
     {
        mat[0] = matrix[0][0];
        mat[1] = matrix[1][0];
        mat[2] = matrix[3][0];
        mat[3] = matrix[0][1];
        mat[4] = matrix[1][1];
        mat[5] = matrix[3][1];
        mat[6] = matrix[0][3];
        mat[7] = matrix[1][3];
        mat[8] = 1.0f;
     }

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
        wlr_log(WLR_DEBUG, "Pixman Render Unbind Output %p", current_output_fb);
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

   void render_begin(struct wlr_buffer *buffer)
     {
        wlr_log(WLR_DEBUG, "Pixman Render Begin With wlr_buffer %p",
                buffer);
        auto renderer = wf::get_core().renderer;
        wlr_renderer_begin_with_buffer(renderer, buffer);
     }

   void render_rectangle(wf::geometry_t box, wf::color_t color, glm::mat4 matrix)
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render Rectangle"); */
        /* wlr_log(WLR_DEBUG, "\tColor: %9.6f %9.6f %9.6f %9.6f", */
        /*         (float)color.r, (float)color.g, (float)color.b, (float)color.a); */
        /* wlr_log(WLR_DEBUG, "\tBox: %d %d %d %d", box.x, box.y, box.width, box.height); */

        if (box.width <= 0 || box.height <= 0) {
            return;
        }

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
        Pixman::mat4_to_mat3(matrix, mat);

        wlr_render_rect(renderer, &wbox, c, mat);
     }

   void render_texture(wf::texture_t tex, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color)
     {
       /* wlr_log(WLR_DEBUG, "Pixman Render WF::Texture %p Framebuffer", tex); */
       /* wlr_log(WLR_DEBUG, "\tGeometry: %d %d %d %d", */
       /*          geometry.x, geometry.y, geometry.width, geometry.height); */

       float mat[9];
       framebuffer.get_orthographic_projection(mat);

       wf::geometry_t geo = geometry;
       int width, height;
       if (tex.texture) {
          width = tex.texture->width;
	      height = tex.texture->height;
       } else if (tex.surface) {
	      width = tex.surface->current.width;
	      height = tex.surface->current.height;
       }

       /* If texture has a viewport, make sure we scale the size of the texture
        * based on the ratio of the real dims to the viewport dims. The viewport
        * dims are already calculated for and stored in the geometry argument of
        * this function.
        */
       if (tex.has_viewport) {
          float scale_factor_x = (float)width / (float)geo.width;
          float scale_factor_y = (float)height / (float)geo.height;

          geo.width = geo.width*scale_factor_x;
          geo.height = geo.height*scale_factor_y;
       }

       if (tex.texture)
          render_transformed_texture(tex.texture, geo,
				     mat,
                                     color);
        else if (tex.surface)
          {
             auto texture = wlr_surface_get_texture(tex.surface);
             if (texture)
               render_transformed_texture(texture, geo,
                                          mat,
                                          color);
          }
     }

   void render_texture(struct wlr_texture *texture, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color)
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render Texture %p Framebuffer", texture); */

       float mat[9];
       framebuffer.get_orthographic_projection(mat);

       render_transformed_texture(texture, geometry,
				  mat,
				  color);
     }

   void render_transformed_texture(struct wlr_texture *tex, const gl_geometry& g, const gl_geometry& texg, float transform[9], glm::vec4 color, enum wl_output_transform rotation)
     {
        float mat[9];

        /* wlr_log(WLR_DEBUG, "Pixman Render Transformed Texture %p with gl_geometry", */
        /*         tex); */
        /* wlr_log(WLR_DEBUG, "\tgl_geometry: %9.6f %9.6f %9.6f %9.6f", */
        /*         g.x1, g.y1, g.x2, g.y2); */

        if (!tex) return;
        auto renderer = wf::get_core().renderer;
        // auto output = wf::get_core().get_active_output();
        const struct wlr_box wbox =
          {
             .x = (int)g.x1,
             .y = (int)g.y1,
             .width = (int)g.x2 - (int)g.x1,
             .height = (int)g.y2 -(int)g.y1,
          };

	// glm::mat3 m = glm::mat3(transform);
	// float *fm = glm::value_ptr(m);

	// wlr_matrix_translate(fm, 1280/2.0, 720/2.0);
	// wlr_matrix_scale(fm, 1280/2.0, 0-720/2.0);
	// fm[8]=1;

        // wlr_matrix_project_box(mat, &wbox, WL_OUTPUT_TRANSFORM_NORMAL, 0,
        //                        output->handle->transform_matrix);
        // wlr_matrix_project_box(mat, &wbox, WL_OUTPUT_TRANSFORM_NORMAL, 0,
        //                        fm);
        wlr_matrix_project_box(mat, &wbox, rotation, 0,
                               transform);
        wlr_render_texture_with_matrix(renderer, tex, mat, (float)color.a);
        // wlr_render_texture_with_matrix(renderer, tex, transform, (float)color.a);
     }

  void render_transformed_texture(struct wlr_texture *tex, const wf::geometry_t& geometry, float transform[9], glm::vec4 color)
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
        gg.x2 = geometry.x + geometry.width;
        gg.y2 = geometry.y + geometry.height;

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

        /* for resizing the buffer if it does not match old size */
        if ((fb->viewport_width != 0) || (fb->viewport_height != 0))
          {
             if ((width != fb->viewport_width) || (height != fb->viewport_height))
               {
                  if (fb->texture)
                    {
                       wlr_texture_destroy(fb->texture);
                       fb->texture = NULL;
                    }
                  if (fb->buffer)
                    {
                       wlr_buffer_drop(fb->buffer);
                       fb->buffer = NULL;
                    }
               }
          }

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

        if (fb->buffer != Pixman::current_output_fb)
          {
             if (first_allocate || (width != fb->viewport_width) ||
                 (height != fb->viewport_height))
               {
                  is_resize = true;
               }
          }

        if (!fb->texture)
          fb->texture = wlr_texture_from_buffer(renderer, fb->buffer);

        return is_resize || first_allocate;
     }

   void fb_blit(const wf::framebuffer_base_t& src, const wf::framebuffer_base_t& dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, const float zoom)
     {
        auto renderer = wf::get_core().renderer;
        float in[9], out[9];
        /* We need to flip the y coordinate to match pixman coordinate system */
        float new_y = dh - (sy + sh);
        float scale_factor_x = (float)dw / (float)sw;
        float scale_factor_y = (float)dh / (float)sh;

        wlr_matrix_identity(in);
        wlr_matrix_identity(out);

        struct wlr_box blit_box = {-sx*scale_factor_x, -new_y*scale_factor_y,
                                    dw*scale_factor_x, dh*scale_factor_y};

        wlr_matrix_project_box(out, &blit_box, WL_OUTPUT_TRANSFORM_NORMAL, 0.0f,
                               in);
        wlr_render_texture_with_matrix(renderer, src.texture, out, 1.0f);
     }
}
