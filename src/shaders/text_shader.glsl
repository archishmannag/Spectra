#shader vertex
#version 420 core

layout(location = 0) in vec2 aPos;  // Vertex position (x, y)
layout(location = 1) in vec2 aTexCoord; // Texture coordinates (u, v)

uniform mat4 projection;

out vec2 TexCoord; // Output texture coordinates for fragment shader

void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord; // Pass texture coordinates to fragment shader
}

#shader fragment
#version 420 core

in vec2 TexCoord; // Input texture coordinates from vertex shader

out vec4 FragColor; // Output color

uniform sampler2D text; // Texture sampler for the text
uniform vec3 textColor; // Color of the text

void main()
{
    // Sample the texture to get the alpha value
    float alpha = texture(text, TexCoord).r;
    
    // Create the final color with the given text color and sampled alpha
    FragColor = vec4(textColor, alpha);
}
