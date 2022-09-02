#pragma once

#include "Instance.h"

class Renderer;
class Window;

class Surface
{
private:
	VkSurfaceKHR surface;

	Renderer& renderer;

public:
	Surface(Renderer& renderer);
	~Surface();

	void createSurface();

	void cleanup();

	inline VkSurfaceKHR& getVkSurface() { return this->surface; }
};

