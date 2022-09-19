#pragma once

#include "../application0/Scene.hpp"

class TestScene : public Scene
{
private:
	int testEntity;
	int testEntity2;

public:
	TestScene();
	virtual ~TestScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};
