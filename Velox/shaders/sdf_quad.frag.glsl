#version 460 core

layout(location=0) in vec4 color;
layout(location=1) in vec2 uv;

layout(location=0) out vec4 frag_color;

uniform sampler2D texture_sampler;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 tex = texture(texture_sampler, uv).rgb;
    float sdf = median(tex.r, tex.g, tex.b);

    float range = 1.0; // smaller = sharper
    float screenPxDistance = fwidth(sdf) * range;

    float alpha = smoothstep(0.5 - screenPxDistance, 0.5 + screenPxDistance, sdf);
    frag_color = vec4(color.rgb, color.a * alpha);
}
