#pragma once

#include "../VengineMath.hpp"

class Swapchain;

struct UIRectangle
{
	float x;
	float y;
	float width;
	float height;
};

class ResTranslator
{
private:
	friend Swapchain;

	static const unsigned int INTERNAL_WIDTH = 1920;
	static const unsigned int INTERNAL_HEIGHT = 1080;

	static uint32_t windowWidth;
	static uint32_t windowHeight;

	static void updateWindowSize(
		const uint32_t& newWindowWidth,
		const uint32_t& newWindowHeight
	);

public:
	static glm::vec4 transformRect(
		const float& x,
		const float& y,
		const float& width,
		const float& height
	);

	static glm::vec2 toInternalPos(
		const glm::vec2& externalPos
	);
};