#version 460

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;
layout(location = 2) in flat int texture_index;

layout(location = 0) out vec4 frag_color;

// layout(set=2, binding=0) uniform sampler2D texture_samplers[1];

void main()
{
    // Index of -1 means no texture should be used (purely color).
    if (texture_index <= -1)
    {
        frag_color = color;
        return;
    }

    // frag_color = texture(texture_samplers[texture_index], uv) * color;
    frag_color = vec4(uv, 1.0, 1.0);
}
