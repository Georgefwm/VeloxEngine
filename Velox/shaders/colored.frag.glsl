#version 460 core

layout(location=0) in vec4 color;
layout(location=1) in vec2 uv;

layout(location=0) out vec4 frag_color;

void main()
{
    frag_color = color; 
}
