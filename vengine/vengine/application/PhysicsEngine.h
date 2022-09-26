#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include "Time.hpp"

#include "../components/MeshComponent.hpp"
#include "../components/Transform.hpp"
#include "SceneHandler.hpp"

class PhysicsEngine
{
private:

	btDefaultCollisionConfiguration* collconfig;
	btCollisionDispatcher* collDisp;
	btBroadphaseInterface* bpInterface;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynWorld;

	btAlignedObjectArray<btCollisionShape*> colShapes;

	float timer;
	const float timeStep;

public:

	PhysicsEngine();
	~PhysicsEngine();

	void initPhysEngine(Scene& scene, int id);

	void createRigidBody(glm::vec3 pos, float weight);
	void createRigidBody(glm::vec3 pos, float weight, btCollisionShape* shape);

	void createSphereCol(glm::vec3 pos, float radius);
	void createSphereCol(glm::vec3 pos, float radius, float weight);

	void createBoxCol(glm::vec3 pos, glm::vec3 halfExtents);
	void createBoxCol(glm::vec3 pos, glm::vec3 halfExtents, float weight);

	void createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents);
	void createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents, float weight);

	void removeFromArray();

};