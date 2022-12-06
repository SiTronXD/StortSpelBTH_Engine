#include "pch.h"
#include "ContactCallbackPair.h"
#include <algorithm>

btScalar ContactCallbackPair::addSingleResult(btManifoldPoint& cp,
	const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
	const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
{
	this->hit = true;
	return btScalar();
}
