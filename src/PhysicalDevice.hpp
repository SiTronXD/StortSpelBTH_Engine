#pragma once

#include "TempPCH.hpp"

class VulkanInstance;

struct SwapChainDetails;
struct QueueFamilyIndices;

class PhysicalDevice
{
private:
	vk::PhysicalDevice physicalDevice;

	void getSwapchainDetails(
		const vk::PhysicalDevice& physDevice,
		vk::SurfaceKHR& surface,
		SwapChainDetails& outputDetails);

	bool checkPhysicalDeviceSuitability(
		const vk::PhysicalDevice& physDevice,
		vk::SurfaceKHR& surface,
		SwapChainDetails& outputSwapChainDetails);
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
		QueueFamilyIndices& outputQueueFamilies,
		SwapChainDetails& outputSwapChainDetails);

	inline vk::PhysicalDevice& getVkPhysicalDevice() { return this->physicalDevice; }
};