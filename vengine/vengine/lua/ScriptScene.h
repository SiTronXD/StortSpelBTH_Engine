#pragma once

#include "../application/Scene.hpp"

struct ScriptHandler;

class ScriptScene : public Scene
{
private:
	std::string luaPath;
	ScriptHandler* scriptHandler;

public:
	ScriptScene(std::string& path, ScriptHandler* scriptHandler);
	virtual ~ScriptScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

