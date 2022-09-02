#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class Window;

class Instance
{
private:
	VkInstance instance;

public:
	Instance();
	~Instance();

	void createInstance(
		bool enableValidationLayers,
		bool enablePrintingBestPractices,
		const std::vector<const char*>& instanceExtensions,
		const std::vector<const char*>& validationLayers,
		Window* window);

	void cleanup();

	inline VkInstance& getVkInstance() { return this->instance; }
};

