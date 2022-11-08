#pragma once
#include <glm/vec3.hpp>

struct Material
{
	uint32_t diffuseTextureIndex;

	// Vulkan
	uint32_t descriptorIndex;
};