#version 450

#define FREQ_PER_DRAW 2

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;

// Output color
layout(location = 0) out vec4 outColor; 

void main() 
{
	outColor = vec4(fragUV, 0.0f, 1.0f);
}