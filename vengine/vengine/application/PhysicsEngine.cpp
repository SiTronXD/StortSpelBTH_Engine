#include "PhysicsEngine.h"
#include <iostream>
#include <string>

PhysicsEngine::PhysicsEngine()
	:colShapes(), timer(0.f), timeStep(1.f / 30.f)
{
	this->collconfig = new btDefaultCollisionConfiguration();
	this->collDisp = new btCollisionDispatcher(this->collconfig);
	this->bpInterface = new btDbvtBroadphase();
	this->solver = new btSequentialImpulseConstraintSolver;
	this->dynWorld = new btDiscreteDynamicsWorld(this->collDisp, this->bpInterface, this->solver, this->collconfig);
	this->dynWorld->setGravity(btVector3(0, -10, 0));
}

PhysicsEngine::~PhysicsEngine()
{
	//remove the rigidbodies from the dynamics world and delete them
	for (int i = this->dynWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = this->dynWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		this->dynWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < this->colShapes.size(); j++)
	{
		btCollisionShape* shape = this->colShapes[j];
		this->colShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete this->dynWorld;

	//delete solver
	delete this->solver;

	//delete broadphase
	delete this->bpInterface;

	//delete dispatcher
	delete this->collDisp;

	delete this->collconfig;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	this->colShapes.clear();
}

void PhysicsEngine::update()
{
	updateSphere();
	updateBox();
	updateCapsule();
	updateRigidBody();
	
	this->timer += Time::getDT();
	while (this->timer >= this->timeStep)
	{
		this->dynWorld->stepSimulation(btScalar(this->timeStep), 1);
		this->dynWorld->updateAabbs();
		this->dynWorld->computeOverlappingPairs();

		for (int j = this->dynWorld->getNumCollisionObjects() - 1; j > -1; j--)
		{
			btCollisionObject* obj = this->dynWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;

			if (capsuleVec.size() != 0 && rigidBodyVec.size() != 0)
			{
				//std::cout << dynWorld->getNonStaticRigidBodies()[0]->getCollisionShape()->getUserIndex() << std::endl;
			}

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
		this->timer -= this->timeStep;
	}
}

void PhysicsEngine::updateSphere()
{
	auto view = this->sceneHandler->getScene()->getSceneReg().view<SphereCollider>();
	auto foo = [&](const auto& entity, SphereCollider& sphere)
	{
		if (sphere.ID == -1)
		{
			if (this->sceneHandler->getScene()->hasComponents<RigidBody>((int)entity))
			{
				RigidBody& rigidBody = this->sceneHandler->getScene()->getComponent<RigidBody>((int)entity);
				createRigidBody(rigidBody, sphere);
			}
			else { createSphereCol(sphere); }
		}
		else if (this->sceneHandler->getScene()->hasComponents<Transform>((int)entity))
		{
			Transform& transform = this->sceneHandler->getScene()->getComponent<Transform>((int)entity);
			btVector3 colTrans = this->dynWorld->getCollisionObjectArray()[sphere.ID]->getWorldTransform().getOrigin();
			btQuaternion colRot = this->dynWorld->getCollisionObjectArray()[sphere.ID]->getWorldTransform().getRotation();
			transform.position = { colTrans.x(), colTrans.y(), colTrans.z() };
			btScalar x, y, z;
			colRot.getEulerZYX(y, x, z);
			transform.rotation.x = glm::degrees(z);
			transform.rotation.y = glm::degrees(x);
			transform.rotation.z = glm::degrees(y);
		}
	};
	view.each(foo);
}

void PhysicsEngine::updateBox()
{
	auto view = this->sceneHandler->getScene()->getSceneReg().view<BoxCollider>();
	auto foo = [&](const auto& entity, BoxCollider& box)
	{
		if (box.ID == -1)
		{
			if (this->sceneHandler->getScene()->hasComponents<RigidBody>((int)entity))
			{
				RigidBody& rigidBody = this->sceneHandler->getScene()->getComponent<RigidBody>((int)entity);
				createRigidBody(rigidBody, box);
			}
			else { createBoxCol(box); }
		}
		else if (this->sceneHandler->getScene()->hasComponents<Transform>((int)entity))
		{
			Transform& transform = this->sceneHandler->getScene()->getComponent<Transform>((int)entity);
			btVector3 colTrans = this->dynWorld->getCollisionObjectArray()[box.ID]->getWorldTransform().getOrigin();
			btQuaternion colRot = this->dynWorld->getCollisionObjectArray()[box.ID]->getWorldTransform().getRotation();
			transform.position = { colTrans.x(), colTrans.y(), colTrans.z() };
			btScalar x, y, z;
			colRot.getEulerZYX(y, x, z);
			transform.rotation.x = glm::degrees(z);
			transform.rotation.y = glm::degrees(x);
			transform.rotation.z = glm::degrees(y);
		}
	};
	view.each(foo);
}

void PhysicsEngine::updateCapsule()
{
	auto view = this->sceneHandler->getScene()->getSceneReg().view<CapsuleCollider>();
	auto foo = [&](const auto& entity, CapsuleCollider& capsule)
	{
		if (capsule.ID == -1)
		{
			if (this->sceneHandler->getScene()->hasComponents<RigidBody>((int)entity))
			{
				RigidBody& rigidBody = this->sceneHandler->getScene()->getComponent<RigidBody>((int)entity);
				createRigidBody(rigidBody, capsule);
			}
			else { createCapsuleCol(capsule); }
		}
		else if (this->sceneHandler->getScene()->hasComponents<Transform>((int)entity))
		{
			Transform& transform = this->sceneHandler->getScene()->getComponent<Transform>((int)entity);
			btVector3 colTrans = this->dynWorld->getCollisionObjectArray()[capsule.ID]->getWorldTransform().getOrigin();
			btQuaternion colRot = this->dynWorld->getCollisionObjectArray()[capsule.ID]->getWorldTransform().getRotation();
			transform.position = { colTrans.x(), colTrans.y(), colTrans.z() };
			btScalar x, y, z;
			colRot.getEulerZYX(y, x, z);
			transform.rotation.x = glm::degrees(z);
			transform.rotation.y = glm::degrees(x);
			transform.rotation.z = glm::degrees(y);
		}
	};
	view.each(foo);
}

void PhysicsEngine::updateRigidBody()
{
	auto view = this->sceneHandler->getScene()->getSceneReg().view<Transform, RigidBody>();
	auto foo = [&](const auto& entity, Transform& transform, RigidBody& rigidBody)
	{
		if (rigidBody.ID == -1)
		{
			if (this->sceneHandler->getScene()->hasComponents<SphereCollider>((int)entity))
			{
				SphereCollider& sphere = this->sceneHandler->getScene()->getComponent<SphereCollider>((int)entity);
				createRigidBody(rigidBody, sphere);
			}
			else if (this->sceneHandler->getScene()->hasComponents<BoxCollider>((int)entity))
			{
				BoxCollider& box = this->sceneHandler->getScene()->getComponent<BoxCollider>((int)entity);
				createRigidBody(rigidBody, box);
			}
			else if (this->sceneHandler->getScene()->hasComponents<CapsuleCollider>((int)entity))
			{
				CapsuleCollider& capsule = this->sceneHandler->getScene()->getComponent<CapsuleCollider>((int)entity);
				createRigidBody(rigidBody, capsule);
			}
			else { createRigidBody(rigidBody); }
		}
		else
		{
			//btTransform hej = dynWorld->getNonStaticRigidBodies().at(0)->getWorldTransform();
			//btVector3 colTrans = this->dynWorld->getCollisionObjectArray()[rigidBody.ID]->getWorldTransform().getOrigin();
			//btQuaternion colRot = this->dynWorld->getCollisionObjectArray()[rigidBody.ID]->getWorldTransform().getRotation();
			//transform.position = { colTrans.x(), colTrans.y(), colTrans.z() };
			//btScalar x, y, z;
			//colRot.getEulerZYX(y, x, z);
			//transform.rotation.x = glm::degrees(z);
			//transform.rotation.y = glm::degrees(x);
			//transform.rotation.z = glm::degrees(y);
		}
	};
	view.each(foo);
}

void PhysicsEngine::createRigidBody(RigidBody& rigidBody)
{
	rigidBodyNoColliderVec.push_back(&rigidBody);
}

void PhysicsEngine::createRigidBody(RigidBody& rigidBody, SphereCollider& collider)
{
	if (collider.ID != -1)
	{
		dynWorld->removeCollisionObject(dynWorld->getCollisionObjectArray().at(collider.ID));
	}

	btCollisionShape* sphereShape = new btSphereShape(collider.radius);
	colShapes.push_back(sphereShape);

	btScalar mass(rigidBody.weight);
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0, 0, 0);
	if (isDynamic) { sphereShape->calculateLocalInertia(mass, localInertia); }

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(rigidBody.pos.x, rigidBody.pos.y, rigidBody.pos.z));
	btScalar x, y, z;
	x = glm::radians(rigidBody.rot.x);
	y = glm::radians(rigidBody.rot.y);
	z = glm::radians(rigidBody.rot.z);
	btQuaternion rotation(y, x, z);
	transform.setRotation(rotation);

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, sphereShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	if (rigidBody.isTrigger)
	{
		body->setCollisionFlags(4);
	}

	this->dynWorld->addRigidBody(body);
	rigidBody.ID = this->dynWorld->getNonStaticRigidBodies().size() - 1;
	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
	this->rigidBodyVec.push_back(&rigidBody);
}

