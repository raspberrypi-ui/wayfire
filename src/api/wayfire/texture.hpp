#ifndef WF_TEXTURE_HPP
# define WF_TEXTURE_HPP

# include <wayfire/config/types.hpp>
# include <wayfire/util.hpp>
# include <wayfire/nonstd/noncopyable.hpp>
# include <wayfire/nonstd/wlroots.hpp>

# include <wayfire/geometry.hpp>

# define GLM_FORCE_RADIANS
# include <glm/mat4x4.hpp>
# include <glm/vec4.hpp>

struct gl_geometry
{
   float x1, y1, x2, y2;
};

namespace wf
{
   /** Represents the different types(formats) of textures in Wayfire. */
   enum texture_type_t
     {
        /* Regular OpenGL texture with 4 channels */
        TEXTURE_TYPE_RGBA     = 0,
        /* Regular OpenGL texture with 4 channels, but alpha channel should be
         * discarded. */
        TEXTURE_TYPE_RGBX     = 1,
        /** An EGLImage, it has been shared via dmabuf */
        TEXTURE_TYPE_EXTERNAL = 2,
        /* Invalid */
        TEXTURE_TYPE_ALL      = 3,
     };

   struct texture_t
     {
        /* Texture type */
        texture_type_t type = TEXTURE_TYPE_RGBA;
        /* Texture target */
        int target = 0x0DE1; //GL_TEXTURE_2D; //GLenum target = GL_TEXTURE_2D;
        /* Actual texture ID */
        uint32_t tex_id; //GLuint tex_id;
     
        /** Invert Y? */
        bool invert_y = false;
        /** Has viewport? */
        bool has_viewport = false;

        struct wlr_texture *texture;
        struct wlr_surface *surface;

        /**
         * Part of the texture which is used for rendering.
         * Valid only if has_viewport is true.
         */
        gl_geometry viewport_box;
     
        /* tex_id will be initialized later */
        texture_t();
        /** Initialize a non-inverted RGBA texture with the given texture id */
        texture_t(uint32_t tex); //texture_t(GLuint tex);
        /** Initialize a texture with the attributes of the wlr texture */
        explicit texture_t(wlr_texture*);
        /** Initialize a texture with the attributes of a mapped surface */
        explicit texture_t(wlr_surface*);
     };
}

#endif
