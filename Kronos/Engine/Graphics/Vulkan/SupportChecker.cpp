#include "SupportChecker.h"
#include "QueueFamilies.h"
#include "../Swapchain.h"
#include "../../Application/Window.h"

#include <set>
#include <string>

std::vector<const char*> SupportChecker::getRequiredExtensions(
	Window& window, 
	bool enableValidationLayers)
{
	// Get required instance extensions from the window
	uint32_t extensionCount = 0;
	const char** requiredExtensions;
	window.getInstanceExtensions(requiredExtensions, extensionCount);
	std::vector<const char*> extensions(requiredExtensions, requiredExtensions + extensionCount);

	// Add extension to support validation layers
	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool SupportChecker::checkValidationLayerSupport(const std::vector<const char*>& layersToSupport)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : layersToSupport)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

bool SupportChecker::checkDeviceExtensionSupport(
	const std::vector<const char*>& deviceExtensions,
	VkPhysicalDevice device)
{
	// Get available extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(
		device, nullptr,
		&extensionCount, nullptr
	);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		device, nullptr,
		&extensionCount, availableExtensions.data()
	);

	// Unique required extensions
	std::set<std::string> requiredExtensions(
		deviceExtensions.begin(), deviceExtensions.end()
	);

	// Remove found extensions
	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	// Have all required extensions been found and removed?
	return requiredExtensions.empty();
}

bool SupportChecker::isDeviceSuitable(
	const std::vector<const char*>& deviceExtensions,
	VkPhysicalDevice device,
	VkSurfaceKHR surface, 
	QueueFamilyIndices& outputIndices)
{
	outputIndices = QueueFamilies::findQueueFamilies(surface, device);

	// Find required extension support
	bool extensionsSupported = checkDeviceExtensionSupport(deviceExtensions, device);

	// Swapchain with correct support
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapchainSupport{};
		Swapchain::querySwapChainSupport(surface, device, swapchainSupport);
		swapChainAdequate = !swapchainSupport.formats.empty() &&
			!swapchainSupport.presentModes.empty();
	}

	// Get device features
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	bool foundSuitableDevice = outputIndices.isComplete() &&
		extensionsSupported &&
		swapChainAdequate &&
		supportedFeatures.samplerAnisotropy &&
		supportedFeatures.fillModeNonSolid;

	return foundSuitableDevice;
}
