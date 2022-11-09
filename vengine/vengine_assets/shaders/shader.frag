#version 450

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNor;
layout(location = 2) in vec2 fragTex;
layout(location = 3) in vec3 fragCamWorldPos;
layout(location = 4) in vec4 fragTintCol;

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
layout(std140, set = FREQ_PER_FRAME, binding = 2) readonly buffer LightBuffer
{
    LightBufferData lights[];
} lightBuffer;

layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D textureSampler0;
layout(set = FREQ_PER_DRAW, binding = 1) uniform sampler2D textureSampler1;

layout(location = 0) out vec4 outColor; // final output color (must also have location)

vec3 calcDiffuse(in vec3 diffuseTexCol, in vec3 normal, in vec3 fragToLightDir)
{
	return 
		diffuseTexCol *
		clamp(
			dot(normal, fragToLightDir),
			0.0f,
			1.0f
		);
}

vec3 calcSpecular(in vec4 specularTexCol, in vec3 normal, in vec3 halfwayDir)
{
	return 
		specularTexCol.rgb * 
		pow(
			max(dot(normal, halfwayDir), 0.0f), 
			specularTexCol.a * 256.0f
		);
}

void main() 
{
	vec3 normal = normalize(fragNor);

	vec3 diffuseTextureCol = mix(texture(textureSampler0, fragTex).rgb, fragTintCol.rgb, fragTintCol.a);
	vec4 specularTextureCol = texture(textureSampler1, fragTex);
	
	// Color from lights
	vec3 finalColor = vec3(0.0f);

	// Ambient lights
	for(uint i = 0; 
		i < allLightsInfo.ambientLightsEndIndex; 
		++i)
	{
		finalColor += lightBuffer.lights[i].color.xyz;
	}

	// Directional lights
	for(uint i = allLightsInfo.ambientLightsEndIndex; 
		i < allLightsInfo.directionalLightsEndIndex; 
		++i)
	{
		vec3 lightDir = lightBuffer.lights[i].direction.xyz;
		vec3 fragToLightDir = -lightDir;
		vec3 fragToViewDir = 
			normalize(fragCamWorldPos - fragWorldPos);
		vec3 halfwayDir = normalize(fragToLightDir + fragToViewDir);

		// Regular diffuse light
		vec3 diffuseLight = calcDiffuse(diffuseTextureCol, normal, fragToLightDir);

		// Blinn specular
		vec3 specularLight = calcSpecular(specularTextureCol, normal, halfwayDir);
		
		// Add blinn-phong light contribution
		finalColor += 
			(diffuseLight + specularLight) * 
			lightBuffer.lights[i].color.xyz;
	}

	// Point lights
	for(uint i = allLightsInfo.directionalLightsEndIndex; 
		i < allLightsInfo.pointLightsEndIndex; 
		++i)
	{
		vec3 fragToLight = lightBuffer.lights[i].position.xyz - fragWorldPos;
		vec3 fragToLightDir = normalize(fragToLight);
		vec3 fragToViewDir = 
			normalize(fragCamWorldPos - fragWorldPos);
		vec3 halfwayDir = normalize(fragToLightDir + fragToViewDir);
		float atten = 1.0f / (1.0f + length(fragToLight));

		// Regular diffuse light
		vec3 diffuseLight = calcDiffuse(diffuseTextureCol, normal, fragToLightDir);

		// Blinn specular
		vec3 specularLight = calcSpecular(specularTextureCol, normal, halfwayDir);

		// Add blinn-phong light contribution
		finalColor += 
			(diffuseLight + specularLight) * atten * 
			lightBuffer.lights[i].color.xyz;
	}

	// Temporary fog
	const float MIN_DIST = 0.995f;
	const float MAX_DIST = 1.0f;
	float distAlpha = clamp(
		(gl_FragCoord.z - MIN_DIST) / (MAX_DIST - MIN_DIST), 
		0.0f, 
		1.0f
	);
	distAlpha = distAlpha * distAlpha;

	// Composite fog
	outColor = vec4(mix(finalColor, vec3(0.8f), distAlpha), 1.0f);

	// No fog
	// outColor = vec4(finalColor, 1.0f);
}