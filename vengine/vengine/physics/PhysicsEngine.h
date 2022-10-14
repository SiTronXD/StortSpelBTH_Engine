#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/Transform.hpp"
#include "../application/SceneHandler.hpp"
#include "../components/BoxCollider.h"
#include "../components/SphereCollider.h"
#include "../components/CapsuleCollider.h"
#include "../components/RigidBody.h"

struct Ray
{
	btVector3 pos;
	btVector3 dir;
};

class PhysicsEngine
{
private:
	SceneHandler* sceneHandler;

	float timer;
	const float timeStep;

	btDefaultCollisionConfiguration* collconfig;
	btCollisionDispatcher* collDisp;
	btBroadphaseInterface* bpInterface;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynWorld;

	btAlignedObjectArray<btCollisionShape*> colShapes;

	std::vector<SphereCollider*> sphereVec;
	std::vector<BoxCollider*> boxVec;
	std::vector<CapsuleCollider*> capsuleVec;
	std::vector<RigidBody*> rigidBodyVec;
	std::vector<RigidBody*> rigidBodyNoColliderVec;

	// Update transforms and rotations for colliders.
	void updateSphere();
	void updateBox();
	void updateCapsule();
	void updateRigidBody();

	// Creation of rigid bodies and colliders
	void createRigidBody(RigidBody& rigidBody);
	void createRigidBody(RigidBody& rigidBody, SphereCollider& shape);
	void createRigidBody(RigidBody& rigidBody, BoxCollider& shape);
	void createRigidBody(RigidBody& rigidBody, CapsuleCollider& shape);

	void createSphereCol(SphereCollider& collider);
	void createBoxCol(BoxCollider& collider);
	void createCapsuleCol(CapsuleCollider& collider);

	bool removeSphereCollider(int ID);
	bool removeBoxCollider(int ID);
	bool removeCapsuleCollider(int ID);

public:

	PhysicsEngine();
	~PhysicsEngine();

	void setSceneHandler(SceneHandler* sceneHandler);

	void update();
	void shootRay(glm::vec3 pos, glm::vec3 dir, float distance = 100.f);
};