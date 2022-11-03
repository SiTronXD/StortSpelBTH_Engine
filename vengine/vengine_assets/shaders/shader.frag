#version 450

#define FREQ_PER_DRAW 2

layout(location = 0) in vec3 fragNor;
layout(location = 1) in vec2 fragTex;

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D textureSampler0;

layout(location = 0) out vec4 outColor; // final output color (must also have location)

void main() 
{
	vec3 normal = normalize(fragNor);

	float diffuse = clamp(
		dot(normal, -normalize(vec3(-0.43f, -1.0f, 1.0f))), 
		0.0f, 
		1.0f
	) * 0.8f + 0.2f;

	vec3 finalCol = texture(textureSampler0, fragTex).rgb * diffuse;
	
	// outColor = vec4(finalCol, 1.0f);

	// Temporary fog
	const float MIN_DIST = 0.995f;
	const float MAX_DIST = 1.0f;
	float distAlpha = clamp(
		(gl_FragCoord.z - MIN_DIST) / (MAX_DIST - MIN_DIST), 
		0.0f, 
		1.0f
	);
	distAlpha = distAlpha * distAlpha;

	outColor = vec4(mix(finalCol, vec3(0.8f), distAlpha), 1.0f);

	outColor = vec4(fragNor, 1.0f);
}