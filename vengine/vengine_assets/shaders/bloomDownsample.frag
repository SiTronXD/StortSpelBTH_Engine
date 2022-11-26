#version 450

#define FREQ_PER_DRAW 2

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;

// Output color
layout(location = 0) out vec4 outColor; 

// Push constant
layout(push_constant) uniform DownsamplePushConstantData
{
	vec4 data; // vec4(resX, resY, 0, weight)
} downsampleData;

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D prevMipTexture;

vec3 sampleTex(in vec2 oneOverSize, in vec2 offset)
{
	return texture(prevMipTexture, fragUV + offset * oneOverSize).rgb;
}

void main()
{
	vec2 oneOverSize = 1.0f / downsampleData.data.xy;

	// 13 samples around current pixel
	// a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
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
	
	vec3 j = sampleTex(oneOverSize, vec2(-0.5f,  0.5f));
	vec3 k = sampleTex(oneOverSize, vec2( 0.5f,  0.5f));
	vec3 l = sampleTex(oneOverSize, vec2(-0.5f, -0.5f));
	vec3 m = sampleTex(oneOverSize, vec2( 0.5f, -0.5f));

	// Weighted sum of samples
	vec3 color = e * 0.125f;
	color += (a + c + g + i) * 0.03125f;
	color += (b + d + f + h) * 0.0625f;
	color += (j + k + l + m) * 0.125f;

	color = max(color, vec3(0.0001f));

	outColor = vec4(color, 1.0f);
}