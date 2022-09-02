#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "QueueFamilies.h"

class Renderer;

class Device
{
private:
	VkDevice device;

	Renderer& renderer;

public:
	Device(Renderer& renderer);
	~Device();

	void createDevice(
		const std::vector<const char*>& deviceExtensions,
		const std::vector<const char*>& validationLayers,
		bool enableValidationLayers,
		const QueueFamilyIndices& queueFamilyIndices);

	void cleanup();

	inline VkDevice& getVkDevice() { return this->device; }
};