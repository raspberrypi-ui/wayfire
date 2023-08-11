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
        mat[0] = matrix[0][0];
        mat[1] = matrix[1][0];
        mat[2] = matrix[3][0];
        mat[3] = matrix[0][1];
        mat[4] = matrix[1][1];
        mat[5] = matrix[3][1];
        mat[6] = matrix[0][2];
        mat[7] = matrix[1][2];
        mat[8] = 1.0f;

        wlr_render_rect(renderer, &wbox, c, mat);
     }

   void render_texture(wf::texture_t tex, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color)
     {
       /* wlr_log(WLR_DEBUG, "Pixman Render WF::Texture %p Framebuffer", tex); */
       /* wlr_log(WLR_DEBUG, "\tGeometry: %d %d %d %d", */
       /*          geometry.x, geometry.y, geometry.width, geometry.height); */

       float mat[9];
       framebuffer.get_orthographic_projection(mat);

       if (tex.texture)
          render_transformed_texture(tex.texture, framebuffer, geometry,
                                     mat,
                                     color);
        else if (tex.surface)
          {
             auto texture = wlr_surface_get_texture(tex.surface);
             if (texture)
               render_transformed_texture(texture, framebuffer, geometry,
                                          mat,
                                          color);
          }
     }

   void render_texture(struct wlr_texture *texture, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color)
     {
        /* wlr_log(WLR_DEBUG, "Pixman Render Texture %p Framebuffer", texture); */

       float mat[9];
       framebuffer.get_orthographic_projection(mat);

       render_transformed_texture(texture, framebuffer, geometry,
                                  mat,
                                  color);
     }

   void render_transformed_texture(struct wlr_texture *tex, const wf::framebuffer_t& framebuffer, const gl_geometry& g, const gl_geometry& texg, float transform[9], glm::vec4 color, float angle)
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

	/* Adapt the op_src_area to framebuffer geometry */
	struct wlr_box src_area = { 0 };
	wlr_pixman_texture_get_src_op_area(tex, &src_area);
	if (!wlr_box_empty(&src_area)) {
	    struct wlr_box fb_box = framebuffer.framebuffer_box_from_geometry_box(src_area);
	    wlr_pixman_texture_set_src_op_area(tex, &fb_box);
	}

        wlr_matrix_project_box(mat, &wbox, WL_OUTPUT_TRANSFORM_NORMAL, angle,
                               transform);
        wlr_render_texture_with_matrix(renderer, tex, mat, (float)color.a);
        // wlr_render_texture_with_matrix(renderer, tex, transform, (float)color.a);
     }

  void render_transformed_texture(struct wlr_texture *tex, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, float transform[9], glm::vec4 color, float angle)
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

        render_transformed_texture(tex, framebuffer, gg, {}, transform, color, angle);
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

        if (!fb->texture)
          fb->texture = wlr_texture_from_buffer(renderer, fb->buffer);

        if (fb->buffer != Pixman::current_output_fb)
          {
             if (first_allocate || (width != fb->viewport_width) ||
                 (height != fb->viewport_height))
               {
                  is_resize = true;

                  /* if the fb is getting resized, destroy old texture */
                  if (fb->texture)
                    {
                       wlr_texture_destroy(fb->texture);
                       fb->texture = NULL;
                    }

                  /* recreate texture on resize */
                  fb->texture = wlr_texture_from_buffer(renderer, fb->buffer);
               }
          }

        return is_resize || first_allocate;
     }

   void _clip_right_or_top(int *srcX0, int *srcX1, int *dstX0, int *dstX1, int max, const float zoom)
     {
        float t, b;

        if (*dstX1 > max)
          {
             t = (float)(max - *dstX0) / (float)(*dstX1 - *dstX0);
             *dstX1 = max;
             b = (*srcX0 < *srcX1) ? zoom : -zoom;//0.5F : -0.5F;
             *srcX1 = *srcX0 + (int) (t * (*srcX1 - *srcX0) + b);
          }
        else if (*dstX0 > max)
          {
             t = (float) (max - *dstX1) / (float) (*dstX0 - *dstX1);
             *dstX0 = max;
             b = (*srcX0 < *srcX1) ? -zoom : zoom;//-0.5F : 0.5F;
             *srcX0 = *srcX1 + (int) (t * (*srcX0 - *srcX1) + b);
          }
     }

   void _clip_left_or_bottom(int *srcX0, int *srcX1, int *dstX0, int *dstX1, int min, const float zoom)
     {
        float t, b;

        if (*dstX0 < min)
          {
             t = (float) (min - *dstX0) / (float) (*dstX1 - *dstX0);
             *dstX0 = min;
             b = (*srcX0 < *srcX1) ? zoom : -zoom;//0.5F : -0.5F;
             *srcX0 = *srcX0 + (int) (t * (*srcX1 - *srcX0) + b);
          }
        else if (*dstX1 < min)
          {
             t = (float) (min - *dstX1) / (float) (*dstX0 - *dstX1);
             *dstX1 = min;
             b = (*srcX0 < *srcX1) ? -zoom : zoom;//-0.5F : 0.5F;
             *srcX1 = *srcX1 + (int) (t * (*srcX0 - *srcX1) + b);
          }
     }

   bool _clip_blit(const wf::framebuffer_base_t &src, const wf::framebuffer_base_t &dst,
                   int *srcX0, int *srcY0, int *srcX1, int *srcY1,
                   int *dstX0, int *dstY0, int *dstX1, int *dstY1,
                   const float zoom)
     {
        const int srcXmin = 0;
        const int srcXmax = src.viewport_width;
        const int srcYmin = 0;
        const int srcYmax = src.viewport_height;
        const int dstXmin = 0;
        const int dstXmax = dst.viewport_width;
        const int dstYmin = 0;
        const int dstYmax = dst.viewport_height;

        if (*dstX0 == *dstX1) return false;
        if (*dstX0 <= dstXmin && *dstX1 <= dstXmin) return false;
        if (*dstX0 >= dstXmax && *dstX1 >= dstXmax) return false;

        if (*dstY0 == *dstY1) return false;
        if (*dstY0 <= dstYmin && *dstY1 <= dstYmin) return false;
        if (*dstY0 >= dstYmax && *dstY1 >= dstYmax) return false;

        if (*srcX0 == *srcX1) return false;
        if (*srcX0 <= srcXmin && *srcX1 <= srcXmin) return false;
        if (*srcX0 >= srcXmax && *srcX1 >= srcXmax) return false;

        if (*srcY0 == *srcY1) return false;
        if (*srcY0 <= srcYmin && *srcY1 <= srcYmin) return false;
        if (*srcY0 >= srcYmax && *srcY1 >= srcYmax) return false;

        _clip_right_or_top(srcX0, srcX1, dstX0, dstX1, dstXmax, zoom);
        _clip_right_or_top(srcY0, srcY1, dstY0, dstY1, dstYmax, zoom);
        _clip_left_or_bottom(srcX0, srcX1, dstX0, dstX1, dstXmin, zoom);
        _clip_left_or_bottom(srcY0, srcY1, dstY0, dstY1, dstYmin, zoom);

        _clip_right_or_top(dstX0, dstX1, srcX0, srcX1, srcXmax, zoom);
        _clip_right_or_top(dstY0, dstY1, srcY0, srcY1, srcYmax, zoom);
        _clip_left_or_bottom(dstX0, dstX1, srcX0, srcX1, srcXmin, zoom);
        _clip_left_or_bottom(dstY0, dstY1, srcY0, srcY1, srcYmin, zoom);

        return true;
     }

   void fb_blit(const wf::framebuffer_base_t& src, const wf::framebuffer_base_t& dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, const float zoom)
     {
        void *sdata, *ddata;
        uint32_t sformat, dformat;
        size_t sstride, dstride;
        struct
          {
             int sx, sy, sw, sh;
             int dx, dy, dw, dh;
          } clip;

        wlr_log(WLR_DEBUG, "Pixman FB Blit");
        wlr_log(WLR_DEBUG, "\tSrc Buffer: %p", src.buffer);
        wlr_log(WLR_DEBUG, "\tSrc: %d %d %d %d", sx, sy, sw, sh);
        wlr_log(WLR_DEBUG, "\tDest Buffer: %p", dst.buffer);
        wlr_log(WLR_DEBUG, "\tDest: %d %d %d %d", dx, dy, dw, dh);

        clip.sx = sx;
        clip.sy = sy;
        clip.sw = sw;
        clip.sh = sh;
        clip.dx = dx;
        clip.dy = dy;
        clip.dw = dw;
        clip.dh = dh;

        if (!_clip_blit(src, dst, &clip.sx, &clip.sy, &clip.sw, &clip.sh,
                        &clip.dx, &clip.dy, &clip.dw, &clip.dh, zoom))
          {
             wlr_log(WLR_ERROR, "\tNO CLIP BLIT !!");
             return;
          }

        bool s;
        s =
          (dx != clip.dx) ||
          (dy != clip.dy) ||
          (dw != clip.dw) ||
          (dh != clip.dh);

        wlr_log(WLR_DEBUG, "\tEnable Scissor: %s",
                (s == true) ? "True" : "False");
        wlr_log(WLR_DEBUG, "\tClip Src: %d %d %d %d",
                clip.sx, clip.sy, clip.sw, clip.sh);
        wlr_log(WLR_DEBUG, "\tClip Dest: %d %d %d %d",
                clip.dx, clip.dy, clip.dw, clip.dh);

        auto scissor_minX = std::min(clip.dx, clip.dw);
        auto scissor_minY = std::min(clip.dy, clip.dh);
        auto scissor_maxX = std::max(clip.dx, clip.dw);
        auto scissor_maxY = std::max(clip.dy, clip.dh);

        wlr_log(WLR_DEBUG, "\tScissor: %d %d %d %d",
                scissor_minX, scissor_minY, scissor_maxX, scissor_maxY);

        struct wlr_box blit_src;
        struct wlr_box blit_dst;

        /* FIXME: THIS COULD BE SOURCE OF CORRUPTED IMAGE ! */
        if (dx < dw)
          {
             blit_dst.x = dx;
             blit_src.x = sx;
             blit_dst.width = dw - dx;
             blit_src.width = sw - sx;
          }
        else
          {
             blit_dst.x = dw;
             blit_src.x = sw;
             blit_dst.width = dx - dw;
             blit_src.width = sx - sw;
          }

        if (dy < dh)
          {
             blit_dst.y = dy;
             blit_src.y = sy;
             blit_dst.height = dh - dy;
             blit_src.height = sh - sy;
          }
        else
          {
             blit_dst.y = dh;
             blit_src.y = sh;
             blit_dst.height = dy - dh;
             blit_src.height = sy - sh;
          }

        wlr_log(WLR_DEBUG, "\tBlit Src: %d %d %d %d",
                blit_src.x, blit_src.y, blit_src.width, blit_src.height);
        wlr_log(WLR_DEBUG, "\tBlit Dest: %d %d %d %d",
                blit_dst.x, blit_dst.y, blit_dst.width, blit_dst.height);

        if (!wlr_buffer_begin_data_ptr_access(src.buffer,
                                              WLR_BUFFER_DATA_PTR_ACCESS_READ,
                                              &sdata, &sformat, &sstride))
          {
             wlr_log(WLR_ERROR, "\tCannot access source buffer data ptr");
             return;
          }

        wlr_log(WLR_DEBUG, "\tSource Stride: %d", sstride);
        wlr_log(WLR_DEBUG, "\tSource Test: %d", blit_src.width * sizeof(uint32_t));

        if (!wlr_buffer_begin_data_ptr_access(dst.buffer,
                                              WLR_BUFFER_DATA_PTR_ACCESS_WRITE,
                                              &ddata, &dformat, &dstride))
          {
             wlr_log(WLR_ERROR, "\tCannot access destination buffer data ptr");
             return;
          }

        wlr_log(WLR_DEBUG, "\tDest Stride: %d", dstride);
        wlr_log(WLR_DEBUG, "\tDest Test: %d", blit_dst.width * sizeof(uint32_t));

        /* FIXME: INCORRECT STRIDE USED HERE */
        int y = 0;
        uint32_t *src_data, *dst_data;
        for (; y < blit_dst.height; y++)
          {
             src_data = (uint32_t*)sdata + ((y + blit_src.y) * blit_src.width) + blit_src.x;
             dst_data = (uint32_t*)ddata + ((y + blit_dst.y) * blit_dst.width) + blit_dst.x;
             memcpy(dst_data, src_data, blit_dst.width * sizeof(uint32_t));
          }

        wlr_buffer_end_data_ptr_access(dst.buffer);
        wlr_buffer_end_data_ptr_access(src.buffer);

#if 0
        auto renderer = wf::get_core().renderer;
        if (!wlr_renderer_begin_with_buffer(renderer, src.buffer))
          {
             wlr_log(WLR_ERROR, "\tCannot begin with src buffer");
             wlr_buffer_end_data_ptr_access(dst.buffer);
             return;
          }

        bool ok = false;
        ok = wlr_renderer_read_pixels(renderer, dformat, dstride,dw, dh,
                                      sx, sy, dx, dy, ddata);
        wlr_renderer_end(renderer);

        wlr_buffer_end_data_ptr_access(dst.buffer);
#endif
     }

  void set_src_op_area(struct wlr_texture *tex, struct wlr_box *src_area)
  {
    wlr_pixman_texture_set_src_op_area(tex, src_area);
  }

  void get_src_op_area(struct wlr_texture *tex, struct wlr_box *src_area)
  {
    wlr_pixman_texture_get_src_op_area(tex, src_area);
  }
}
