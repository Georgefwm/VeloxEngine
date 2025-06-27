#version 460 core

layout(std140, binding=0) uniform ubo
{
    mat4 u_projection;
    mat4 u_view;
    ivec2 u_resolution;
};

layout(location=0) in vec3  in_position;
layout(location=1) in vec4  in_color;
layout(location=2) in vec2  in_uv;
layout(location=3) in float in_font_weight_bias;
layout(location=4) in vec4  in_outline_color;
layout(location=5) in float in_outline_width;
layout(location=6) in vec4  in_shadow_color;
layout(location=7) in vec2  in_shadow_offset;

layout(location=0) out vec4  out_color;
layout(location=1) out vec2  out_uv;
layout(location=2) out float out_font_weight_bias;
layout(location=3) out vec4  out_outline_color;
layout(location=4) out float out_outline_width;
layout(location=5) out vec4  out_shadow_color;
layout(location=6) out vec2  out_shadow_offset;

void main()
{
    gl_Position = u_projection * u_view * vec4(in_position, 1.0f);

    out_color            = in_color;
    out_uv               = in_uv;
    out_outline_color    = in_outline_color;
    out_outline_width    = in_outline_width;
    out_shadow_color     = in_shadow_color;
    out_shadow_offset    = in_shadow_offset;
    out_font_weight_bias = in_font_weight_bias;
}
