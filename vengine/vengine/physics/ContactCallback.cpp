#include "ContactCallback.h"

btScalar ContactCallback::addSingleResult(btManifoldPoint& cp,
	const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
	const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
{
	// Not in vector
	if (std::find(this->entitiesHit.begin(), this->entitiesHit.end(), colObj1Wrap->getCollisionObject()->getUserIndex()) == this->entitiesHit.end())
	{
		this->entitiesHit.push_back(colObj1Wrap->getCollisionObject()->getUserIndex());
	}
	return btScalar();
}
