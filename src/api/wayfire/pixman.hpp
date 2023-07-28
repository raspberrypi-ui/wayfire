#ifndef WF_PIXMAN_HPP
# define WF_PIXMAN_HPP

# include <pixman.h>

# include <wayfire/config/types.hpp>
# include <wayfire/util.hpp>
# include <wayfire/nonstd/noncopyable.hpp>
# include <wayfire/nonstd/wlroots.hpp>
# include <wayfire/framebuffer.hpp>
# include <wayfire/texture.hpp>
# include <wayfire/geometry.hpp>

namespace Pixman
{
   void render_begin(); // use to just bind the fb but not draw
   void render_begin(const wf::framebuffer_base_t& fb);
   void render_begin(int32_t viewport_width, int32_t viewport_height);
   void render_begin(int32_t viewport_width, int32_t viewport_height, uint32_t fb);
   void render_begin(struct wlr_buffer *buffer);
   void render_rectangle(wf::geometry_t box, wf::color_t color, glm::mat4 matrix);
   void render_texture(struct wlr_texture *texture, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color = glm::vec4(1.f));
   void render_texture(wf::texture_t texture, const wf::framebuffer_t& framebuffer, const wf::geometry_t& geometry, glm::vec4 color = glm::vec4(1.f));
  void render_transformed_texture(struct wlr_texture *tex, const gl_geometry& g, const gl_geometry& texg, float transform[9], glm::vec4 color = glm::vec4(1.f), float angle = 0.0f);
  void render_transformed_texture(struct wlr_texture *tex, const wf::geometry_t& geometry, float transform[9], glm::vec4 color = glm::vec4(1.f), float angle = 0.0f);
//   void render_transformed_texture(wf::texture_t tex, const gl_geometry& g, const gl_geometry& texg, glm::mat4 transform = glm::mat4(1.0), glm::vec4 color = glm::vec4(1.f));
//   void render_transformed_texture(wf::texture_t tex, const wf::geometry_t& geometry, glm::mat4 transform = glm::mat4(1.0), glm::vec4 color = glm::vec4(1.f));
   void render_end();
   void clear(wf::color_t color);
   void fb_blit(const wf::framebuffer_base_t& src, const wf::framebuffer_base_t& dest, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
}

#endif
