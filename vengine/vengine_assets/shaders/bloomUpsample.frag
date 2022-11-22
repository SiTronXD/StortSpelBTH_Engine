#version 450

#define FREQ_PER_DRAW 2

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;

// Output color
layout(location = 0) out vec4 outColor; 

// Push constant
layout(push_constant) uniform ResolutionPushConstantData
{
	vec4 resolution;
} pushConstantData;

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D prevMipTexture;

vec3 sampleTex(in vec2 oneOverSize, in vec2 offset)
{
	return 
		texture(prevMipTexture, fragUV + offset * oneOverSize).rgb;
}

void main()
{
	vec2 oneOverSize = 1.0f / pushConstantData.resolution.xy;

	// 9 samples around current pixel
	// a - b - c
	// d - e - f
	// g - h - i

	vec3 a = sampleTex(oneOverSize, vec2(-1.0f,  1.0f));
	vec3 b = sampleTex(oneOverSize, vec2( 0.0f,  1.0f));
	vec3 c = sampleTex(oneOverSize, vec2( 1.0f,  1.0f));
	
	vec3 d = sampleTex(oneOverSize, vec2(-1.0f,  0.0f));
	vec3 e = sampleTex(oneOverSize, vec2( 0.0f,  0.0f));
	vec3 f = sampleTex(oneOverSize, vec2( 1.0f,  0.0f));
	
	vec3 g = sampleTex(oneOverSize, vec2(-1.0f, -1.0f));
	vec3 h = sampleTex(oneOverSize, vec2( 0.0f, -1.0f));
	vec3 i = sampleTex(oneOverSize, vec2( 1.0f, -1.0f));

	// Weighted sum of samples
	vec3 color = e * 4.0f;
	color += (b + d + f + h) * 2.0f;
	color += (a + c + g + i);
	color *= 1.0f / 16.0f;

	outColor = vec4(color, 1.0f);
}