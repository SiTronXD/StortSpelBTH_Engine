#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "LinearMath/btAlignedObjectArray.h"

#include <glm/glm.hpp>
#include "Time.hpp"

#include "../components/MeshComponent.hpp"
#include "../components/Transform.hpp"
#include "SceneHandler.hpp"

struct Rays
{
	btVector3 pos;
	btVector3 dir;
};

class PhysicsEngine
{
private:

	btDefaultCollisionConfiguration* collconfig;
	btCollisionDispatcher* collDisp;
	btBroadphaseInterface* bpInterface;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynWorld;

	btAlignedObjectArray<btCollisionShape*> colShapes;
	std::vector<Rays> rayVec;

	float timer;
	const float timeStep;

public:

	PhysicsEngine();
	~PhysicsEngine();

	void update(Scene& scene, int id);

	void createRigidBody(glm::vec3 pos, float weight);
	void createRigidBody(glm::vec3 pos, glm::vec3 rot, float weight, btCollisionShape* shape);

	void createSphereCol(glm::vec3 pos, glm::vec3 rot, float radius);
	void createSphereCol(glm::vec3 pos, glm::vec3 rot, float radius, float weight);

	void createBoxCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents);
	void createBoxCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents, float weight);

	void createCapsuleCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents);
	void createCapsuleCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents, float weight);

	void shootRay(glm::vec3 pos, glm::vec3 dir, float distance = 300.f);

};