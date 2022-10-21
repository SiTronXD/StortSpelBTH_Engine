#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#include "BulletHelper.hpp"

#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/Transform.hpp"
#include "../components/BoxCollider.h"
#include "../components/SphereCollider.h"
#include "../components/CapsuleCollider.h"

#include "../components/Rigidbody.h"
#include "../components/Collider.h"

class SceneHandler;

struct Ray
{
	glm::vec3 pos;
	glm::vec3 dir;
};

struct RayPayload
{
	bool hit = false;
	int entity = -1;
	glm::vec3 hitPoint = glm::vec3(0.0f);
	glm::vec3 hitNormal = glm::vec3(0.0f);
};

class PhysicsEngine
{
private:
	struct ColShapeInfo
	{
		btCollisionShape* shape = nullptr;
		ColType type;
		float radius;
		float height;
		glm::vec3 extents;
	};

	SceneHandler* sceneHandler;

	float timer;
	const float TIMESTEP = 1.0f / 30.0f;

	btCollisionDispatcher* collDisp;
	btBroadphaseInterface* bpInterface;
	btSequentialImpulseConstraintSolver* solver;
	btDefaultCollisionConfiguration* collconfig;
	btDiscreteDynamicsWorld* dynWorld;

	btAlignedObjectArray<ColShapeInfo> colShapes;

	std::vector<int> removeIndicies;

	/*std::vector<SphereCollider*> sphereVec;
	std::vector<BoxCollider*> boxVec;
	std::vector<CapsuleCollider*> capsuleVec;
	std::vector<Rigidbody*> rigidBodyVec;
	std::vector<Rigidbody*> rigidBodyNoColliderVec;

	std::vector<Collider*> colVector;
	std::vector<Rigidbody*> rbVector;*/

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

	btCollisionShape* createShape(const int& entity, Collider& col);
	void createCollider(const int& entity, Collider& col);
	void createRigidbody(const int& entity, Rigidbody& rb, Collider& col);

	void removeObject(btCollisionObject* obj, int index);
	
	void cleanup();
public:

	PhysicsEngine();
	~PhysicsEngine();

	void setSceneHandler(SceneHandler* sceneHandler);

	void init();
	void update();
	RayPayload shootRay(Ray ray, float maxDist = 100.0f);
};