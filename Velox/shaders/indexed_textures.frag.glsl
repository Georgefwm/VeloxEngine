#version 460

#extension GL_EXT_nonuniform_qualifier : require

layout(location=0) in vec4 color;
layout(location=1) in vec2 uv;
layout(location=2) in flat int texture_index;

layout(location=0) out vec4 frag_color;

// This has to have a declared size of 1 here for SPIR-V compilation.
// In the renderer it is set as an unbounded array so indices >=1 are valid.
layout(set=0, binding=1) uniform sampler2D texture_samplers[32]; 

void main()
{
    // Index of -1 means no texture should be used (purely color).
    if (texture_index == -1)
    {
        frag_color = color;
        return;
    }

    vec4 tex = texture(texture_samplers[nonuniformEXT(texture_index)], uv) * color;
    frag_color = vec4(tex.rgb * color.rgb, tex.a * color.a); 
}
