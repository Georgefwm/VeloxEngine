#version 460 core

layout(std140, binding=0) uniform ubo
{
    mat4 u_projection;
    mat4 u_view;
    ivec2 u_resolution;
};

layout(location=0) in vec3 in_position;
layout(location=1) in vec4 in_color;
layout(location=2) in vec2 in_uv;
layout(location=3) in vec4 in_outline_color;

layout(location=0) out vec4 out_color;
layout(location=1) out vec2 out_uv;
layout(location=2) out vec4 out_outline_color;

void main()
{
    gl_Position = u_projection * u_view * vec4(in_position, 1.0f);

    out_color = in_color;
    out_uv = in_uv;
    out_outline_color = in_outline_color;
}
