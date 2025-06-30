#version 460 core

layout(std140, binding=0) uniform ubo
{
    mat4 u_projection;
    mat4 u_view;
    ivec2 u_resolution;
};

layout(location=0) in vec3  in_position;
layout(location=1) in vec4  in_inner_color;
layout(location=2) in vec2  in_uv;
layout(location=3) in float in_threshold;
layout(location=4) in float in_out_bias;
layout(location=5) in vec4  in_outer_color;
layout(location=6) in float in_outline_width_absolute;
layout(location=7) in float in_outline_width_relative;
layout(location=8) in float in_outline_blur;

layout(location=0) out vec4  inner_color;
layout(location=1) out vec2  uv;
layout(location=2) out float threshold;
layout(location=3) out float out_bias;
layout(location=4) out vec4  outer_color;
layout(location=5) out float outline_width_absolute;
layout(location=6) out float outline_width_relative;
layout(location=7) out float outline_blur;

void main()
{
    gl_Position = u_projection * u_view * vec4(in_position, 1.0f);

    inner_color            = in_inner_color;
    uv                     = in_uv;
    threshold              = in_threshold;
    out_bias               = in_out_bias;
    outer_color            = in_outer_color;
    outline_width_absolute = in_outline_width_absolute;
    outline_width_relative = in_outline_width_relative;
    outline_blur           = in_outline_blur;
}
