#version 450

// Input data from vertex shader
layout(location = 0) in vec3 fragCol;

// Output color
layout(location = 0) out vec4 outColor; 

void main() 
{
	outColor = vec4(fragCol, 1.0f);
}