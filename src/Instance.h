#pragma once

#include "TempPCH.h"

class Window;

class Instance
{
private:
	vk::Instance instance;

	bool checkInstanceExtensionSupport(
		std::vector<const char*>* checkExtensions);

public:
	Instance();
	~Instance();

	void createInstance(Window& window);

	void destroy(vk::SurfaceKHR& surface);
	void cleanup();

	inline vk::Instance& getVkInstance() { return this->instance; }
};