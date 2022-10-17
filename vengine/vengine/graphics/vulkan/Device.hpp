#pragma once

#include "PhysicalDevice.hpp"

class VulkanInstance;
class QueueFamilies;

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
		QueueFamilies& outputQueueFamilies,
		
		// TODO: remove this
		vk::DispatchLoaderDynamic& outputDynamicDispatch);

	void destroyBuffer(vk::Buffer& buffer);
	void waitIdle();
	void cleanup();

	inline vk::Device& getVkDevice() { return this->device; }
};