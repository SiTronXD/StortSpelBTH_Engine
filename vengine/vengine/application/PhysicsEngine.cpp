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

void PhysicsEngine::initPhysEngine(Scene& scene, int id)
{
	timer += Time::getDT();

	Transform& trans = scene.getComponent<Transform>(id);
	btVector3 hej = dynWorld->getCollisionObjectArray()[1]->getWorldTransform().getOrigin();
	btQuaternion holla = dynWorld->getCollisionObjectArray()[1]->getWorldTransform().getRotation();
	trans.position = { hej.x(), hej.y(), hej.z()};
	btScalar x, y, z;
	x = trans.rotation.x;
	y = trans.rotation.y;
	z = trans.rotation.z;
	float degreesToRadian = 180.f / 3.14f;
	x = x * degreesToRadian;
	y = y * degreesToRadian;
	z = z * degreesToRadian;
	holla.setEulerZYX(x, y, z);

	while (timer >= timeStep)
	{
		dynWorld->stepSimulation(btScalar(timeStep), 1);
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

void PhysicsEngine::createRigidBody(glm::vec3 pos, float weight, btCollisionShape* shape)
{
	btScalar mass(weight);
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0, 0, 0);
	if (isDynamic) { shape->calculateLocalInertia(mass, localInertia); }

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	dynWorld->addRigidBody(body);
}

void PhysicsEngine::createSphereCol(glm::vec3 pos, float radius)
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

void PhysicsEngine::createSphereCol(glm::vec3 pos, float radius, float weight)
{
	btCollisionShape* sphereShape = new btSphereShape(radius);
	colShapes.push_back(sphereShape);

	createRigidBody(pos, weight, sphereShape);
}

void PhysicsEngine::createBoxCol(glm::vec3 pos, glm::vec3 halfExtents)
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

void PhysicsEngine::createBoxCol(glm::vec3 pos, glm::vec3 halfExtents, float weight)
{
	btCollisionShape* boxShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(boxShape);

	createRigidBody(pos, weight, boxShape);
}

void PhysicsEngine::createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents)
{
	btCollisionShape* capsuleShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(capsuleShape);

	btCollisionObject* capsule = new btCollisionObject;
	capsule->setCollisionShape(capsuleShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	capsule->setWorldTransform(transform);
	dynWorld->addCollisionObject(capsule);
}

void PhysicsEngine::createCapsuleCol(glm::vec3 pos, glm::vec3 halfExtents, float weight)
{
	btCollisionShape* capsuleShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
	colShapes.push_back(capsuleShape);

	createRigidBody(pos, weight, capsuleShape);
}

void PhysicsEngine::removeFromArray()
{
	dynWorld->removeCollisionObject(dynWorld->getCollisionObjectArray()[1]);
}