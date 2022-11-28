#version 450

#define FREQ_PER_DRAW 2

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D textureSampler;

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;

// Output color
layout(location = 0) out vec4 outColor; 

#define GAMMA 2.2f
vec3 invGammaCorrection(in vec3 x)
{
	return pow(clamp(x, 0.0f, 1.0f), vec3(GAMMA));
}

void main() 
{
	vec4 texCol = texture(textureSampler, fragUV);
	texCol.rgb = invGammaCorrection(texCol.rgb);

	outColor = texCol;
}