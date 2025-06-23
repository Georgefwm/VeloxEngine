#version 460 core

layout(location=0) in vec4 color;
layout(location=1) in vec2 uv;

layout(location=0) out vec4 frag_color;

uniform sampler2D texture_sampler;

void main()
{
    vec4 tex = texture(texture_sampler, uv) * color;
    frag_color = vec4(tex.rgb * color.rgb, tex.a * color.a); 
}
