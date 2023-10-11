#include <wayfire/pixman.hpp>
#include <wayfire/nonstd/wlroots-full.hpp>
#include <drm_fourcc.h>

#include "deco-shadow.hpp"
#include "../main.hpp"

static glm::vec4 erf(glm::vec4 x) {
    glm::vec4 s = glm::sign(x), a = glm::abs(x);
    x = glm::vec4(1.0f) + (glm::vec4(0.278393f) + (glm::vec4(0.230389f) + glm::vec4(0.078108f) * (a * a)) * a) * a;
    x *= x;
    return s - s / (x * x);
}

static float box_shadow(glm::vec2 lower, glm::vec2 upper, glm::vec2 point, float sigma) {
    glm::vec4 query = glm::vec4(lower - point, upper - point);
    glm::vec4 integral = 0.5f + 0.5f * erf(query * (sqrt(0.5f) / sigma));
    return (integral.z - integral.x) * (integral.w - integral.y);
}

static uint32_t vec4_to_bgr(glm::vec4 col) {
    uint32_t r = (uint32_t)(col.r * 255) & 0xFF;
    uint32_t g = (uint32_t)(col.g * 255) & 0xFF;
    uint32_t b = (uint32_t)(col.b * 255) & 0xFF;
    uint32_t a = (uint32_t)(col.a * 255) & 0xFF;
    return (b << 0) | (g << 8) | (r << 16) | (a << 24);
}

void wf::winshadows::decoration_shadow_t::generate_shadow_texture(wf::point_t window_origin, bool glow) {
    auto renderer = wf::get_core().renderer;
    auto formats = wlr_renderer_get_render_formats(renderer);
    auto format = wlr_drm_format_set_get(formats, DRM_FORMAT_ARGB8888);

    bool use_glow = (glow && is_glow_enabled());

    float radius = shadow_radius_option;
    wf::color_t color = shadow_color_option;

    // Premultiply alpha for shader
    glm::vec4 premultiplied = {
        color.r * color.a,
        color.g * color.a,
        color.b * color.a,
        color.a
    };
    wf::color_t glow_color = glow_color_option;
    glm::vec4 glow_premultiplied = {
        glow_color.r * glow_color.a,
        glow_color.g * glow_color.a,
        glow_color.b * glow_color.a,
        glow_color.a * (1.0 - glow_emissivity_option)
    };
    wf::geometry_t bounds = outer_geometry + window_origin;

    size_t width = bounds.width;
    size_t height = bounds.height;

    size_t narrow_width = (width - window_geometry.width) / 2;
    size_t narrow_height = (height - window_geometry.height) / 2;

    if (shadow_image[0] == NULL) {
        shadow_image[0] = (uint32_t*)malloc(width*narrow_height*sizeof(uint32_t));
        shadow_image[1] = (uint32_t*)malloc(width*narrow_height*sizeof(uint32_t));
        shadow_image[2] = (uint32_t*)malloc(narrow_width*window_geometry.height*sizeof(uint32_t));
        shadow_image[3] = (uint32_t*)malloc(narrow_width*window_geometry.height*sizeof(uint32_t));
    } else {
        shadow_image[0] = (uint32_t*)realloc(shadow_image[0], width*narrow_height*sizeof(uint32_t));
        shadow_image[1] = (uint32_t*)realloc(shadow_image[1], width*narrow_height*sizeof(uint32_t));
        shadow_image[2] = (uint32_t*)realloc(shadow_image[2], narrow_width*window_geometry.height*sizeof(uint32_t));
        shadow_image[3] = (uint32_t*)realloc(shadow_image[3], narrow_width*window_geometry.height*sizeof(uint32_t));
    }

    float inner_x = window_geometry.x + window_origin.x;
    float inner_y = window_geometry.y + window_origin.y;
    float inner_w = window_geometry.width;
    float inner_h = window_geometry.height;
    float shadow_x = inner_x + horizontal_offset;
    float shadow_y = inner_y + vertical_offset;

    float glow_sigma = glow_radius_option / 3.0f;
    glm::vec2 glow_lower = {inner_x, inner_y};
    glm::vec2 glow_upper = {inner_x + inner_w, inner_y + inner_h};

    float sigma = radius / 3.0f;
    glm::vec2 lower = {shadow_x, shadow_y};
    glm::vec2 upper = {shadow_x + inner_w, shadow_y + inner_h};


    /* Generate the top left shadow of the window, and mirror it to compute the top right part.
     *
     * And mirror the full top shadow to compute the full bottom shadow.
     *
     * When computing the top left shadow, instead of fully computing
     * it, just compute the corner plus a small part, and then
     * replicate horizontally the rest of the shadow
     */
    size_t l_width = narrow_width + radius;
    for(size_t y = 0; y < narrow_height; y++) {
      for(size_t x = 0; x < (width + 1)/2; x++) {
	if (x > l_width) {
	  shadow_image[0][y*width + x] = shadow_image[0][y*width + l_width];
	} else {
	  glm::vec2 point{(float)x + bounds.x, (float)y + bounds.y};
	  glm::vec4 out = premultiplied * box_shadow(lower, upper, point, sigma);
	  if (use_glow)
	    {
	      out += glow_premultiplied * box_shadow(glow_lower, glow_upper, point, glow_sigma);
	    }
	  shadow_image[0][y*width + x] = vec4_to_bgr(out);
	}
	shadow_image[0][y*width + (width - x - 1)] = shadow_image[0][y*width + x];
	shadow_image[1][(narrow_height - y - 1)*width + x] = shadow_image[0][y*width + x];
	shadow_image[1][(narrow_height - y - 1)*width + (width - x - 1)] = shadow_image[0][y*width + x];
      }
    }

    /* Like the above step, but now for the left and right shadows of
       the window */
    size_t l_height = narrow_height + radius;
    for(size_t y = 0; y < (size_t)(window_geometry.height + 1)/2; y++) {
      for(size_t x = 0; x < narrow_width; x++) {
	if (y > l_height) {
	  shadow_image[2][y*narrow_width + x] = shadow_image[2][l_height*narrow_width + x];
	} else {
	  glm::vec2 point{(float)x + bounds.x, (float)y + narrow_height + bounds.y};
	  glm::vec4 out = premultiplied * box_shadow(lower, upper, point, sigma);
	  if (use_glow)
	    {
	      out += glow_premultiplied * box_shadow(glow_lower, glow_upper, point, glow_sigma);
	    }
	  shadow_image[2][y*narrow_width + x] = vec4_to_bgr(out);
	}
	shadow_image[2][(window_geometry.height - y - 1)*narrow_width + x] = shadow_image[2][y*narrow_width + x];
	shadow_image[3][y*narrow_width + (narrow_width - x - 1)] = shadow_image[2][y*narrow_width + x];
	shadow_image[3][(window_geometry.height - y - 1)*narrow_width + (narrow_width - x - 1)] = shadow_image[2][y*narrow_width + x];
      }
    }

    if (shadow_texture[0] != NULL) {
        wlr_texture_destroy(shadow_texture[0]);
        wlr_texture_destroy(shadow_texture[1]);
        wlr_texture_destroy(shadow_texture[2]);
        wlr_texture_destroy(shadow_texture[3]);
    }
    shadow_texture[0] = wlr_texture_from_pixels(renderer, format[0].format, width*sizeof(uint32_t),
                                                width, narrow_height, shadow_image[0]);
    shadow_texture[1] = wlr_texture_from_pixels(renderer, format[0].format, width*sizeof(uint32_t),
                                                width, narrow_height, shadow_image[1]);
    shadow_texture[2] = wlr_texture_from_pixels(renderer, format[0].format, narrow_width*sizeof(uint32_t),
                                                narrow_width, window_geometry.height, shadow_image[2]);
    shadow_texture[3] = wlr_texture_from_pixels(renderer, format[0].format, narrow_width*sizeof(uint32_t),
                                                narrow_width, window_geometry.height, shadow_image[3]);

    cached_geometry = outer_geometry;
    cached_glow = use_glow;
}