void PhysicsEngine::createRigidBody(RigidBody& rigidBody, BoxCollider& collider)
{
	if (collider.ID != -1)
	{
		dynWorld->removeCollisionObject(dynWorld->getCollisionObjectArray().at(collider.ID));
	}

	btCollisionShape* boxShape = new btBoxShape(btVector3(collider.halfExtents.x, collider.halfExtents.y, collider.halfExtents.z));
	colShapes.push_back(boxShape);

	btScalar mass(rigidBody.weight);
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0, 0, 0);
	if (isDynamic) { boxShape->calculateLocalInertia(mass, localInertia); }

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(rigidBody.pos.x, rigidBody.pos.y, rigidBody.pos.z));
	btScalar x, y, z;
	x = glm::radians(rigidBody.rot.x);
	y = glm::radians(rigidBody.rot.y);
	z = glm::radians(rigidBody.rot.z);
	btQuaternion rotation(y, x, z);
	transform.setRotation(rotation);

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	if (rigidBody.isTrigger)
	{
		body->setCollisionFlags(4);
	}

	this->dynWorld->addRigidBody(body);
	rigidBody.ID = this->dynWorld->getNonStaticRigidBodies().size() - 1;
	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
	this->rigidBodyVec.push_back(&rigidBody);
}

void PhysicsEngine::createRigidBody(RigidBody& rigidBody, CapsuleCollider& collider)
{
	if (collider.ID != -1)
	{
		dynWorld->removeCollisionObject(dynWorld->getCollisionObjectArray().at(collider.ID));
	}

	btCollisionShape* capsuleShape = new btCapsuleShape(collider.radius, collider.height);
	colShapes.push_back(capsuleShape);

	btScalar mass(rigidBody.weight);
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0, 0, 0);
	if (isDynamic) { capsuleShape->calculateLocalInertia(mass, localInertia); }

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(rigidBody.pos.x, rigidBody.pos.y, rigidBody.pos.z));
	btScalar x, y, z;
	x = glm::radians(rigidBody.rot.x);
	y = glm::radians(rigidBody.rot.y);
	z = glm::radians(rigidBody.rot.z);
	btQuaternion rotation(y, x, z);
	transform.setRotation(rotation);

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, capsuleShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	if (rigidBody.isTrigger)
	{
		body->setCollisionFlags(4);
	}

	this->dynWorld->addRigidBody(body);
	rigidBody.ID = this->dynWorld->getNonStaticRigidBodies().size() - 1;
	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
	this->rigidBodyVec.push_back(&rigidBody);
}

