#version 450

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

#define GLOW_MAP_SCALE 64.0f

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragViewPos;
layout(location = 2) in vec3 fragNor;
layout(location = 3) in vec2 fragTex;
layout(location = 4) in vec4 fragCamWorldPos;	// vec4(fragCamWorldPos, receiveShadows)
layout(location = 5) in vec4 fragTintCol;		// vec4(fragTintCol, fragTintColAlpha)
layout(location = 6) in vec4 fragEmissionCol;	// vec4(fragEmissionCol, intensity)
layout(location = 7) in vec4 fragTiling;

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

#define MAX_NUM_CASCADES 4
layout(set = FREQ_PER_FRAME, binding = 4) uniform ShadowMapInfoBuffer
{
    mat4 viewProjection[MAX_NUM_CASCADES];
    vec2 shadowMapSize;
	float shadowMapMinBias;
	float shadowMapAngleBias;
	uvec4 cascadeSettings; // uvec4(numCascades, cascadeVisualization, 0, 0)
} shadowMapInfoBuffer;

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
layout(set = FREQ_PER_FRAME, binding = 3) uniform sampler2DArray shadowMapSampler;
layout(set = FREQ_PER_DRAW, binding = 0) uniform sampler2D diffuseTextureSampler;
layout(set = FREQ_PER_DRAW, binding = 1) uniform sampler2D specularTextureSampler;
layout(set = FREQ_PER_DRAW, binding = 2) uniform sampler2D glowMapTextureSampler;

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

vec3 sampleCascade(in uint i)
{
	// Transform to the light's NDC
	vec4 fragLightNDC = 
		shadowMapInfoBuffer.viewProjection[i] * 
		vec4(fragWorldPos, 1.0f);
	fragLightNDC.xyz /= fragLightNDC.w;
	fragLightNDC.y = -fragLightNDC.y;

	// Fragment is outside light frustum
	if( fragLightNDC.x < -1.0f || fragLightNDC.x > 1.0f ||
		fragLightNDC.y < -1.0f || fragLightNDC.y > 1.0f ||
		fragLightNDC.z < 0.0f || fragLightNDC.z > 1.0f)
	{
		return vec3(0.0f);
	}
	{
		return vec3(
			i == 0 || i == 3 ? 1.0f : 0.0f,
			i == 1 || i == 3 ? 1.0f : 0.0f,
			i == 2 ? 1.0f : 0.0f
		);
	}
}

bool isInsideLightFrustum(in vec4 ndcPos, in vec2 oneOverSmSize)
{
	// Avoid 3x3 sampling outside shadow map
	return	abs(ndcPos.x) < 1.0f - oneOverSmSize.x * 3.0f &&
			abs(ndcPos.y) < 1.0f - oneOverSmSize.y * 3.0f &&
			ndcPos.z > 0.0f && ndcPos.z < 1.0f;
}

float getShadowFactor(in vec3 normal, in vec3 lightDir)
{
	// Don't receive shadows
	if(fragCamWorldPos.w < 0.5f)
	{
		return 1.0f;
	}

	uint numCascades = shadowMapInfoBuffer.cascadeSettings.x;
	
	vec2 shadowMapSize = shadowMapInfoBuffer.shadowMapSize;
	vec2 oneOverSize = vec2(1.0f) / shadowMapSize;

	// Brute force search through each cascade frustum
	vec4 fragLightNDC = vec4(0.0f);
	uint cascadeIndex = numCascades - 1;
	for(uint i = 0; i < numCascades; ++i)
	{
		// Transform to the light's NDC
		fragLightNDC = 
			shadowMapInfoBuffer.viewProjection[i] * 
			vec4(fragWorldPos, 1.0f);
		fragLightNDC.xyz /= fragLightNDC.w;
		fragLightNDC.y = -fragLightNDC.y;

		// Fragment is inside light frustum
		if(isInsideLightFrustum(fragLightNDC, oneOverSize))
		{
			cascadeIndex = i;
			break;
		}
	}

	// Fragment is outside light frustum
	if(!isInsideLightFrustum(fragLightNDC, oneOverSize))
	{
		return 0.0f;
	}

	// Texture coordinates
	fragLightNDC.xy = fragLightNDC.xy * 0.5f + vec2(0.5f);

	// Calculate shadow bias
	float bias = 
		shadowMapInfoBuffer.shadowMapMinBias + 
		shadowMapInfoBuffer.shadowMapAngleBias * (1.0f - dot(normal, -lightDir));

	// 3x3 PCF
	float shadowFactor = 0.0f;
	for(int y = -1; y <= 1; ++y)
	{
		for(int x = -1; x <= 1; ++x)
		{
			vec2 offset = vec2(x, y) * oneOverSize;
			float approxGaussWeight = 4.0f / pow(2.0f, (abs(x) + abs(y))) / 16.0f;
			float sampleValue = 
				texture(
					shadowMapSampler, 
					vec3(fragLightNDC.xy + offset, float(cascadeIndex))
				).r;

			shadowFactor +=
				(fragLightNDC.z - bias >= sampleValue ? 0.0f : 1.0f) 
				* approxGaussWeight;
		}
	}

	return shadowFactor;

	// 2x2 PCF
	/*vec2 shadowMapSize = shadowMapInfoBuffer.shadowMapSize;
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

	return shadowFactor;*/
}

