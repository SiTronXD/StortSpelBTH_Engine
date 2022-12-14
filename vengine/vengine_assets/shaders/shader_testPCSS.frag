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
	uint spotlightsEndIndex;
} allLightsInfo;

layout(set = FREQ_PER_FRAME, binding = 4) uniform ShadowMapBuffer
{
    mat4 projection;
    mat4 view;
    vec2 shadowMapSize;
	float shadowMapMinBias;
	float shadowMapAngleBias;
} shadowMapBuffer;

// Storage buffer
struct LightBufferData
{
	// Point lights/Spotlights
    vec4 position;

	// Directional lights/Spotlights
	// vec4(x, y, z, cos(angle / 2))
    vec4 direction;

	// Ambient/Directional/Point lights/Spotlights
    vec4 color;
};
layout(std140, set = FREQ_PER_FRAME, binding = 2) readonly buffer LightBuffer
{
    LightBufferData lights[];
} lightBuffer;

// Combined image samplers
layout(set = FREQ_PER_FRAME, binding = 3) uniform sampler2D shadowMapSampler;
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

float getShadowFactor(in vec3 normal, in vec3 lightDir)
{
	// Transform to the light's NDC
	vec4 fragLightNDC = 
		shadowMapBuffer.projection * 
		shadowMapBuffer.view * 
		vec4(fragWorldPos, 1.0f);
	fragLightNDC.xyz /= fragLightNDC.w;
	fragLightNDC.y = -fragLightNDC.y;

	// Fragment is outside light frustum
	if( fragLightNDC.x < -1.0f || fragLightNDC.x > 1.0f ||
		fragLightNDC.y < -1.0f || fragLightNDC.y > 1.0f ||
		fragLightNDC.z < 0.0f || fragLightNDC.z > 1.0f)
	{
		return 1.0f;
	}

	// Texture coordinates
	fragLightNDC.xy = fragLightNDC.xy * 0.5f + vec2(0.5f);

	// Calculate shadow bias
	float bias = 
		shadowMapBuffer.shadowMapMinBias + 
		shadowMapBuffer.shadowMapAngleBias * (1.0f - dot(normal, -lightDir));

	// 2x2 PCF
	vec2 shadowMapSize = shadowMapBuffer.shadowMapSize;
	vec2 oneOverSize = vec2(1.0f) / shadowMapSize;
	vec2 unnormalizedFragTex = fragLightNDC.xy * shadowMapSize;
	vec2 unnormalizedCorner = floor(unnormalizedFragTex);
	vec2 fragLightTex00 = (unnormalizedCorner + vec2(0.0f, 0.0f) + vec2(0.5f)) * oneOverSize;
	vec2 fragLightTex10 = (unnormalizedCorner + vec2(1.0f, 0.0f) + vec2(0.5f)) * oneOverSize;
	vec2 fragLightTex01 = (unnormalizedCorner + vec2(0.0f, 1.0f) + vec2(0.5f)) * oneOverSize;
	vec2 fragLightTex11 = (unnormalizedCorner + vec2(1.0f, 1.0f) + vec2(0.5f)) * oneOverSize;

	float shadowMapValue00 = fragLightNDC.z - bias >= texture(shadowMapSampler, fragLightTex00).r ? 0.0f : 1.0f;
	float shadowMapValue10 = fragLightNDC.z - bias >= texture(shadowMapSampler, fragLightTex10).r ? 0.0f : 1.0f;
	float shadowMapValue01 = fragLightNDC.z - bias >= texture(shadowMapSampler, fragLightTex01).r ? 0.0f : 1.0f;
	float shadowMapValue11 = fragLightNDC.z - bias >= texture(shadowMapSampler, fragLightTex11).r ? 0.0f : 1.0f;
	float shadowFactor = mix(
		mix(shadowMapValue00, shadowMapValue10, fract(unnormalizedFragTex.x)),
		mix(shadowMapValue01, shadowMapValue11, fract(unnormalizedFragTex.x)),
		fract(unnormalizedFragTex.y)
	);
	
	return shadowFactor;
}

float gaussWeight(in float len, in float radius)
{
	float x = len * 1.33f / radius;
	return exp(-x * x * 3.1415f);
}

