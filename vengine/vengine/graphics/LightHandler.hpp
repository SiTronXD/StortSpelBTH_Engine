#pragma once

#include "vulkan/UniformBufferStructs.hpp"
#include "ShaderInput.hpp"

class Scene;

class LightHandler
{
private:
	std::vector<LightBufferData> lightBuffer;

public:
	static const uint32_t MAX_NUM_LIGHTS = 16;

	void updateLightBuffers(
		Scene* scene,
		ShaderInput& shaderInput,
		ShaderInput& animShaderInput,
		const UniformBufferID& allLightsInfoUB,
		const UniformBufferID& animAllLightsInfoUB,
		const StorageBufferID& lightBufferSB,
		const StorageBufferID& animLightBufferSB,
		const bool& hasAnimations);
};