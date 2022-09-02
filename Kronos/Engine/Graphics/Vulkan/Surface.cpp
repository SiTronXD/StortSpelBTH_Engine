#include "Surface.h"

#include "../Renderer.h"

Surface::Surface(Renderer& renderer)
	: renderer(renderer),
	surface(VK_NULL_HANDLE)
{
}

Surface::~Surface()
{
}

void Surface::createSurface()
{
	if (glfwCreateWindowSurface(
		this->renderer.getVkInstance(),
		this->renderer.getWindow().getWindowHandle(),
		nullptr,
		&this->surface) != VK_SUCCESS)
	{
		Log::error("Failed to create window surface.");
		return;
	}
}

void Surface::cleanup()
{
	vkDestroySurfaceKHR(this->renderer.getVkInstance(), this->surface, nullptr);
}
