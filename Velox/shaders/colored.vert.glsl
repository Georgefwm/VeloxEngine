#version 460 core

layout(std140, binding=0) uniform ubo
{
    mat4 u_projection;
    mat4 u_view;
    ivec2 u_resolution;
};

layout(location=0) in vec3 in_position;
layout(location=1) in vec4 in_color;

layout(location=0) out vec4 out_color;

void main()
{
    // Convert pixel coords to shader coords.
    vec2 normalised_position = ((in_position.xy / u_resolution.xy) * 2.0) - 1.0;

    normalised_position.y = -normalised_position.y;
    // Leave the z value as is.
    gl_Position = vec4(normalised_position, in_position.z, 1.0f);

    out_color   = in_color;
}
