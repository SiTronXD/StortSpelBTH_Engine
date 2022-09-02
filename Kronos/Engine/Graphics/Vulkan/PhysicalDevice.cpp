#include "PhysicalDevice.h"
#include "SupportChecker.h"
#include "Instance.h"
#include "Surface.h"

#include "../Renderer.h"

PhysicalDevice::PhysicalDevice()
	: physicalDevice(VK_NULL_HANDLE)
{
}

PhysicalDevice::~PhysicalDevice()
{
}

void PhysicalDevice::pickPhysicalDevice(
	Instance& instance, 
	Surface& surface, 
	const std::vector<const char*>& deviceExtensions,
	QueueFamilies& outputQueueFamilies)
{
	// Get device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.getVkInstance(), &deviceCount, nullptr);

	// No devices found
	if (deviceCount == 0)
	{
		Log::error("Failed to find GPUs with Vulkan support.");
		return;
	}

	// Get device handles
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance.getVkInstance(), &deviceCount, devices.data());

	// Pick the first best found device
	for (const auto& device : devices)
	{
		QueueFamilyIndices indices{};

		// Check support
		if (SupportChecker::isDeviceSuitable(deviceExtensions, device, surface.getVkSurface(), indices))
		{
			this->physicalDevice = device;

			// Set indices after finding a suitable device
			outputQueueFamilies.setIndices(indices);

			break;
		}
	}

	if (this->physicalDevice == VK_NULL_HANDLE)
	{
		Log::error("Failed to find a suitable GPU.");
		return;
	}
}

void PhysicalDevice::cleanup()
{
}
