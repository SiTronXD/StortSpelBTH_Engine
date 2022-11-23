#version 450

#define FREQ_PER_FRAME 0
#define FREQ_PER_DRAW 2

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;

// Output color
layout(location = 0) out vec4 outColor; 

layout(set = FREQ_PER_FRAME, binding = 0) uniform BloomSettingsBufferData
{
    vec4 strength;
} bloomSettingsData;

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D hdrTexture;
layout(set = FREQ_PER_DRAW, binding = 1) uniform sampler2D bloomTexture;

void getConvolutionColor(inout vec3 outputColor)
{
	const float[] ridgeDetectionkernel = float[](
		-1.0f, -1.0f, -1.0f,
		-1.0f,  8.0f, -1.0f,
		-1.0f, -1.0f, -1.0f
	);

	const float[] gaussianBlurKernel = float[](
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
		2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
	);

	outputColor = vec3(0.0f);
	for(int y = -1; y <= 1; ++y)
	{
		for(int x = -1; x <= 1; ++x)
		{
			int index = 3 * (y + 1) + (x + 1);
			vec2 offset = vec2(x, y) / vec2(1280.0f, 720.0f);
			outputColor += 
				texture(hdrTexture, fragUV + offset).rgb * 
				ridgeDetectionkernel[index];
		}
	}
}

vec3 tonemapACES(in vec3 x)
{
	const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

#define GAMMA 2.2f
vec3 gammaCorrection(in vec3 x)
{
	return pow(clamp(x, 0.0f, 1.0f), vec3(1.0f / GAMMA));
}

void main()
{
	vec3 hdrColor = texture(hdrTexture, fragUV).rgb;
	vec3 bloomColor = texture(bloomTexture, fragUV).rgb;

	vec3 color = mix(hdrColor, bloomColor, bloomSettingsData.strength.x);
	color = tonemapACES(color);
	color = gammaCorrection(color);

	outColor = vec4(color, 1.0f);
}