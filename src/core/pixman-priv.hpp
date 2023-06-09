#ifndef WF_PIXMAN_PRIV_HPP
# define WF_PIXMAN_PRIV_HPP

# include <wayfire/pixman.hpp>
# include <wayfire/output.hpp>
# include <wayfire/framebuffer.hpp>
# include <wayfire/texture.hpp>

namespace Pixman
{
   void init();
   void fini();
   void bind_output(struct wlr_buffer *fb);
   void unbind_output();
   bool fb_alloc(wf::framebuffer_base_t *fb, int width, int height);
}

#endif /* end of include guard: WF_PIXMAN_PRIV_HPP */

