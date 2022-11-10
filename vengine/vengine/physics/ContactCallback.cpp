#include "pch.h"
#include "ContactCallback.h"
#include <algorithm>

btScalar ContactCallback::addSingleResult(btManifoldPoint& cp,
	const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
	const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
{
	// Not in vector and valid
	int entity = colObj1Wrap->getCollisionObject()->getUserIndex();
	if (std::find(this->entitiesHit.begin(), this->entitiesHit.end(), entity) == this->entitiesHit.end() &&
		this->scene->entityValid(entity))
	{
		this->entitiesHit.push_back(entity);
	}
	return btScalar();
}
