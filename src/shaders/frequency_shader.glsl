#shader vertex
#version 420 core

layout(location = 0) in vec3 aPos;  // Vertex position (x, y, z)

uniform mat4 projection;
uniform float u_screenWidth;   // Screen width uniform

out vec3 fragcolor; // Output color for fragment shader

void main() {
    gl_Position = projection * vec4(aPos, 1.0);
    
    // Normalize x to 0-1 range
    float normalizedX = aPos.x / u_screenWidth;
    
    // R channel: 1.0 at x=0, 0.0 at x=screen-width
    float r = 1.0 - normalizedX;
    
    // G channel: Uniform sine from x=0 to x=width
    float g = sin(normalizedX * 2.0 * 3.14159265); //* 0.5 + 0.5;
    
    // B channel: 0.0 at x=0, 1.0 at x=width
    float b = normalizedX;
    
    fragcolor = vec3(r, g, b);
}

#shader fragment
#version 420 core

in vec3 fragcolor;

out vec4 color;

void main()
{
    color = vec4(fragcolor, 1.0);
}
