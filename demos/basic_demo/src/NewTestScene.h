#pragma once
#pragma once

#include "vengine.h"
#define AUDIO 0
class NewTestScene : public Scene
{
private:
	Entity audioSource1;
	Entity audioSource2;
	float volume1;
	float volume2;
	float master;
	float music;
public:
	NewTestScene();
	virtual ~NewTestScene();

	//  Inherited via Scene
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

