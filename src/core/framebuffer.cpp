#include <wayfire/util/log.hpp>
#include <map>
#include <sys/mman.h>
#include "opengl-priv.hpp"
#include "wayfire/output.hpp"
#include "core-impl.hpp"
#include "config.h"
#include <wayfire/nonstd/wlroots-full.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace wf
{
   bool wf::framebuffer_base_t::allocate(int width, int height)
     {
        bool ret = false;

        ret = OpenGL::fb_alloc(this, width, height);

        viewport_width  = width;
        viewport_height = height;

        return ret;
     }

   void wf::framebuffer_base_t::copy_state(wf::framebuffer_base_t&& other)
     {
        this->viewport_width  = other.viewport_width;
        this->viewport_height = other.viewport_height;

        this->buffer = other.buffer;

        this->fb  = other.fb;
        this->tex = other.tex;

        other.reset();
     }

   wf::framebuffer_base_t::framebuffer_base_t(wf::framebuffer_base_t&& other)
     {
        copy_state(std::move(other));
     }

   wf::framebuffer_base_t& wf::framebuffer_base_t::operator =(
    wf::framebuffer_base_t&& other)
     {
        if (this == &other)
          {
             return *this;
          }

        release();
        copy_state(std::move(other));

        return *this;
     }

   void wf::framebuffer_base_t::bind() const
     {
        GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb));
        GL_CALL(glViewport(0, 0, viewport_width, viewport_height));
     }

   void wf::framebuffer_base_t::scissor(wlr_box box) const
     {
     GL_CALL(glEnable(GL_SCISSOR_TEST));
     GL_CALL(glScissor(box.x, viewport_height - box.y - box.height,
                               box.width, box.height));
     }

   void wf::framebuffer_base_t::release()
     {
     if ((fb != uint32_t(-1)) && (fb != 0))
       {
          GL_CALL(glDeleteFramebuffers(1, &fb));
       }

     if ((tex != uint32_t(-1)) && ((fb != 0) || (tex != 0)))
       {
          GL_CALL(glDeleteTextures(1, &tex));
       }

        reset();
     }

   void wf::framebuffer_base_t::reset()
     {
        wlr_log(WLR_DEBUG, "Framebuffer Reset %p", this);
        buffer = NULL;

        fb  = -1;
        tex = -1;
        viewport_width = viewport_height = 0;
     }

   wlr_box wf::framebuffer_t::framebuffer_box_from_geometry_box(wlr_box box) const
     {
        /* Step 1: Make relative to the framebuffer */
        box.x -= this->geometry.x;
        box.y -= this->geometry.y;

        /* Step 2: Apply scale to box */
        wlr_box scaled = box * scale;

        /* Step 3: rotate */
        if (has_nonstandard_transform)
          {
             // TODO: unimplemented, but also unused for now
             LOGE("unimplemented reached: framebuffer_box_from_geometry_box"
                  " with has_nonstandard_transform");

             return {0, 0, 0, 0};
          }

        int width = viewport_width, height = viewport_height;
        if (wl_transform & 1)
          {
             std::swap(width, height);
          }

        wlr_box result;
        wl_output_transform transform =
          wlr_output_transform_invert((wl_output_transform)wl_transform);

        wlr_box_transform(&result, &scaled, transform, width, height);

        return result;
     }

   glm::mat4 wf::framebuffer_t::get_orthographic_projection() const
     {
        auto ortho = glm::ortho(1.0f * geometry.x,
                                1.0f * geometry.x + 1.0f * geometry.width,
                                1.0f * geometry.y + 1.0f * geometry.height,
                                1.0f * geometry.y);

        return this->transform * ortho;
     }

   void wf::framebuffer_t::logic_scissor(wlr_box box) const
     {
        scissor(framebuffer_box_from_geometry_box(box));
     }

} // end namespace wf
