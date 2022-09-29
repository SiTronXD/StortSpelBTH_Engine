#include "ScriptScene.h"
#include "ScriptHandler.h"

ScriptScene::ScriptScene(std::string& path, ScriptHandler* scriptHandler):
	luaPath(path), scriptHandler(scriptHandler)
{
}

ScriptScene::~ScriptScene()
{
}

void ScriptScene::init()
{
	this->scriptHandler->runScript(luaPath);
}

void ScriptScene::update()
{
}
