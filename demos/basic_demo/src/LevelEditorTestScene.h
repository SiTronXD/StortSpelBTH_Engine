#pragma once

#include "vengine.h"
#include "vengine/shared_memory/Comlib.h"

class LevelEditorTestScene : public Scene
{
private:
	Entity camEntity;
	Entity testEntity;
	Comlib* comLib;
	char* msg;
	MessageHeader* header;
	std::unordered_map<std::string, Entity> mayaObjects;

public:
	LevelEditorTestScene();
	virtual ~LevelEditorTestScene();

	//  Inherited via Scene
	virtual void init() override;
	virtual void update() override;


};

