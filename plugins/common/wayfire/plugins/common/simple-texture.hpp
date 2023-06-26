#pragma once
#include <wayfire/opengl.hpp>
#include <wayfire/pixman.hpp>
#include <wayfire/nonstd/noncopyable.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>
#include <cairo.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <wayfire/util/log.hpp>

namespace wf
{
struct simple_texture_t : public noncopyable_t
{
    GLuint tex = -1;
    int width  = 0;
    int height = 0;
    struct wlr_buffer *buffer = nullptr;
    struct wlr_texture *texture = nullptr;

    /**
     * Destroy the GL texture.
     * This will call OpenGL::render_begin()/end() internally.
     */
    void release()
    {
       /* NB: We have to use the getenv version of this test as various
        * plugins include this file and trying to include ../main.hpp
        * causes compile errors for those plugins */
       if (!getenv("WAYFIRE_USE_PIXMAN"))
         {
            if (this->tex == (GLuint) - 1)
              {
                 return;
              }
            OpenGL::render_begin();
            GL_CALL(glDeleteTextures(1, &tex));
            OpenGL::render_end();
         }
       else
         {
            if (this->buffer == nullptr)
              return;
            wlr_buffer_drop(this->buffer);
            /* TODO: Drop texture ?? */
         }

        this->tex = -1;
        this->buffer = nullptr;
        this->texture = nullptr;
    }

    /** Auto-release the texture when the object is destroyed */
    ~simple_texture_t()
    {
        release();
    }
};
}
