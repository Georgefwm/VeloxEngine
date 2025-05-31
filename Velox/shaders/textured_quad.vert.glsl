#version 460

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_uv;

void main()
{
    gl_Position = vec4(in_position, 1.0f);
    out_color   = in_color;
    out_uv      = in_uv;
}
