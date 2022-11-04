#pragma once

#include <btBulletCollisionCommon.h>
#include <vector>
#include "CallbackType.h"

class CollisionDispatcher : public btCollisionDispatcher
{
private:
	std::vector<CallbackType> types;
	std::vector<std::pair<int, int>> exits;

	virtual btPersistentManifold* getNewManifold(const btCollisionObject* b0, const btCollisionObject* b1) override;
	virtual void releaseManifold(btPersistentManifold* manifold) override;
public:
	CollisionDispatcher(btCollisionConfiguration* collisionConfiguration);
	virtual ~CollisionDispatcher();

	inline std::vector<CallbackType>& getTypes() { return types; }
	inline std::vector<std::pair<int, int>>& getExits() { return exits; }
};

