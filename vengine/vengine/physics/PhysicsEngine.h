#pragma once

#include <glm/glm.hpp>
#include <LinearMath/btAlignedObjectArray.h>

#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/Transform.hpp"
#include "../components/Rigidbody.h"
#include "../components/Collider.h"

class SceneHandler;
class btCollisionShape;
class btCollisionObject;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;

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

	// Raycast into the scene and get resulting hit
	RayPayload raycast(Ray ray, float maxDist = 100.0f);
	// Test contact of a collider within the scene. Get the resulting entities hit
	// NOTE: The collider argument doesn't need to be attached to an entity for the function to work
	std::vector<int> testContact(Collider& col, glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f));
};