void debugCascades(inout vec4 outColor)
{
	uint numCascades = shadowMapInfoBuffer.cascadeSettings.x;
	for(uint i = 0; i < numCascades; ++i)
	{
		vec4 col = vec4(sampleCascade(i), 0.0f);

		if(dot(col, col) > 0.5f)
		{
			outColor += col * 0.1f;
			break;
		}
	}
}

#define GAMMA 2.2f
vec3 invGammaCorrection(in vec3 x)
{
	return pow(clamp(x, 0.0f, 1.0f), vec3(GAMMA));
}

void main() 
{
	vec3 normal = normalize(fragNor);

	vec2 finalFragTex = fragTiling.xy + fragTex * fragTiling.zw;

	vec3 diffuseTextureCol = mix(texture(diffuseTextureSampler, finalFragTex).rgb, fragTintCol.rgb, fragTintCol.a);
	diffuseTextureCol = invGammaCorrection(diffuseTextureCol);
	vec4 specularTextureCol = texture(specularTextureSampler, finalFragTex);
	vec3 glowMapTextureCol = texture(glowMapTextureSampler, finalFragTex).rgb;
	
	// Color from lights
	vec3 finalColor = vec3(0.0f);

	// Ambient lights
	for(uint i = 0; 
		i < allLightsInfo.ambientLightsEndIndex; 
		++i)
	{
		finalColor += 
			lightBuffer.lights[i].color.xyz *
			diffuseTextureCol;
	}

	// Directional lights
	for(uint i = allLightsInfo.ambientLightsEndIndex; 
		i < allLightsInfo.directionalLightsEndIndex; 
		++i)
	{
		vec3 lightDir = lightBuffer.lights[i].direction.xyz;
		vec3 fragToLightDir = -lightDir;
		vec3 fragToViewDir = 
			normalize(fragCamWorldPos.xyz - fragWorldPos);
		vec3 halfwayDir = normalize(fragToLightDir + fragToViewDir);

		// Regular diffuse light
		vec3 diffuseLight = calcDiffuse(diffuseTextureCol, normal, fragToLightDir);

		// Blinn specular
		vec3 specularLight = calcSpecular(specularTextureCol, normal, halfwayDir);
		
		// Add blinn-phong light contribution
		finalColor += 
			(diffuseLight + specularLight) * 
			lightBuffer.lights[i].color.xyz * 
			getShadowFactor(normal, lightDir);
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
			normalize(fragCamWorldPos.xyz - fragWorldPos);
		vec3 halfwayDir = normalize(fragToLightDir + fragToViewDir);
		float atten = 1.0f / (1.0f + dot(fragToLight, fragToLight));

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
			normalize(fragCamWorldPos.xyz - fragWorldPos);
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
	distAlpha = pow(distAlpha, 5.0f);

	// Emission
	// 0.5 / 255 = 0.00196078431372549019607843137255
	glowMapTextureCol = max(
		glowMapTextureCol, 
		step(
			0.00196078431372549019607843137255f, 
			dot(glowMapTextureCol, glowMapTextureCol)
		) * 0.02f
	);
	finalColor += 
		max(
			fragEmissionCol.rgb, 
			vec3(0.02f)
		) * (glowMapTextureCol * GLOW_MAP_SCALE) * fragEmissionCol.w;

	// Composite fog
	outColor = vec4(mix(finalColor, vec3(0.8f), distAlpha), 1.0f);
	//outColor = vec4(finalColor, 1.0f); // (No fog)

	// Debug cascades
	if(shadowMapInfoBuffer.cascadeSettings.y > 0u)
	{
		debugCascades(outColor);
	}
}