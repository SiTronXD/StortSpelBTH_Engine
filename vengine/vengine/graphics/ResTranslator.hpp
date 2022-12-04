#pragma once 
 #include "op_overload.hpp"

#include "../VengineMath.hpp"

class Swapchain;

class ResTranslator
{
public:
	static const unsigned int INTERNAL_WIDTH = 1920;
	static const unsigned int INTERNAL_HEIGHT = 1080;
private:
	friend Swapchain;

	static const float INTERNAL_ASPECT_RATIO;

	static uint32_t windowWidth;
	static uint32_t windowHeight;
	static float aspectRatio;
	static float screenSizeScaleX;
	static float screenSizeScaleY;

	static void updateWindowSize(
		const uint32_t& newWindowWidth,
		const uint32_t& newWindowHeight
	);

public:
	static glm::vec4 transformRect(
		const glm::vec2 position,
		const glm::vec2 dimension
	);

	static glm::vec2 toInternalPos(
		const glm::vec2& externalPos
	);

	static glm::vec2 getInternalDimensions();
};