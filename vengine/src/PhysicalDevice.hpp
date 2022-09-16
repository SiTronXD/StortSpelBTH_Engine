#pragma once

#include "TempPCH.hpp"

class VulkanInstance;

struct SwapChainDetails;
struct QueueFamilyIndices;

class PhysicalDevice
{
private:
	vk::PhysicalDevice physicalDevice;

	bool checkPhysicalDeviceSuitability(
		vk::PhysicalDevice& physDevice,
		vk::SurfaceKHR& surface);
	bool checkDeviceExtensionSupport(
		const vk::PhysicalDevice& physDevice);

	QueueFamilyIndices getQueueFamilies(
		const vk::PhysicalDevice& physDevice,
		vk::SurfaceKHR& surface);

public:
	PhysicalDevice();
	~PhysicalDevice();

	void pickPhysicalDevice(
		VulkanInstance& instance,
		vk::SurfaceKHR& surface,
		QueueFamilyIndices& outputQueueFamilies);

	inline vk::PhysicalDevice& getVkPhysicalDevice() { return this->physicalDevice; }
};