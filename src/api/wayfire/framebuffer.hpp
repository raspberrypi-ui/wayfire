#ifndef WF_FRAMEBUFFER_HPP
# define WF_FRAMEBUFFER_HPP

# include <wayfire/config/types.hpp>
# include <wayfire/util.hpp>
# include <wayfire/nonstd/noncopyable.hpp>
# include <wayfire/nonstd/wlroots.hpp>

# include <wayfire/geometry.hpp>

# define GLM_FORCE_RADIANS
# include <glm/mat4x4.hpp>
# include <glm/vec4.hpp>

namespace wf
{
/* Simple framebuffer, used mostly to allocate framebuffers for workspace
 * streams.
 *
 * Resources (tex/fb) are not automatically destroyed */
struct framebuffer_base_t : public noncopyable_t
{
    uint32_t tex = -1, fb = -1; /* GLuint tex = -1, fb = -1; */
    int32_t viewport_width = 0, viewport_height = 0;

    /* used for pixman rendering */
    struct wlr_buffer *buffer = nullptr;
    /* uint32_t *buffer_dest, *texture_dest; */
    /* pixman_image_t *buffer, *texture; */

    framebuffer_base_t() = default;
    framebuffer_base_t(framebuffer_base_t&& other);
    framebuffer_base_t& operator =(framebuffer_base_t&& other);

    /* The functions below assume they are called between
     * OpenGL::render_begin() and OpenGL::render_end() */

    /* will invalidate texture contents if width or height changes.
     * If tex and/or fb haven't been set, it creates them
     * Return true if texture was created/invalidated */
    bool allocate(int width, int height);

    /* Make the framebuffer current, and adjust viewport to its size */
    void bind() const;

    /* Set the GL scissor to the given box, after inverting it to match GL
     * coordinate space */
    void scissor(wlr_box box) const;

    /* Will destroy the texture and framebuffer
     * Warning: will destroy tex/fb even if they have been allocated outside of
     * allocate() */
    void release();

    /* Reset the framebuffer, WITHOUT freeing resources.
     * There is no need to call reset() after release() */
    void reset();

  private:
    void copy_state(framebuffer_base_t&& other);
};

/* A more feature-complete framebuffer.
 * It represents an area of the output, with the corresponding dimensions,
 * transforms, etc */
struct framebuffer_t : public framebuffer_base_t
{
    wf::geometry_t geometry = {0, 0, 0, 0};

    uint32_t wl_transform = WL_OUTPUT_TRANSFORM_NORMAL;
    float scale = 1.0;

    /* Indicates if the framebuffer has other transform than indicated
     * by scale and wl_transform */
    bool has_nonstandard_transform = false;

    /* Transform contains output rotation, and possibly
     * other framebuffer transformations, if has_nonstandard_transform is set */
    glm::mat4 transform = glm::mat4(1.0);

    /* The functions below to convert between coordinate systems don't need a
     * bound OpenGL context */

    /**
     * Get the geometry of the given box after projecting it onto the framebuffer.
     *
     * The resulting geometry is affected by the framebuffer geometry, scale and
     * transform.
     */
    wlr_box framebuffer_box_from_geometry_box(wlr_box box) const;

    /* Returns a matrix which contains an orthographic projection from "geometry"
     * coordinates to the framebuffer coordinates. */
    glm::mat4 get_orthographic_projection() const;

    /**
     * Set the scissor region to the given box.
     *
     * In contrast to wf::framebuffer_base_t, this method takes its argument
     * as a box with "logical" coordinates, not raw framebuffer coordinates.
     *
     * @param box The scissor box, in the same coordinate system as the
     *   framebuffer's geometry.
     */
    void logic_scissor(wlr_box box) const;
};
}

#endif