void PhysicsEngine::createSphereCol(SphereCollider& collider)
{
	btCollisionShape* sphereShape = new btSphereShape(collider.radius);
	this->colShapes.push_back(sphereShape);

	btCollisionObject* sphere = new btCollisionObject;
	if (collider.isTrigger)
	{
		sphere->setCollisionFlags(4);
	}
	sphere->setCollisionShape(sphereShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(collider.pos.x, collider.pos.y, collider.pos.z));
	btScalar x, y, z;
	x = glm::radians(collider.rot.x);
	y = glm::radians(collider.rot.y);
	z = glm::radians(collider.rot.z);
	btQuaternion rotation(y, x, z);
	transform.setRotation(rotation);

	sphere->setWorldTransform(transform);
	this->dynWorld->addCollisionObject(sphere);

	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
	sphereVec.emplace_back(&collider);
}

void PhysicsEngine::createBoxCol(BoxCollider& collider)
{
	btCollisionShape* boxShape = new btBoxShape(btVector3(collider.halfExtents.x, collider.halfExtents.y, collider.halfExtents.z));
	this->colShapes.push_back(boxShape);

	btCollisionObject* box = new btCollisionObject;
	if (collider.isTrigger)
	{
		box->setCollisionFlags(4);
	}
	box->setCollisionShape(boxShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(collider.pos.x, collider.pos.y, collider.pos.z));
	btScalar x, y, z;
	x = glm::radians(collider.rot.x);
	y = glm::radians(collider.rot.y);
	z = glm::radians(collider.rot.z);
	btQuaternion rotation(y, x, z);
	transform.setRotation(rotation);

	box->setWorldTransform(transform);
	dynWorld->addCollisionObject(box);

	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
	boxVec.emplace_back(&collider);
}

