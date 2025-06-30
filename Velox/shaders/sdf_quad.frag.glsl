#version 460 core

// Very much inspired/taken from a series of RedBlobGames articles.
// https://www.redblobgames.com/x/2403-distance-field-fonts/

layout(location=0) in vec4  inner_color;
layout(location=1) in vec2  uv;
layout(location=2) in float threshold;
layout(location=3) in float out_bias;
layout(location=4) in vec4  outer_color;
layout(location=5) in float outline_width_absolute;
layout(location=6) in float outline_width_relative;
layout(location=7) in float outline_blur;

layout(location=0) out vec4 frag_color;

uniform sampler2D msdf_texture;


float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float smootherstep(float edge0, float edge1, float x)
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float px_range = 4.0f; // Set in Asset.cpp when loading msdf atlas.

float screenPxRange() {
    vec2 screen_tex_size = vec2(1.0) / fwidth(uv);
    return max(0.5 * dot(vec2(px_range), screen_tex_size), 1.0);
}

void main()
{
    // distances are stored with 1.0 meaning "inside" and 0.0 meaning "outside"
    vec4 distances = texture(msdf_texture, uv);
    float d_msdf = median(distances.r, distances.g, distances.b);

    float d_sdf = distances.a; // mtsdf format only
    d_msdf = min(d_msdf, d_sdf + 0.1);  // HACK: to fix glitch in msdf near edges

    // blend between sharp and rounded corners
    float d_inner = d_msdf * d_sdf;
    float d_outer = d_msdf * d_sdf;

    // typically 0.5 is the threshold, >0.5 inside <0.5 outside
    float inverted_threshold = 1.0 - threshold; // because I want the ui to be +larger -smaller

    float width = screenPxRange();

    float inner = width * (d_inner - inverted_threshold) + 0.5 + out_bias;
    float outer = width * (d_outer - inverted_threshold + outline_width_relative) + 0.5 + out_bias + outline_width_absolute;

    float inner_opacity = clamp(inner, 0.0, 1.0);
    float outer_opacity = clamp(outer, 0.0, 1.0);

    vec4 l_outer_color = outer_color;
    if (outline_blur > 0.0) {
        float blur_start = outline_width_relative + outline_width_absolute / width;
        l_outer_color.a = smootherstep(
            blur_start,
            blur_start * (1.0 - outline_blur),
            inverted_threshold - d_sdf - out_bias / width);
    }

    vec4 color = inner_color.a * ((inner_color * inner_opacity) + (l_outer_color * (outer_opacity - inner_opacity)));

    frag_color = color;
}
