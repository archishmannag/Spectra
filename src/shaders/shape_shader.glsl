#shader vertex
#version 420 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 projection;

out vec4 vertexColor;

void main() {
    gl_Position = projection * vec4(aPos, 1.0);
    vertexColor = aColor;
}

#shader fragment
#version 420 core

in vec4 vertexColor;

out vec4 color;

void main() {
    // Use vertex color if available, otherwise use uniform color
    color = vertexColor;
}
