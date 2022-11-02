#include "CollisionDispatcher.h"

btPersistentManifold* CollisionDispatcher::getNewManifold(const btCollisionObject* b0, const btCollisionObject* b1)
{
	btPersistentManifold* man = btCollisionDispatcher::getNewManifold(b0, b1);

	int num = this->getNumManifolds();
	this->types.resize(num);
	this->types[(size_t)num - 1] = CallbackType::ENTER;

	return man;
}

void CollisionDispatcher::releaseManifold(btPersistentManifold* manifold)
{
	this->exits.push_back(std::pair<int, int>(
		manifold->getBody0()->getUserIndex(), 
		manifold->getBody1()->getUserIndex()));
	btCollisionDispatcher::releaseManifold(manifold);
}

CollisionDispatcher::CollisionDispatcher(btCollisionConfiguration* collisionConfiguration)
	: btCollisionDispatcher(collisionConfiguration)
{
}

CollisionDispatcher::~CollisionDispatcher()
{
}
