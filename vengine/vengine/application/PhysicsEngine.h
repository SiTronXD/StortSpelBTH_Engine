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

	void createRigidBody(glm::vec3 pos, float weight, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f));
	void createRigidBody(glm::vec3 pos, btCollisionShape* shape, float weight, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f), bool passThrough = false);

	void createSphereCol(glm::vec3 pos, float radius, bool passThrough = false, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f));
	void createSphereCol(glm::vec3 pos, float radius, float weight, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f), bool passThrough = false);

	void createBoxCol(glm::vec3 pos, glm::vec3 halfExtents, bool passThrough = false, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f));
	void createBoxCol(glm::vec3 pos, glm::vec3 halfExtents, float weight, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f), bool passThrough = false);

	void createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents, bool passThrough = false, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f));
	void createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents, float weight, glm::vec3 rot = glm::vec3(0.f, 0.f, 0.f), bool passThrough = false);

	void shootRay(glm::vec3 pos, glm::vec3 dir, float distance = 300.f);

	void applyForce(glm::vec3 force);
};