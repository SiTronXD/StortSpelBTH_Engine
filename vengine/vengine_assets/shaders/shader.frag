#version 450

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNor;
layout(location = 2) in vec2 fragTex;

// Uniform buffer for light indices
// Ambient: [0, ambientLightsEndIndex)
// Directional: [ambientLightsEndIndex, directionalLightsEndIndex)
// Point: [directionalLightsEndIndex, pointLightsEndIndex)
layout(set = FREQ_PER_FRAME, binding = 1) uniform AllLightsInfo
{
    uint ambientLightsEndIndex;
    uint directionalLightsEndIndex;
    uint pointLightsEndIndex;
	
    uint padding0;
} allLightsInfo;

// Storage buffer
struct LightBufferData
{
	// Point lights
    vec4 position;

	// Directional lights
    vec4 direction;

	// Ambient/Directional/Point lights
    vec4 color;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer LightBuffer
{
    LightBufferData lights[];
} lightBuffer;

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
	
	// Texture + static diffuse light
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

	// Color from lights
	vec3 finalLightColor = vec3(0.0f);

	// Ambient lights
	for(uint i = 0; 
		i < allLightsInfo.ambientLightsEndIndex; 
		++i)
	{
		finalLightColor += lightBuffer.lights[i].color.xyz;
	}

	// Directional lights
	for(uint i = allLightsInfo.ambientLightsEndIndex; 
		i < allLightsInfo.directionalLightsEndIndex; 
		++i)
	{
		finalLightColor += 
			clamp(
				dot(normal, -lightBuffer.lights[i].direction.xyz),
				0.0f,
				1.0f
			) * lightBuffer.lights[i].color.xyz;
	}

	// Point lights
	for(uint i = allLightsInfo.directionalLightsEndIndex; 
		i < allLightsInfo.pointLightsEndIndex; 
		++i)
	{
		finalLightColor += 
			lightBuffer.lights[i].color.xyz * 
			1.0f / length(lightBuffer.lights[i].position.xyz - fragWorldPos.xyz);
	}
	outColor = vec4(finalLightColor, 1.0f);
}