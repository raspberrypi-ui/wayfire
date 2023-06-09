#pragma once
#include <wayfire/opengl.hpp>
#include <wayfire/nonstd/noncopyable.hpp>
#include <stdlib.h>

namespace wf
{
struct simple_texture_t : public noncopyable_t
{
    GLuint tex = -1;
    int width  = 0;
    int height = 0;

    /**
     * Destroy the GL texture.
     * This will call OpenGL::render_begin()/end() internally.
     */
    void release()
    {
        if (this->tex == (GLuint) - 1)
        {
            return;
        }

       /* NB: We have to use the getenv version of this test as various
        * plugins include this file and trying to include ../main.hpp
        * causes compile errors for those plugins */
       if (!getenv("WAYFIRE_USE_PIXMAN"))
         {
            OpenGL::render_begin();
            GL_CALL(glDeleteTextures(1, &tex));
            OpenGL::render_end();
         }

        this->tex = -1;
    }

    /** Auto-release the texture when the object is destroyed */
    ~simple_texture_t()
    {
        release();
    }
};
}
