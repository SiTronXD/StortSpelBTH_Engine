#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Renderer;

class Window
{
private:
	GLFWwindow* windowHandle;

public:
	Window();
	~Window();

	void init(Renderer& renderer, int width, int height);
	void update();
	void awaitEvents();

	void getFramebufferSize(int& widthOutput, int& heightOutput);
	void getInstanceExtensions(const char**& extensions, uint32_t& extensionCount);

	bool isRunning() const;

	inline GLFWwindow* getWindowHandle() { return this->windowHandle; }
};