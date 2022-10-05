#pragma once

#include "SceneHandler.hpp"

class SceneHandler;

class MatrixHandler
{
private:
	SceneHandler* sceneHandler;

public:
    MatrixHandler();

	void setSceneHandler(SceneHandler* sceneHandler);
	void update();
};