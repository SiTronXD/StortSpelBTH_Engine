#pragma once

#include "TempPCH.hpp"

class Window;

class VulkanInstance
{
private:
	vk::Instance instance;

	bool checkInstanceExtensionSupport(
		std::vector<const char*>* checkExtensions);

public:
	VulkanInstance();
	~VulkanInstance();

	void createInstance(Window& window);

	void destroy(vk::SurfaceKHR& surface);
	void cleanup();

	inline vk::Instance& getVkInstance() { return this->instance; }
};