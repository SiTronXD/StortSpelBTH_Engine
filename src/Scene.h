#pragma once

class SceneHandler;

class Scene
{
private:
	SceneHandler& sceneHandler;
	
public:
	Scene(SceneHandler& sceneHandler);
	virtual ~Scene();

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void renderUI() = 0;
};
