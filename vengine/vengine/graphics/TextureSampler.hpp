#pragma once

#include "TempPCH.hpp"

class Device;

struct TextureSamplerSettings
{
	vk::Filter filterMode = vk::Filter::eLinear;
	vk::Bool32 unnormalizedCoordinates = VK_FALSE;
};

class TextureSampler
{
private:
	vk::Sampler textureSampler{};

	Device* device;

public:
	void createSampler(
		Device& device,
		const TextureSamplerSettings& settings);

	void cleanup();

	inline vk::Sampler& getVkSampler() { return this->textureSampler; }

	static void settingsToString(
		const TextureSamplerSettings& samplerSettings,
		std::string& outputString);
};