float getShadowFactorPCSS(in vec3 normal, in vec3 lightDir)
{
	// Transform to the light's NDC
	vec4 fragLightNDC = 
		shadowMapBuffer.projection * 
		shadowMapBuffer.view * 
		vec4(fragWorldPos, 1.0f);
	fragLightNDC.xyz /= fragLightNDC.w;
	fragLightNDC.y = -fragLightNDC.y;

	// Fragment is outside light frustum
	if( fragLightNDC.x < -1.0f || fragLightNDC.x > 1.0f ||
		fragLightNDC.y < -1.0f || fragLightNDC.y > 1.0f ||
		fragLightNDC.z < 0.0f || fragLightNDC.z > 1.0f)
	{
		return 1.0f;
	}

	// Texture coordinates
	fragLightNDC.xy = fragLightNDC.xy * 0.5f + vec2(0.5f);

	// Calculate shadow bias
	float bias = 
		shadowMapBuffer.shadowMapMinBias + 
		shadowMapBuffer.shadowMapAngleBias * (1.0f - dot(normal, -lightDir));

	vec2 shadowMapSize = shadowMapBuffer.shadowMapSize;
	vec2 oneOverSmSize = vec2(1.0f) / shadowMapSize;
	
	// Start sampling
	const float wLight = 1.0f;
	float blockerRadius = wLight * 4.0f;
	float dBlocker = 0.0f;
	float numSamples = 0.0f;
	for(float y = -blockerRadius; y <= blockerRadius; y += 1.0f)
	{
		for(float x = -blockerRadius; x <= blockerRadius; x += 1.0f)
		{
			vec2 offset = (vec2(x, y) + vec2(0.5f)) * oneOverSmSize;
			float sampleValue = 
				texture(shadowMapSampler, fragLightNDC.xy + offset).r;

			if(fragLightNDC.z - bias >= sampleValue)
			{
				dBlocker += sampleValue;
				numSamples += 1.0f;
			}
		}
	}
	dBlocker /= numSamples;

	if(numSamples < 1.0f)
	{
		return 1.0f;
	}

	//float dBlocker = max(texture(shadowMapSampler, fragLightNDC.xy).r, 0.0001f);
	float dReceiver = fragLightNDC.z;
	float penumbraSize = (dReceiver - dBlocker) * wLight / dBlocker;
	penumbraSize = min(penumbraSize, 8.0f);
	
	float smDiameter = penumbraSize * 1024.0f * oneOverSmSize.x;
	smDiameter = clamp(smDiameter, 1.0f, 8.0f);
	smDiameter = 8.0f;
	float smRadius = smDiameter * 0.5f;

	// Start sampling
	float shadowMapFactor = 1.0f;
	float weightSum = 0.0f;
	for(float y = -smRadius; y <= smRadius; y += 1.0f)
	{
		for(float x = -smRadius; x <= smRadius; x += 1.0f)
		{
			vec2 offset = vec2(x, y) * oneOverSmSize;
			float sampleValue = 
				fragLightNDC.z - bias >= texture(shadowMapSampler, fragLightNDC.xy + offset).r ? 0.0f : 1.0f;
			float weight = gaussWeight(length(offset), smRadius);
			weightSum += weight;

			shadowMapFactor += sampleValue * weight;
		}
	}

	// Normalize
	shadowMapFactor /= weightSum;

	return shadowMapFactor;
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
			lightBuffer.lights[i].color.xyz * 
			getShadowFactorPCSS(normal, lightDir);
	}

	// Point lights
	for(uint i = allLightsInfo.directionalLightsEndIndex; 
		i < allLightsInfo.pointLightsEndIndex; 
		++i)
	{
		LightBufferData lightData = lightBuffer.lights[i];

		vec3 fragToLight = lightData.position.xyz - fragWorldPos;
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
			lightData.color.xyz;
	}

	// Spotlights
	for(uint i = allLightsInfo.pointLightsEndIndex;
		i < allLightsInfo.spotlightsEndIndex;
		++i)
	{
		LightBufferData lightData = lightBuffer.lights[i];

		vec3 fragToLight = lightData.position.xyz - fragWorldPos;
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
			lightData.color.xyz *
			clamp(
				(dot(-fragToLightDir, lightData.direction.xyz) - lightData.direction.w) /
					(1.0f - lightData.direction.w), 
				0.0f, 
				1.0f
			);
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