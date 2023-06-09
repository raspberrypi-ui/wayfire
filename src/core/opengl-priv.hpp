#ifndef WF_OPENGL_PRIV_HPP
#define WF_OPENGL_PRIV_HPP

#include <wayfire/opengl.hpp>
#include <wayfire/output.hpp>
#include <wayfire/framebuffer.hpp>

namespace OpenGL
{
/** Initialize OpenGL helper functions */
void init();
/** Destroy the default GL program and resources */
void fini();
/** Indicate we have started repainting the given output */
void bind_output(uint32_t fb);
/** Indicate the output frame has been finished */
void unbind_output();
   bool fb_alloc(wf::framebuffer_base_t *fb, int width, int height);
}

#endif /* end of include guard: WF_OPENGL_PRIV_HPP */
