#pragma once
#include <glm/vec4.hpp>

struct Material
{
	uint32_t diffuseTextureIndex;
	uint32_t specularTextureIndex;
	uint32_t glowMapTextureIndex;

	glm::vec3 emissionColor = glm::vec3(0.0f);
	glm::vec4 tintColor = glm::vec4(0.0f);

	// Vulkan
	uint32_t descriptorIndex = ~0u;
};

