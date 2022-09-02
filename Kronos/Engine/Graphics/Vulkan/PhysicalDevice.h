#pragma once

#include <vector>

#include "QueueFamilies.h"

class Renderer;
class Instance;
class Surface;

class PhysicalDevice
{
private:
	VkPhysicalDevice physicalDevice;

public:
	PhysicalDevice();
	~PhysicalDevice();

	void pickPhysicalDevice(
		Instance& instance, 
		Surface& surface,
		const std::vector<const char*>& deviceExtensions,
		QueueFamilies& outputQueueFamilies);

	void cleanup();

	inline VkPhysicalDevice& getVkPhysicalDevice() { return this->physicalDevice; }
};

