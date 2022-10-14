#include "ResTranslator.hpp"
#include "../dev/Log.hpp"

uint32_t ResTranslator::windowWidth = 0;
uint32_t ResTranslator::windowHeight = 0;

void ResTranslator::update(
	const uint32_t& newWindowWidth, 
	const uint32_t& newWindowHeight)
{
	ResTranslator::windowWidth = newWindowWidth;
	ResTranslator::windowHeight = newWindowHeight;
}

UIRectangle ResTranslator::transformRect(
	const UIRectangle& internalRect)
{
	UIRectangle newRect = internalRect;

	float screenMidX = windowWidth * 0.5f;
	float screenMidY = windowHeight * 0.5f;

	float screenSizeScaleY = (float) windowHeight / INTERNAL_HEIGHT;


	// Scale
	newRect.width *= screenSizeScaleY;
	newRect.height *= screenSizeScaleY;

	// Position
	newRect.x = screenMidX + (internalRect.x - (internalRect.width * 0.5f)) * screenSizeScaleY;
	newRect.y = screenMidY + (-internalRect.y - (internalRect.height * 0.5f)) * screenSizeScaleY;

	return newRect;
}

DirectX::XMFLOAT2 ResTranslator::toInternalPos(
	const DirectX::XMFLOAT2& externalPos)
{
	float aspectRatio = (float) windowWidth / windowHeight;
	float newX = (externalPos.x / windowWidth - 0.5f) * INTERNAL_HEIGHT * aspectRatio;
	float newY = -(externalPos.y / windowHeight - 0.5f) * INTERNAL_HEIGHT;

	return DirectX::XMFLOAT2(newX, newY);
}