wf::winshadows::decoration_shadow_t::decoration_shadow_t() {
    if (!runtime_config.use_pixman) {
        OpenGL::render_begin();
        shadow_program.set_simple(
            OpenGL::compile_program(shadow_vert_shader, shadow_frag_shader)
        );
        shadow_glow_program.set_simple(
            OpenGL::compile_program(shadow_vert_shader, shadow_glow_frag_shader)
        );
        OpenGL::render_end();
    } else {
        shadow_image[0] = NULL;
        shadow_texture[0] = NULL;
    }
}

wf::winshadows::decoration_shadow_t::~decoration_shadow_t() {
    if (!runtime_config.use_pixman) {
        OpenGL::render_begin();
        shadow_program.free_resources();
        shadow_glow_program.free_resources();
        OpenGL::render_end();
    }
}

void wf::winshadows::decoration_shadow_t::render(const framebuffer_t& fb, wf::point_t window_origin, const geometry_t& scissor, const bool glow) {
    float radius = shadow_radius_option;

    wf::color_t color = shadow_color_option;

    // Premultiply alpha for shader
    glm::vec4 premultiplied = {
        color.r * color.a,
        color.g * color.a,
        color.b * color.a,
        color.a
    };

    // Glow color, alpha=0 => additive blending (exploiting premultiplied alpha)
    wf::color_t glow_color = glow_color_option;
    glm::vec4 glow_premultiplied = {
        glow_color.r * glow_color.a,
        glow_color.g * glow_color.a,
        glow_color.b * glow_color.a,
        glow_color.a * (1.0 - glow_emissivity_option)
    };

    bool use_glow = (glow && is_glow_enabled());

    if (!runtime_config.use_pixman) {
        // Enable glow shader only when glow radius > 0 and view is focused
        OpenGL::program_t &program = 
            use_glow ? shadow_glow_program : shadow_program;

        OpenGL::render_begin(fb);
        fb.logic_scissor(scissor);

        program.use(wf::TEXTURE_TYPE_RGBA);

        // Compute vertex rectangle geometry
        wf::geometry_t bounds = outer_geometry + window_origin;
        float left = bounds.x;
        float right = bounds.x + bounds.width;
        float top = bounds.y;
        float bottom = bounds.y + bounds.height;

        GLfloat vertexData[] = {
            left, bottom,
            right, bottom,
            right, top,
            left, top
        };

        glm::mat4 matrix = fb.get_orthographic_projection();

        program.attrib_pointer("position", 2, 0, vertexData);
        program.uniformMatrix4f("MVP", matrix);
        program.uniform1f("sigma", radius / 3.0f);
        program.uniform4f("color", premultiplied);

        float inner_x = window_geometry.x + window_origin.x;
        float inner_y = window_geometry.y + window_origin.y;
        float inner_w = window_geometry.width;
        float inner_h = window_geometry.height;
        float shadow_x = inner_x + horizontal_offset;
        float shadow_y = inner_y + vertical_offset;
        program.uniform2f("lower", shadow_x, shadow_y);
        program.uniform2f("upper", shadow_x + inner_w, shadow_y + inner_h);

        if (use_glow) {
            program.uniform1f("glow_sigma", glow_radius_option / 3.0f);
            program.uniform4f("glow_color", glow_premultiplied);
            program.uniform2f("glow_lower", inner_x, inner_y);
            program.uniform2f("glow_upper", inner_x + inner_w, inner_y + inner_h);
        }

        GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        GL_CALL(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));

        program.deactivate();
        OpenGL::render_end();
    } else {
        if (shadow_texture[0] == NULL || cached_geometry != outer_geometry || cached_glow != use_glow) {
            generate_shadow_texture(window_origin, glow);
        }

        wf::geometry_t bounds = outer_geometry + window_origin;
        int width = (int)bounds.width;
        int height = (int)bounds.height;

        int narrow_width = (int)(width - window_geometry.width) / 2;
        int narrow_height = (int)(height - window_geometry.height) / 2;

        wf::geometry_t win_bounds[4] = {
            {bounds.x, bounds.y, width, narrow_height},
            {bounds.x, bounds.y + window_geometry.height + narrow_height, width, narrow_height},
            {bounds.x, bounds.y + narrow_height, narrow_width, window_geometry.height},
            {bounds.x + window_geometry.width + narrow_width, bounds.y + narrow_height, narrow_width, window_geometry.height},
        };


        Pixman::render_begin(fb);
        fb.logic_scissor(scissor);
        Pixman::render_texture(shadow_texture[0], fb, win_bounds[0], glm::vec4(1.f));
        Pixman::render_texture(shadow_texture[1], fb, win_bounds[1], glm::vec4(1.f));
        Pixman::render_texture(shadow_texture[2], fb, win_bounds[2], glm::vec4(1.f));
        Pixman::render_texture(shadow_texture[3], fb, win_bounds[3], glm::vec4(1.f));
        Pixman::render_end();
    }
}

