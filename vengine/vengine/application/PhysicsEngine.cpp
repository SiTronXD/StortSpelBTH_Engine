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

btVector3 from(10, 0, 0);
btVector3 to(20, 0, 0);

void PhysicsEngine::update(Scene& scene, int id)
{
	timer += Time::getDT();

	Transform& trans = scene.getComponent<Transform>(id);
	btVector3 colTrans = dynWorld->getCollisionObjectArray()[1]->getWorldTransform().getOrigin();
	btQuaternion colRot = dynWorld->getCollisionObjectArray()[1]->getWorldTransform().getRotation();
	trans.position = { colTrans.x(), colTrans.y(), colTrans.z()};
	btScalar x, y, z;
	colRot.getEulerZYX(y, x, z);
	trans.rotation.x = glm::degrees(z);
	trans.rotation.y = glm::degrees(x);
	trans.rotation.z = glm::degrees(y);
	
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

			// ###### CHECK IF NON COLLIDABLE OBJECT COLLIDED
			//int numManifolds = dynWorld->getDispatcher()->getNumManifolds();
			//for (size_t i = 0; i < numManifolds; i++)
			//{
			//	if (dynWorld->getDispatcher()->getManifoldByIndexInternal(i))
			//	{
			//		std::cout << "DUDSADASD%%%%%%%%%%%%%%%%%%%" << std::endl;
			//	}
			//}

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

		}
		timer -= timeStep;
		std::cout << std::to_string(timer) << std::endl;
	}
}

void PhysicsEngine::createRigidBody(glm::vec3 pos, float weight, glm::vec3 rot)
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

void PhysicsEngine::createRigidBody(glm::vec3 pos, btCollisionShape* shape, float weight, glm::vec3 rot, bool passThrough)
{
	btScalar mass(weight);
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0, 0, 0);
	if (isDynamic) { shape->calculateLocalInertia(mass, localInertia); }

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
	btScalar x, y, z;
	x = glm::radians(rot.x);
	y = glm::radians(rot.y);
	z = glm::radians(rot.z);
	btQuaternion rotation(y, x, z);
	transform.setRotation(rotation);

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	if (passThrough)
	{
		body->setCollisionFlags(4);
	}

	dynWorld->addRigidBody(body);
}

void PhysicsEngine::createSphereCol(glm::vec3 pos, float radius, bool passThrough, glm::vec3 rot)
{
	btCollisionShape* sphereShape = new btSphereShape(radius);
	colShapes.push_back(sphereShape);

	btCollisionObject* sphere = new btCollisionObject;
	if (passThrough)
	{
		sphere->setCollisionFlags(4);
	}
	sphere->setCollisionShape(sphereShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	sphere->setWorldTransform(transform);
	dynWorld->addCollisionObject(sphere);
}

void PhysicsEngine::createSphereCol(glm::vec3 pos, float radius, float weight, glm::vec3 rot, bool passThrough)
{
	btCollisionShape* sphereShape = new btSphereShape(radius);
	colShapes.push_back(sphereShape);

	createRigidBody(pos, sphereShape, weight, rot, passThrough);
}

void PhysicsEngine::createBoxCol(glm::vec3 pos, glm::vec3 halfExtents, bool passThrough, glm::vec3 rot)
{
	btCollisionShape* boxShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(boxShape);
	
	btCollisionObject* box = new btCollisionObject;
	if (passThrough)
	{
		box->setCollisionFlags(4);
	}
	box->setCollisionShape(boxShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	box->setWorldTransform(transform);
	dynWorld->addCollisionObject(box);
}

void PhysicsEngine::createBoxCol(glm::vec3 pos, glm::vec3 halfExtents, float weight, glm::vec3 rot, bool passThrough)
{
	btCollisionShape* boxShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(boxShape);

	createRigidBody(pos, boxShape, weight, rot, passThrough);
}

void PhysicsEngine::createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents, bool passThrough, glm::vec3 rot)
{
	btCollisionShape* capsuleShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(capsuleShape);

	btCollisionObject* capsule = new btCollisionObject;
	if (passThrough)
	{
		capsule->setCollisionFlags(4);
	}
	capsule->setCollisionShape(capsuleShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
	btScalar x, y, z;
	x = glm::radians(rot.y);
	y = glm::radians(rot.x);
	z = glm::radians(rot.z);
	btQuaternion rotation(x, y, z);
	transform.setRotation(rotation);

	capsule->setWorldTransform(transform);
	dynWorld->addCollisionObject(capsule);
}

void PhysicsEngine::createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents, float weight, glm::vec3 rot, bool passThrough)
{
	btCollisionShape* capsuleShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(capsuleShape);

	createRigidBody(pos, capsuleShape, weight, rot, passThrough);
}

void PhysicsEngine::shootRay(glm::vec3 pos, glm::vec3 dir, float distance)
{
	dir = glm::normalize(dir);
	btVector3 btPos = { pos.x, pos.y, pos.z };
	btVector3 btEndPos = { pos.x + (distance * dir.x), pos.y + (distance * dir.y), pos.z + (distance * dir.z) };
	Rays ray = {ray.pos = btPos, ray.dir = btEndPos};

	btCollisionWorld::ClosestRayResultCallback closestResults(ray.pos, ray.dir);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

	dynWorld->rayTest(ray.pos, ray.dir, closestResults);

	if (closestResults.hasHit())
	{
		btVector3 p = ray.pos.lerp(ray.dir, closestResults.m_closestHitFraction);
		std::cout << "########## BIG HIT ##########" << std::endl;
		// p.x() p.y() p.z() Returns position which the ray has hit.
	}
}

void PhysicsEngine::applyForce(glm::vec3 force)
{
	btVector3 btForce = { force.x, force.y, force.z };
	dynWorld->getNonStaticRigidBodies().at(0)->applyForce(btForce, btVector3(0, 0, 0));
}