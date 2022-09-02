#pragma once

#include <vector>
#include "QueueFamilies.h"

class Window;

class SupportChecker
{
private:
public:
	static std::vector<const char*> getRequiredExtensions(Window& window, bool enableValidationLayers);
	static bool checkValidationLayerSupport(const std::vector<const char*>& layersToSupport);
	static bool checkDeviceExtensionSupport(
		const std::vector<const char*>& deviceExtensions, 
		VkPhysicalDevice device);
	static bool isDeviceSuitable(
		const std::vector<const char*>& deviceExtensions, 
		VkPhysicalDevice device,
		VkSurfaceKHR surface,
		QueueFamilyIndices& outputIndices);
};