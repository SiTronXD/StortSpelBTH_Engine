#pragma once

#include "PhysicalDevice.hpp"

class VulkanInstance;

class Device
{
private:
	vk::Device device;

public:
	Device();
	~Device();

	void createDevice(
		VulkanInstance& instance,
		PhysicalDevice& physicalDevice, 
		QueueFamilyIndices& queueFamilies,
		
		// TODO: remove these
		vk::DispatchLoaderDynamic& outputDynamicDispatch,
		vk::Queue& outputGraphicsQueue,
		vk::Queue& outputPresentationQueue);

	void cleanup();

	inline vk::Device& getVkDevice() { return this->device; }
};