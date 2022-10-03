#version 450

#define FREQ_PER_DRAW 2

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragTex;

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D textureSampler0;

layout(location = 0) out vec4 outColour; // final output color (must also have location)

void main() 
{
	outColour = texture(textureSampler0, fragTex);
}