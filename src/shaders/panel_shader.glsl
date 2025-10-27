#shader vertex
#version 420 core

layout(location = 0) in vec2 aPos;

uniform mat4 projection;
uniform vec2 u_panel_size;
uniform vec2 u_panel_pos;

out vec2 v_uv;

void main() {
    v_uv = (aPos-u_panel_pos)/u_panel_size;
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}

#shader fragment
#version 420 core

in vec2 v_uv;
out vec4 fragColor;

uniform vec4 u_fill_color;
uniform vec4 u_border_color;
uniform vec2 u_panel_size;
uniform float u_border_radius;
uniform float u_border_thickness;

// Signed Distance Function (SDF) for rounded box centered at (0.5, 0.5)
float roundedBoxSDF(vec2 uv, vec2 size, float radius)
{
    // shift so (0.5, 0.5) is center of the quad
    vec2 p = (uv - 0.5) * size;
    vec2 halfSize = size * 0.5 - vec2(radius);
    vec2 d = abs(p) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
}

void main()
{
    float dist = roundedBoxSDF(v_uv, u_panel_size, u_border_radius);

    // Edge anti-aliasing factor
    float aa = fwidth(dist) * 0.5;

    // Fill mask: inside area of the rounded rectangle
    float fillMask = smoothstep(aa, -aa, dist + u_border_thickness);

    // Border mask: narrow region around the shape's outer edge
    float outerMask = smoothstep(aa, -aa, dist);
    float borderMask = outerMask - fillMask;

    // Mix fill and border colors
    vec4 color = mix(u_fill_color, u_border_color, borderMask);

    // Combine alpha from masks
    float alpha = outerMask;

    fragColor = vec4(color.rgb, color.a * alpha);
}
