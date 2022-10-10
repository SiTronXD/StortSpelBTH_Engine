#version 450

// Input data from vertex shader
layout(location = 0) in vec2 uv;

// Output color
layout(location = 0) out vec4 outColor; 

void main() 
{
	outColor = vec4(uv, 0.0f, 1.0f);
}