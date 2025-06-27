#version 460 core

layout(location=0) in vec4  color;
layout(location=1) in vec2  uv;
layout(location=2) in vec4  outline_color;
layout(location=3) in float outline_width;
layout(location=4) in vec4  shadow_color;
layout(location=5) in vec2  shadow_offset;

layout(location=0) out vec4 frag_color;

uniform sampler2D msdf_texture;

float px_range = 4.0f; // Set in Asset.cpp when loading msdf atlas.

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec4 distances = texture(msdf_texture, uv);
    float sd = median(distances.r, distances.g, distances.b); // Alpha isn't used.

    float screen_px_dist = fwidth(sd) * px_range;

    // Fill alpha.
    float fill_alpha = smoothstep(0.5 - screen_px_dist, 0.5 + screen_px_dist, sd);

    // Outline alpha.
    float outline_alpha = smoothstep(0.5 + outline_width - screen_px_dist, 0.5 + outline_width + screen_px_dist, sd);

    vec2 shadow_uv = uv + shadow_offset / textureSize(msdf_texture, 0); // shift in UV space

    vec3 shadow_sample = texture(msdf_texture, shadow_uv).rgb;
    float shadow_sd = median(shadow_sample.r, shadow_sample.g, shadow_sample.b);
    float shadow_px_dist = fwidth(shadow_sd) * px_range;
    float shadow_alpha = smoothstep(0.5 - shadow_px_dist, 0.5 + shadow_px_dist, shadow_sd);

    // Combine colors (shadow -> outline -> fill)
    vec4 result = mix(shadow_color, outline_color, outline_alpha);
    result = mix(result, color, fill_alpha);

    // Premultiplied alpha
    result.a = max(max(shadow_color.a * shadow_alpha, outline_alpha), fill_alpha) * color.a;

    frag_color = result;

    // float alpha = smoothstep(0.5 - screenPxDistance, 0.5 + screenPxDistance, distance);
    // frag_color = vec4(color.rgb, color.a * alpha);
}
