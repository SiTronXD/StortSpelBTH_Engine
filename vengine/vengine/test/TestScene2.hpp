#pragma once

#include "../application/Scene.hpp"

class TestScene2 : public Scene
{
private:
	uint32_t uiTextureIndex0;
	uint32_t uiTextureIndex1;
	int playerID;
public:
	TestScene2();
	virtual ~TestScene2();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};
