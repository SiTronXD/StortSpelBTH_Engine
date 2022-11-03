#pragma once

#include "TempPCH.hpp"
#include "Texture.hpp"

class Device;

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