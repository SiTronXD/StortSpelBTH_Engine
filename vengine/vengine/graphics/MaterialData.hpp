#pragma once
#include <glm/vec3.hpp>

struct Material
{
	uint32_t diffuseTextureIndex;
	uint32_t specularTextureIndex;

	// Vulkan
	uint32_t descriptorIndex;
};