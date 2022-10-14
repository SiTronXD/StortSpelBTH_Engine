#include "ResTranslator.hpp"
#include "../dev/Log.hpp"

uint32_t ResTranslator::windowWidth = 0;
uint32_t ResTranslator::windowHeight = 0;

void ResTranslator::updateWindowSize(
	const uint32_t& newWindowWidth, 
	const uint32_t& newWindowHeight)
{
	ResTranslator::windowWidth = newWindowWidth;
	ResTranslator::windowHeight = newWindowHeight;
}

glm::vec4 ResTranslator::transformRect(
	const float& x,
	const float& y,
	const float& width,
	const float& height)
{
	glm::vec4 result(x, y, width, height);

	float aspectRatio = (float) windowWidth / windowHeight;
	float internalAspectRatio = (float)INTERNAL_WIDTH / INTERNAL_HEIGHT;
	float screenSizeScaleX = (float) windowWidth / INTERNAL_WIDTH;
	float screenSizeScaleY = (float) windowHeight / INTERNAL_HEIGHT;


	// Scale
	result.z *= screenSizeScaleY / screenSizeScaleX / internalAspectRatio;
	result.w *= 1.0f;

	// Position
	result.x = result.x / INTERNAL_WIDTH * 2.0f * screenSizeScaleY / screenSizeScaleX;
	result.y = result.y / INTERNAL_HEIGHT * 2.0f;

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