void PhysicsEngine::createCapsuleCol(CapsuleCollider& collider)
{
	btCollisionShape* capsuleShape = new btCapsuleShape(collider.radius, collider.radius);
	this->colShapes.push_back(capsuleShape);

	btCollisionObject* capsule = new btCollisionObject;
	if (collider.isTrigger)
	{
		capsule->setCollisionFlags(4);
	}
	capsule->setCollisionShape(capsuleShape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(collider.pos.x, collider.pos.y, collider.pos.z));
	btScalar x, y, z;
	x = glm::radians(collider.rot.y);
	y = glm::radians(collider.rot.x);
	z = glm::radians(collider.rot.z);
	btQuaternion rotation(x, y, z);
	transform.setRotation(rotation);
	capsule->setWorldTransform(transform);

	this->dynWorld->addCollisionObject(capsule);
	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
	this->capsuleVec.emplace_back(&collider);
}

bool PhysicsEngine::removeSphereCollider(int ID)
{
	int tempID = this->dynWorld->getCollisionObjectArray().size() - 1;
	delete this->dynWorld->getCollisionObjectArray()[ID];

	for (size_t i = 0; i < sphereVec.size(); i++)
	{
		if (this->sphereVec.back()->ID == tempID)
		{
			this->sphereVec.pop_back();
		}
		else if (this->sphereVec[i]->ID == tempID)
		{
			std::swap(this->sphereVec[ID], this->sphereVec[i]);
			this->sphereVec.pop_back();
			return true;
		}
	}
	return false;
}

bool PhysicsEngine::removeBoxCollider(int ID)
{
	int tempID = this->dynWorld->getCollisionObjectArray().size() - 1;
	delete this->dynWorld->getCollisionObjectArray()[ID];

	for (size_t i = 0; i < boxVec.size(); i++)
	{
		if (this->boxVec.back()->ID == tempID)
		{
			this->boxVec.pop_back();
		}
		else if (this->boxVec[i]->ID == tempID)
		{
			std::swap(this->boxVec[ID], this->boxVec[i]);
			this->boxVec.pop_back();
			return true;
		}
	}
	return false;
}

bool PhysicsEngine::removeCapsuleCollider(int ID)
{
	int tempID = this->dynWorld->getCollisionObjectArray().size() - 1;
	delete this->dynWorld->getCollisionObjectArray()[ID];

	for (size_t i = 0; i < capsuleVec.size(); i++)
	{
		if (this->capsuleVec.back()->ID == tempID)
		{
			this->capsuleVec.pop_back();
		}
		else if (this->capsuleVec[i]->ID == tempID)
		{
			std::swap(this->capsuleVec[ID], this->capsuleVec[i]);
			this->capsuleVec.pop_back();
			return true;
		}
	}
	return false;
}

void PhysicsEngine::shootRay(glm::vec3 pos, glm::vec3 dir, float distance)
{
	dir = glm::normalize(dir);
	btVector3 btPos = { pos.x, pos.y, pos.z };
	btVector3 btEndPos = { pos.x + (distance * dir.x), pos.y + (distance * dir.y), pos.z + (distance * dir.z) };
	Rays ray = {ray.pos = btPos, ray.dir = btEndPos};

	btCollisionWorld::ClosestRayResultCallback closestResults(ray.pos, ray.dir);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

	this->dynWorld->rayTest(ray.pos, ray.dir, closestResults);

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
	this->dynWorld->getNonStaticRigidBodies().at(0)->applyForce(btForce, btVector3(0, 0, 0));
}

void PhysicsEngine::setSceneHandler(SceneHandler* sceneHandler)
{
	this->sceneHandler = sceneHandler;
}