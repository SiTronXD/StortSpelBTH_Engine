#include "PhysicsEngine.h"
#include <iostream>
#include <string>

PhysicsEngine::PhysicsEngine()
	:colShapes(), timer(0.f), timeStep(1.f / 30.f)
{
	collconfig = new btDefaultCollisionConfiguration();
	collDisp = new btCollisionDispatcher(collconfig);
	bpInterface = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynWorld = new btDiscreteDynamicsWorld(collDisp, bpInterface, solver, collconfig);
	dynWorld->setGravity(btVector3(0, -10, 0));
}

PhysicsEngine::~PhysicsEngine()
{
	//remove the rigidbodies from the dynamics world and delete them
	for (int i = dynWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < colShapes.size(); j++)
	{
		btCollisionShape* shape = colShapes[j];
		colShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete dynWorld;

	//delete solver
	delete solver;

	//delete broadphase
	delete bpInterface;

	//delete dispatcher
	delete collDisp;

	delete collconfig;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	colShapes.clear();
}

constexpr float radiansToDegrees = 180.f / 3.14f;
constexpr float degreesToRadians = 3.14f / 180.0f;
btVector3 from(10, 0, 0);
btVector3 to(20, 0, 0);

void PhysicsEngine::update(Scene& scene, int id)
{
	timer += Time::getDT();

	Transform& trans = scene.getComponent<Transform>(id);
	btVector3 hej = dynWorld->getCollisionObjectArray()[1]->getWorldTransform().getOrigin();
	btQuaternion holla = dynWorld->getCollisionObjectArray()[1]->getWorldTransform().getRotation();
	trans.position = { hej.x(), hej.y(), hej.z()};
	btScalar x, y, z;
	holla.getEulerZYX(y, x, z);
	trans.rotation.x = z * radiansToDegrees;
	trans.rotation.y = x * radiansToDegrees;
	trans.rotation.z = y * radiansToDegrees;
	
	while (timer >= timeStep)
	{
		dynWorld->stepSimulation(btScalar(timeStep), 1);
		dynWorld->updateAabbs();
		dynWorld->computeOverlappingPairs();

		for (int j = dynWorld->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = dynWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;
			if (body && body->getMotionState())
			{
				body->getMotionState()->getWorldTransform(trans);
			}
			else
			{
				trans = obj->getWorldTransform();
			}
			printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));

			static float up = 0.f;
			static float dir = 1.f;

			/////all hits
			//{
			//	btVector3 from(-30, 1 + up, 0);
			//	btVector3 to(30, 1, 0);
			//	btCollisionWorld::AllHitsRayResultCallback allResults(from, to);
			//	allResults.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
			//	//kF_UseGjkConvexRaytest flag is now enabled by default, use the faster but more approximate algorithm
			//	//allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;
			//	allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

			//	dynWorld->rayTest(from, to, allResults);

			//	for (int i = 0; i < allResults.m_hitFractions.size(); i++)
			//	{
			//		btVector3 p = from.lerp(to, allResults.m_hitFractions[i]);
			//	}
			//}

			///first hit
			{
				//btVector3 from(-30, 1.2, 0);
				//btVector3 to(30, 1.2, 0);

				for (size_t i = 0; i < rayVec.size(); i++)
				{
					btCollisionWorld::ClosestRayResultCallback closestResults(rayVec[i].pos, rayVec[i].dir);
					closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

					dynWorld->rayTest(rayVec[i].pos, rayVec[i].dir, closestResults);

					if (closestResults.hasHit())
					{
						btVector3 p = rayVec[i].pos.lerp(rayVec[i].dir, closestResults.m_closestHitFraction);
						std::cout << "########## BIG HIT ##########" << std::endl;
					}
				}
			}
		}
		timer -= timeStep;
		std::cout << std::to_string(timer) << std::endl;
	}
}

void PhysicsEngine::createRigidBody(glm::vec3 pos, float weight)
{
	btScalar mass(weight);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, NULL);
	btRigidBody* body = new btRigidBody(rbInfo);

	dynWorld->addRigidBody(body);
}

void PhysicsEngine::createRigidBody(glm::vec3 pos, glm::vec3 rot, float weight, btCollisionShape* shape)
{
	btScalar mass(weight);
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0, 0, 0);
	if (isDynamic) { shape->calculateLocalInertia(mass, localInertia); }

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
	btScalar x, y, z;
	x = rot.x * degreesToRadians;
	y = rot.y * degreesToRadians;
	z = rot.z * degreesToRadians;
	btQuaternion rotation(y, x, z);
	transform.setRotation(rotation);

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	dynWorld->addRigidBody(body);
}

void PhysicsEngine::createSphereCol(glm::vec3 pos, glm::vec3 rot, float radius)
{
	btCollisionShape* sphereShape = new btSphereShape(radius);
	colShapes.push_back(sphereShape);

	btCollisionObject* sphere = new btCollisionObject;
	sphere->setCollisionShape(sphereShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	sphere->setWorldTransform(transform);
	dynWorld->addCollisionObject(sphere);
}

void PhysicsEngine::createSphereCol(glm::vec3 pos, glm::vec3 rot, float radius, float weight)
{
	btCollisionShape* sphereShape = new btSphereShape(radius);
	colShapes.push_back(sphereShape);

	createRigidBody(pos, rot, weight, sphereShape);
}

void PhysicsEngine::createBoxCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents)
{
	btCollisionShape* boxShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(boxShape);

	btCollisionObject* box = new btCollisionObject;
	box->setCollisionShape(boxShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	box->setWorldTransform(transform);
	dynWorld->addCollisionObject(box);
}

void PhysicsEngine::createBoxCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents, float weight)
{
	btCollisionShape* boxShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(boxShape);

	createRigidBody(pos, rot, weight, boxShape);
}

void PhysicsEngine::createCapsuleCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents)
{
	btCollisionShape* capsuleShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(capsuleShape);

	btCollisionObject* capsule = new btCollisionObject;
	capsule->setCollisionShape(capsuleShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
	btScalar x, y, z;
	x = rot.y * degreesToRadians;
	y = rot.x * degreesToRadians;
	z = rot.z * degreesToRadians;
	btQuaternion rotation(x, y, z);
	transform.setRotation(rotation);

	capsule->setWorldTransform(transform);
	dynWorld->addCollisionObject(capsule);
}

void PhysicsEngine::createCapsuleCol(glm::vec3 pos, glm::vec3 rot, glm::vec3 halfExtents, float weight)
{
	btCollisionShape* capsuleShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(capsuleShape);

	createRigidBody(pos, rot, weight, capsuleShape);
}

void PhysicsEngine::shootRay(glm::vec3 pos, glm::vec3 dir, float distance)
{
	dir = glm::normalize(dir);
	btVector3 btPos = { pos.x, pos.y, pos.z };
	btVector3 btEndPos = { pos.x + (distance * dir.x), pos.y + (distance * dir.y), pos.z + (distance * dir.z) };
	Rays ray = {ray.pos = btPos, ray.dir = btEndPos};
	rayVec.emplace_back(ray);
}