#version 460

layout(std140, binding = 0) uniform ubo
{
    mat4 u_projection;
    mat4 u_view;
    ivec2 u_resolution;    // Add resolution as ivec2 (2 ints)
    // Padding for std140 alignment: ivec2 is 2 ints (8 bytes),
    // std140 requires 16-byte alignment, so add padding:
    int padding[2];
} ubo;

layout(location=0) in vec3 in_position;
layout(location=1) in vec4 in_color;
layout(location=2) in vec2 in_uv;
layout(location=3) in int  in_texture_index;

layout(location=0) out vec4 out_color;
layout(location=1) out vec2 out_uv;
layout(location=2) out int  out_texture_index;


void main()
{
    // Convert pixel coords to shader coords.
    vec2 normalised_position = ((in_position.xy / ubo.screen_resolution.xy) * 2.0) - 1.0;
    
    // Leave the z value as is.
    gl_Position = vec4(normalised_position, in_position.z, 1.0f);

    out_color = in_color;
    out_uv = in_uv;
    out_texture_index = in_texture_index;
}
