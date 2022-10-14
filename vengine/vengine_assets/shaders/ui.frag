#version 450

#define FREQ_PER_DRAW 2

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;

// Output color
layout(location = 0) out vec4 outColor; 

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D textureSampler0;

void main() 
{
	outColor = texture(textureSampler0, fragUV);
}