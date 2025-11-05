#shader vertex
#version 420 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 projection;

out vec4 fragcolor;

void main() {
    gl_Position = projection * vec4(aPos, 1.0);
    fragcolor = aColor;
}

#shader fragment
#version 420 core

in vec4 fragcolor;

out vec4 color;

void main() {
    color = fragcolor;
}
