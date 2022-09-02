#pragma once

#include "Graphics/Renderer.h"

class Engine
{
//private:
public: //TODO: should be private...
	Window window;
	Renderer renderer;

public:
	Engine();
	~Engine();

	void init();
};