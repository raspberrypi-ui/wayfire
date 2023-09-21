#include <wayfire/util/log.hpp>
#include <map>
#include "opengl-priv.hpp"
#include "wayfire/texture.hpp"
#include "wayfire/output.hpp"
#include "core-impl.hpp"
#include "config.h"
#include <wayfire/nonstd/wlroots-full.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../main.hpp"

namespace wf
{
   wf::texture_t::texture_t()
     {}
   wf::texture_t::texture_t(uint32_t tex) //(GLuint tex)
     {
        this->tex_id = tex;
     }

   wf::texture_t::texture_t(wlr_texture *texture)
     {
        bool has_alpha = false;

        this->texture = texture;

       assert(wlr_texture_is_gles2(texture));
       wlr_gles2_texture_attribs attribs;
       wlr_gles2_texture_get_attribs(texture, &attribs);
       this->target   = attribs.target;
       this->tex_id   = attribs.tex;
       has_alpha = attribs.has_alpha;

        /* Wayfire works in inverted Y while wlroots doesn't, so we do invert here */
        this->invert_y = true;

        if (this->target == GL_TEXTURE_2D)
          {
             this->type = has_alpha ?
               wf::TEXTURE_TYPE_RGBA : wf::TEXTURE_TYPE_RGBX;
          } else
          {
             this->type = wf::TEXTURE_TYPE_EXTERNAL;
          }
     }

   wf::texture_t::texture_t(wlr_surface *surface) :
     texture_t(surface->buffer->texture)
       {
          this->surface = surface;

          if (surface->current.viewport.has_src)
            {
               this->has_viewport = true;

               auto width  = surface->buffer->texture->width;
               auto height = surface->buffer->texture->height;

               wlr_fbox fbox;
               wlr_surface_get_buffer_source_box(surface, &fbox);
               viewport_box.x1 = fbox.x / width;
               viewport_box.x2 = (fbox.x + fbox.width) / width;
               viewport_box.y1 = 1.0 - (fbox.y + fbox.height) / height;
               viewport_box.y2 = 1.0 - (fbox.y) / height;
            }
       }
}
