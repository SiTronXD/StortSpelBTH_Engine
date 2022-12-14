#pragma once

#include <btBulletCollisionCommon.h>
#include <glm/glm.hpp>
#include <vector>

struct ContactCallbackPair : btCollisionWorld::ContactResultCallback
{
	bool hit = false;
	// Inherited via ContactResultCallback
	virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override;
};

