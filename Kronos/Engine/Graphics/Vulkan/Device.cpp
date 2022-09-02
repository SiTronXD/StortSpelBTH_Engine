#include <set>

#include "Device.h"
#include "../Renderer.h"

Device::Device(Renderer& renderer)
	: renderer(renderer),
	device(VK_NULL_HANDLE)
{
}

Device::~Device()
{
}

void Device::createDevice(
	const std::vector<const char*>& deviceExtensions,
	const std::vector<const char*>& validationLayers,
	bool enableValidationLayers,
	const QueueFamilyIndices& indices)
{
	// Unique queue families to be used
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	// Create queue create info structs
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// ---------- Device features ----------

	// Device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// ---------- Logical device ----------

	// Logical device create info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Not used in newer versions of vulkan
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Create the logical device
	if (vkCreateDevice(
		this->renderer.getVkPhysicalDevice(),
		&createInfo,
		nullptr,
		&this->getVkDevice()) != VK_SUCCESS)
	{
		Log::error("Failed to create logical device!");
		return;
	}
}

void Device::cleanup()
{
	vkDestroyDevice(this->device, nullptr);
}
