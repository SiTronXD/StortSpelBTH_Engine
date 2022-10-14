#pragma once

#include <DirectXMath.h>

struct UIRectangle
{
	int x;
	int y;
	int width;
	int height;
};

class ResTranslator
{
private:
	static const unsigned int INTERNAL_WIDTH = 1920;
	static const unsigned int INTERNAL_HEIGHT = 1080;

	static uint32_t windowWidth;
	static uint32_t windowHeight;

public:
	static void update(
		const uint32_t& newWindowWidth,
		const uint32_t& newWindowHeight
	);

	static UIRectangle transformRect(
		const UIRectangle& internalRect
	);

	static DirectX::XMFLOAT2 toInternalPos(
		const DirectX::XMFLOAT2& externalPos
	);
};