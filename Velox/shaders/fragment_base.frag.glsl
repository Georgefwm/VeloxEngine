#version 460

layout (location = 0) in vec4 in_color;

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = in_color;
}
