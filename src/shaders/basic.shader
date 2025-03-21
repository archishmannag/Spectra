#shader vertex
#version 420 core

layout(location = 0) in vec2 aPos;  // Vertex position (x, y)

uniform mat4 projection;

out vec3 fragcolor; // Output color for fragment shader

void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);

    fragcolor = vec3(1.0, 0.0, 0.0); // Set color to red
}

#shader fragment
#version 420 core

in vec3 fragcolor;

out vec4 color;

void main()
{
    color = vec4(fragcolor, 1.0);
}