wf::region_t wf::winshadows::decoration_shadow_t::calculate_region() const {
    // TODO: geometry and region depending on whether glow is active or not
    wf::region_t region = wf::region_t(shadow_geometry) | wf::region_t(glow_geometry);

    if (clip_shadow_inside) {
        region ^= window_geometry;
    }

    return region;
}

wf::geometry_t wf::winshadows::decoration_shadow_t::get_geometry() const {
    return outer_geometry;
}

void wf::winshadows::decoration_shadow_t::resize(const int window_width, const int window_height, const bool borders) {
    int bmod = borders ? border_size : 0;
    window_geometry =  {
        bmod,
        bmod,
        window_width - 2 * bmod,
        window_height - 2 * bmod
    };

    shadow_geometry = {
        -shadow_radius_option + horizontal_offset, -shadow_radius_option + vertical_offset,
        window_width + shadow_radius_option * 2, window_height + shadow_radius_option * 2
    };

    glow_geometry = {
        -glow_radius_option, -glow_radius_option,
        window_width + glow_radius_option * 2, window_height + glow_radius_option * 2
    };

    int left = std::min(shadow_geometry.x, glow_geometry.x);
    int top = std::min(shadow_geometry.y, glow_geometry.y);
    int right = std::max(shadow_geometry.x + shadow_geometry.width, glow_geometry.x + glow_geometry.width);
    int bottom = std::max(shadow_geometry.y + shadow_geometry.height, glow_geometry.y + glow_geometry.height);
    outer_geometry = {
        left,
        top,
        right - left,
        bottom - top
    };
}

bool wf::winshadows::decoration_shadow_t::is_glow_enabled() const {
    return glow_radius_option > 0;
}
