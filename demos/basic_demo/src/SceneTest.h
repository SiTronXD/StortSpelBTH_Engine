#pragma once

#include "vengine.h"

class SceneTest : public Scene
{
private:
	const static int NUM = 5;
	Entity entities[NUM * NUM];
	Entity floor;
public:
	SceneTest();
	virtual ~SceneTest();

	// Inherited via Scene
	virtual void init() override;
	virtual void start() override;
	virtual void update() override;

	virtual void onCollisionEnter(Entity e1, Entity e2) override;
	virtual void onCollisionStay(Entity e1, Entity e2) override;
	virtual void onCollisionExit(Entity e1, Entity e2) override;
	virtual void onTriggerEnter(Entity e1, Entity e2) override;
	virtual void onTriggerStay(Entity e1, Entity e2) override;
	virtual void onTriggerExit(Entity e1, Entity e2) override;
};

