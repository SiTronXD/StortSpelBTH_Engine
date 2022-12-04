#pragma once 
 #include "op_overload.hpp"

#include "../application/Scene.hpp"

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
