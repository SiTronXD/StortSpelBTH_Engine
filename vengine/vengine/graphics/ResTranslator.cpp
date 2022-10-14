#include "ResTranslator.hpp"
#include "../dev/Log.hpp"

const float ResTranslator::INTERNAL_ASPECT_RATIO = 
	(float) INTERNAL_WIDTH / INTERNAL_HEIGHT;

uint32_t ResTranslator::windowWidth = 0;
uint32_t ResTranslator::windowHeight = 0;
float ResTranslator::aspectRatio = 1.0f;
float ResTranslator::screenSizeScaleX = 0.0f;
float ResTranslator::screenSizeScaleY = 0.0f;

void ResTranslator::updateWindowSize(
	const uint32_t& newWindowWidth, 
	const uint32_t& newWindowHeight)
{
	ResTranslator::windowWidth = newWindowWidth;
	ResTranslator::windowHeight = newWindowHeight;
	ResTranslator::aspectRatio = 
		(float) ResTranslator::windowWidth / ResTranslator::windowHeight;
	ResTranslator::screenSizeScaleX = (float)windowWidth / INTERNAL_WIDTH;
	ResTranslator::screenSizeScaleY = (float)windowHeight / INTERNAL_HEIGHT;
}

glm::vec4 ResTranslator::transformRect(
	const float& x,
	const float& y,
	const float& width,
	const float& height)
{
	glm::vec4 result(0.0f);

	// 16:9 and wider
	if (aspectRatio >= INTERNAL_ASPECT_RATIO)
	{
		// Scale
		result.z = width / INTERNAL_HEIGHT * 2.0f * screenSizeScaleY / screenSizeScaleX / INTERNAL_ASPECT_RATIO;
		result.w = height / INTERNAL_HEIGHT * 2.0f;

		// Position
		result.x = x / INTERNAL_WIDTH * 2.0f * screenSizeScaleY / screenSizeScaleX;
		result.y = y / INTERNAL_HEIGHT * 2.0f;
	}
	// Smaller than 16:9
	else
	{
		// Scale
		result.z = width / INTERNAL_WIDTH * 2.0f;
		result.w = height / INTERNAL_WIDTH * 2.0f * screenSizeScaleX / screenSizeScaleY * INTERNAL_ASPECT_RATIO;

		// Position
		result.x = x / INTERNAL_WIDTH * 2.0f;
		result.y = y / INTERNAL_HEIGHT * 2.0f * screenSizeScaleX / screenSizeScaleY;
	}

	return result;
}

glm::vec2 ResTranslator::toInternalPos(
	const glm::vec2& externalPos)
{
	// TODO: fix this
	Log::error("ResTranslator::toInternalPos does not work atm...");

	float aspectRatio = (float) windowWidth / windowHeight;
	float newX = (externalPos.x / windowWidth - 0.5f) * INTERNAL_HEIGHT * aspectRatio;
	float newY = -(externalPos.y / windowHeight - 0.5f) * INTERNAL_HEIGHT;

	return glm::vec2(newX, newY);
}
