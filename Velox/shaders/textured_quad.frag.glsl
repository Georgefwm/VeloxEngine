#version 460

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 frag_color;

layout(set=2, binding=0) uniform sampler2D texture_sampler;

void main()
{
    frag_color = texture(texture_sampler, uv) * color;
    // frag_color = vec4(uv, 1.0, 1.0);
}
