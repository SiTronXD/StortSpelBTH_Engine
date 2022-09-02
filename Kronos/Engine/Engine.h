#pragma once

#include "Graphics/Renderer.h"

class Engine
{
private:
	Window window;
	Renderer renderer;

public:
	Engine();
	~Engine();

	void init();
};