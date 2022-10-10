#version 460

// Vertex data
layout(location = 0) in vec3 pos;

void main()
{
    gl_Position = vec4(pos, 1.0);
}