#version 450

#define FREQ_PER_DRAW 2

// Input data from vertex shader
layout(location = 0) in vec2 fragUV;

// Output color
layout(location = 0) out vec4 outColor; 

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D prevMipTexture;

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
				texture(prevMipTexture, fragUV + offset).rgb * 
				gaussianBlurKernel[index];
		}
	}
}

void main()
{
	vec3 prevMipColor = texture(prevMipTexture, fragUV).rgb;

	getConvolutionColor(prevMipColor);

	outColor = vec4(prevMipColor, 1.0f);
}