#version 450

#define FREQ_PER_DRAW 2

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragMultiplyColor;
layout(location = 2) in flat uvec4 fragBoundsUV;

// Output color
layout(location = 0) out vec4 outColor; 

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D textureSampler0;

void main() 
{
	// Use this sampling function to generate correct SPIR-V when
	// using unnormalized uv coordinates
	outColor = textureLod(
		textureSampler0, 
		(fragBoundsUV.z <= 1 && fragBoundsUV.w <= 1) ? 
		fragUV : 
		clamp(
			uvec2(fragUV), 
			fragBoundsUV.xy, 
			fragBoundsUV.xy + fragBoundsUV.zw
		), 
		0
	) * fragMultiplyColor;
}