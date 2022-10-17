#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#include "BulletHelper.hpp"

#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/Transform.hpp"
#include "../application/SceneHandler.hpp"
#include "../components/BoxCollider.h"
#include "../components/SphereCollider.h"
#include "../components/CapsuleCollider.h"

#include "../components/Rigidbody.h"
#include "../components/Collider.h"

struct Ray
{
	glm::vec3 pos;
	glm::vec3 dir;
};

class PhysicsEngine
{
private:
	SceneHandler* sceneHandler;

	float timer;
	const float TIMESTEP = 1.0f / 30.0f;

	btCollisionDispatcher* collDisp;
	btBroadphaseInterface* bpInterface;
	btSequentialImpulseConstraintSolver* solver;
	btDefaultCollisionConfiguration* collconfig;
	btDiscreteDynamicsWorld* dynWorld;

	btAlignedObjectArray<btCollisionShape*> colShapes;

	std::vector<SphereCollider*> sphereVec;
	std::vector<BoxCollider*> boxVec;
	std::vector<CapsuleCollider*> capsuleVec;
	std::vector<Rigidbody*> rigidBodyVec;
	std::vector<Rigidbody*> rigidBodyNoColliderVec;

	std::vector<Collider*> colVector;
	std::vector<Rigidbody*> rbVector;

	// Update transforms and rotations for colliders.
	//void updateSphere();
	//void updateBox();
	//void updateCapsule();

	//// Creation of rigid bodies and colliders
	//void createRigidBody(Rigidbody& rigidBody);
	//void createRigidBody(Rigidbody& rigidBody, SphereCollider& shape);
	//void createRigidBody(Rigidbody& rigidBody, BoxCollider& shape);
	//void createRigidBody(Rigidbody& rigidBody, CapsuleCollider& shape);

	//void createSphereCol(SphereCollider& collider);
	//void createBoxCol(BoxCollider& collider);
	//void createCapsuleCol(CapsuleCollider& collider);

	//bool removeSphereCollider(int ID);
	//bool removeBoxCollider(int ID);
	//bool removeCapsuleCollider(int ID);

	void updateColliders();
	void updateRigidbodies();

	void createCollider(Collider& col);
	void createRigidbody(Rigidbody& rb, Collider& col);

public:

	PhysicsEngine();
	~PhysicsEngine();

	void setSceneHandler(SceneHandler* sceneHandler);

	void update();
	bool shootRay(Ray ray, float maxDist = 100.0f);
};