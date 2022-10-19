#include "PhysicsEngine.h"
#include "../application/SceneHandler.hpp"
#include <iostream>
#include <string>

//void PhysicsEngine::updateSphere()
//{
//	auto view =
//	    this->sceneHandler->getScene()->getSceneReg().view<SphereCollider>();
//	auto foo = [&](const auto& entity, SphereCollider& sphere)
//	{
//		if (sphere.ID == -1)
//		{
//			if (this->sceneHandler->getScene()->hasComponents<RigidBody>((int
//			    )entity))
//			{
//				RigidBody& rigidBody =
//				    this->sceneHandler->getScene()->getComponent<RigidBody>((int
//				    )entity);
//				createRigidBody(rigidBody, sphere);
//			}
//			else
//			{
//				createSphereCol(sphere);
//			}
//		}
//		else if (this->sceneHandler->getScene()->hasComponents<Transform>((int
//		         )entity))
//		{
//			Transform& transform =
//			    this->sceneHandler->getScene()->getComponent<Transform>((int
//			    )entity);
//			btVector3 colTrans =
//			    this->dynWorld->getCollisionObjectArray()[sphere.ID]
//			        ->getWorldTransform()
//			        .getOrigin();
//			btQuaternion colRot =
//			    this->dynWorld->getCollisionObjectArray()[sphere.ID]
//			        ->getWorldTransform()
//			        .getRotation();
//			transform.position = {colTrans.x(), colTrans.y(), colTrans.z()};
//			btScalar x, y, z;
//			colRot.getEulerZYX(y, x, z);
//			transform.rotation.x = glm::degrees(z);
//			transform.rotation.y = glm::degrees(x);
//			transform.rotation.z = glm::degrees(y);
//		}
//	};
//	view.each(foo);
//}
//
//void PhysicsEngine::updateBox()
//{
//	auto view =
//	    this->sceneHandler->getScene()->getSceneReg().view<BoxCollider>();
//	auto foo = [&](const auto& entity, BoxCollider& box)
//	{
//		if (box.ID == -1)
//		{
//			if (this->sceneHandler->getScene()->hasComponents<RigidBody>((int
//			    )entity))
//			{
//				RigidBody& rigidBody =
//				    this->sceneHandler->getScene()->getComponent<RigidBody>((int
//				    )entity);
//				createRigidBody(rigidBody, box);
//			}
//			else
//			{
//				createBoxCol(box);
//			}
//		}
//		else if (this->sceneHandler->getScene()->hasComponents<Transform>((int
//		         )entity))
//		{
//			Transform& transform =
//			    this->sceneHandler->getScene()->getComponent<Transform>((int
//			    )entity);
//			btVector3 colTrans =
//			    this->dynWorld->getCollisionObjectArray()[box.ID]
//			        ->getWorldTransform()
//			        .getOrigin();
//			btQuaternion colRot =
//			    this->dynWorld->getCollisionObjectArray()[box.ID]
//			        ->getWorldTransform()
//			        .getRotation();
//			transform.position = {colTrans.x(), colTrans.y(), colTrans.z()};
//			btScalar x, y, z;
//			colRot.getEulerZYX(y, x, z);
//			transform.rotation.x = glm::degrees(z);
//			transform.rotation.y = glm::degrees(x);
//			transform.rotation.z = glm::degrees(y);
//		}
//	};
//	view.each(foo);
//}
//
//void PhysicsEngine::updateCapsule()
//{
//	auto view =
//	    this->sceneHandler->getScene()->getSceneReg().view<CapsuleCollider>();
//	auto foo = [&](const auto& entity, CapsuleCollider& capsule)
//	{
//		if (capsule.ID == -1)
//		{
//			if (this->sceneHandler->getScene()->hasComponents<RigidBody>((int
//			    )entity))
//			{
//				RigidBody& rigidBody =
//				    this->sceneHandler->getScene()->getComponent<RigidBody>((int
//				    )entity);
//				createRigidBody(rigidBody, capsule);
//			}
//			else
//			{
//				createCapsuleCol(capsule);
//			}
//		}
//		else if (this->sceneHandler->getScene()->hasComponents<Transform>((int
//		         )entity))
//		{
//			Transform& transform =
//			    this->sceneHandler->getScene()->getComponent<Transform>((int
//			    )entity);
//			btVector3 colTrans =
//			    this->dynWorld->getCollisionObjectArray()[capsule.ID]
//			        ->getWorldTransform()
//			        .getOrigin();
//			btQuaternion colRot =
//			    this->dynWorld->getCollisionObjectArray()[capsule.ID]
//			        ->getWorldTransform()
//			        .getRotation();
//			transform.position = {colTrans.x(), colTrans.y(), colTrans.z()};
//			btScalar x, y, z;
//			colRot.getEulerZYX(y, x, z);
//			transform.rotation.x = glm::degrees(z);
//			transform.rotation.y = glm::degrees(x);
//			transform.rotation.z = glm::degrees(y);
//		}
//	};
//	view.each(foo);
//}
//
//void PhysicsEngine::updateRigidBody()
//{
//	auto view = this->sceneHandler->getScene()
//	                ->getSceneReg()
//	                .view<Transform, RigidBody>();
//	auto foo =
//	    [&](const auto& entity, Transform& transform, RigidBody& rigidBody)
//	{
//		if (rigidBody.ID == -1)
//		{
//			if (this->sceneHandler->getScene()->hasComponents<SphereCollider>(
//			        (int)entity
//			    ))
//			{
//				SphereCollider& sphere =
//				    this->sceneHandler->getScene()
//				        ->getComponent<SphereCollider>((int)entity);
//				createRigidBody(rigidBody, sphere);
//			}
//			else if (this->sceneHandler->getScene()->hasComponents<BoxCollider>(
//			             (int)entity
//			         ))
//			{
//				BoxCollider& box =
//				    this->sceneHandler->getScene()->getComponent<BoxCollider>(
//				        (int)entity
//				    );
//				createRigidBody(rigidBody, box);
//			}
//			else if (this->sceneHandler->getScene()
//			             ->hasComponents<CapsuleCollider>((int)entity))
//			{
//				CapsuleCollider& capsule =
//				    this->sceneHandler->getScene()
//				        ->getComponent<CapsuleCollider>((int)entity);
//				createRigidBody(rigidBody, capsule);
//			}
//			else
//			{
//				createRigidBody(rigidBody);
//			}
//		}
//		else
//		{
//			// btTransform hej = dynWorld->getNonStaticRigidBodies().at(0)->getWorldTransform();
//			// btVector3 colTrans = this->dynWorld->getCollisionObjectArray()[rigidBody.ID]->getWorldTransform().getOrigin();
//			// btQuaternion colRot = this->dynWorld->getCollisionObjectArray()[rigidBody.ID]->getWorldTransform().getRotation();
//			// transform.position = { colTrans.x(), colTrans.y(), colTrans.z() };
//			// btScalar x, y, z;
//			// colRot.getEulerZYX(y, x, z);
//			// transform.rotation.x = glm::degrees(z);
//			// transform.rotation.y = glm::degrees(x);
//			// transform.rotation.z = glm::degrees(y);
//		}
//	};
//	view.each(foo);
//}
//
//void PhysicsEngine::createRigidBody(Rigidbody& rigidBody)
//{
//	rigidBodyNoColliderVec.push_back(&rigidBody);
//}
//
//void PhysicsEngine::createRigidBody(
//	Rigidbody& rigidBody, SphereCollider& collider
//)
//{
//	if (collider.ID != -1)
//	{
//		dynWorld->removeCollisionObject(
//		    dynWorld->getCollisionObjectArray().at(collider.ID)
//		);
//	}
//
//	btCollisionShape* sphereShape = new btSphereShape(collider.radius);
//	colShapes.push_back(sphereShape);
//
//	btScalar mass(rigidBody.weight);
//	bool isDynamic = (mass != 0.f);
//	btVector3 localInertia(0, 0, 0);
//	if (isDynamic)
//	{
//		sphereShape->calculateLocalInertia(mass, localInertia);
//	}
//
//	btTransform transform;
//	transform.setIdentity();
//	transform.setOrigin(
//	    btVector3(rigidBody.pos.x, rigidBody.pos.y, rigidBody.pos.z)
//	);
//	btScalar x, y, z;
//	x = glm::radians(rigidBody.rot.x);
//	y = glm::radians(rigidBody.rot.y);
//	z = glm::radians(rigidBody.rot.z);
//	btQuaternion rotation(y, x, z);
//	transform.setRotation(rotation);
//
//	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
//	btRigidBody::btRigidBodyConstructionInfo rbInfo(
//	    mass, motionState, sphereShape, localInertia
//	);
//	btRigidBody* body = new btRigidBody(rbInfo);
//	if (rigidBody.isTrigger)
//	{
//		body->setCollisionFlags(4);
//	}
//
//	this->dynWorld->addRigidBody(body);
//	rigidBody.ID = this->dynWorld->getNonStaticRigidBodies().size() - 1;
//	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	this->rigidBodyVec.push_back(&rigidBody);
//}
//
//void PhysicsEngine::createRigidBody(Rigidbody& rigidBody, BoxCollider& collider)
//{
//	if (collider.ID != -1)
//	{
//		dynWorld->removeCollisionObject(
//		    dynWorld->getCollisionObjectArray().at(collider.ID)
//		);
//	}
//
//	btCollisionShape* boxShape = new btBoxShape(btVector3(
//	    collider.halfExtents.x, collider.halfExtents.y, collider.halfExtents.z
//	));
//	colShapes.push_back(boxShape);
//
//	btScalar mass(rigidBody.weight);
//	bool isDynamic = (mass != 0.f);
//	btVector3 localInertia(0, 0, 0);
//	if (isDynamic)
//	{
//		boxShape->calculateLocalInertia(mass, localInertia);
//	}
//
//	btTransform transform;
//	transform.setIdentity();
//	transform.setOrigin(
//	    btVector3(rigidBody.pos.x, rigidBody.pos.y, rigidBody.pos.z)
//	);
//	btScalar x, y, z;
//	x = glm::radians(rigidBody.rot.x);
//	y = glm::radians(rigidBody.rot.y);
//	z = glm::radians(rigidBody.rot.z);
//	btQuaternion rotation(y, x, z);
//	transform.setRotation(rotation);
//
//	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
//	btRigidBody::btRigidBodyConstructionInfo rbInfo(
//	    mass, motionState, boxShape, localInertia
//	);
//	btRigidBody* body = new btRigidBody(rbInfo);
//	if (rigidBody.isTrigger)
//	{
//		body->setCollisionFlags(4);
//	}
//
//	this->dynWorld->addRigidBody(body);
//	rigidBody.ID = this->dynWorld->getNonStaticRigidBodies().size() - 1;
//	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	this->rigidBodyVec.push_back(&rigidBody);
//}
//
//void PhysicsEngine::createRigidBody(
//	Rigidbody& rigidBody, CapsuleCollider& collider
//)
//{
//	if (collider.ID != -1)
//	{
//		dynWorld->removeCollisionObject(
//		    dynWorld->getCollisionObjectArray().at(collider.ID)
//		);
//	}
//
//	btCollisionShape* capsuleShape =
//	    new btCapsuleShape(collider.radius, collider.height);
//	colShapes.push_back(capsuleShape);
//
//	btScalar mass(rigidBody.weight);
//	bool isDynamic = (mass != 0.f);
//	btVector3 localInertia(0, 0, 0);
//	if (isDynamic)
//	{
//		capsuleShape->calculateLocalInertia(mass, localInertia);
//	}
//
//	btTransform transform;
//	transform.setIdentity();
//	transform.setOrigin(
//	    btVector3(rigidBody.pos.x, rigidBody.pos.y, rigidBody.pos.z)
//	);
//	btScalar x, y, z;
//	x = glm::radians(rigidBody.rot.x);
//	y = glm::radians(rigidBody.rot.y);
//	z = glm::radians(rigidBody.rot.z);
//	btQuaternion rotation(y, x, z);
//	transform.setRotation(rotation);
//
//	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
//	btRigidBody::btRigidBodyConstructionInfo rbInfo(
//	    mass, motionState, capsuleShape, localInertia
//	);
//	btRigidBody* body = new btRigidBody(rbInfo);
//	if (rigidBody.isTrigger)
//	{
//		body->setCollisionFlags(4);
//	}
//
//	this->dynWorld->addRigidBody(body);
//	rigidBody.ID = this->dynWorld->getNonStaticRigidBodies().size() - 1;
//	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	this->rigidBodyVec.push_back(&rigidBody);
//}
//
//void PhysicsEngine::createSphereCol(SphereCollider& collider)
//{
//	btCollisionShape* sphereShape = new btSphereShape(collider.radius);
//	this->colShapes.push_back(sphereShape);
//
//	btCollisionObject* sphere = new btCollisionObject;
//	if (collider.isTrigger)
//	{
//		sphere->setCollisionFlags(4);
//	}
//	sphere->setCollisionShape(sphereShape);
//
//	btTransform transform;
//	transform.setIdentity();
//	transform.setOrigin(
//	    btVector3(collider.pos.x, collider.pos.y, collider.pos.z)
//	);
//	btScalar x, y, z;
//	x = glm::radians(collider.rot.x);
//	y = glm::radians(collider.rot.y);
//	z = glm::radians(collider.rot.z);
//	btQuaternion rotation(y, x, z);
//	transform.setRotation(rotation);
//
//	sphere->setWorldTransform(transform);
//	this->dynWorld->addCollisionObject(sphere);
//
//	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	sphereVec.emplace_back(&collider);
//}
//
//void PhysicsEngine::createBoxCol(BoxCollider& collider)
//{
//	btCollisionShape* boxShape = new btBoxShape(btVector3(
//	    collider.halfExtents.x, collider.halfExtents.y, collider.halfExtents.z
//	));
//	this->colShapes.push_back(boxShape);
//
//	btCollisionObject* box = new btCollisionObject;
//	if (collider.isTrigger)
//	{
//		box->setCollisionFlags(4);
//	}
//	box->setCollisionShape(boxShape);
//
//	btTransform transform;
//	transform.setIdentity();
//	transform.setOrigin(
//	    btVector3(collider.pos.x, collider.pos.y, collider.pos.z)
//	);
//	btScalar x, y, z;
//	x = glm::radians(collider.rot.x);
//	y = glm::radians(collider.rot.y);
//	z = glm::radians(collider.rot.z);
//	btQuaternion rotation(y, x, z);
//	transform.setRotation(rotation);
//
//	box->setWorldTransform(transform);
//	dynWorld->addCollisionObject(box);
//
//	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	boxVec.emplace_back(&collider);
//}
//
//void PhysicsEngine::createCapsuleCol(CapsuleCollider& collider)
//{
//	btCollisionShape* capsuleShape =
//	    new btCapsuleShape(collider.radius, collider.radius);
//	this->colShapes.push_back(capsuleShape);
//
//	btCollisionObject* capsule = new btCollisionObject;
//	if (collider.isTrigger)
//	{
//		capsule->setCollisionFlags(4);
//	}
//	capsule->setCollisionShape(capsuleShape);
//
//	btTransform transform;
//	transform.setIdentity();
//	transform.setOrigin(
//	    btVector3(collider.pos.x, collider.pos.y, collider.pos.z)
//	);
//	btScalar x, y, z;
//	x = glm::radians(collider.rot.y);
//	y = glm::radians(collider.rot.x);
//	z = glm::radians(collider.rot.z);
//	btQuaternion rotation(x, y, z);
//	transform.setRotation(rotation);
//	capsule->setWorldTransform(transform);
//
//	this->dynWorld->addCollisionObject(capsule);
//	collider.ID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	this->capsuleVec.emplace_back(&collider);
//}
//
//bool PhysicsEngine::removeSphereCollider(int ID)
//{
//	int tempID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	delete this->dynWorld->getCollisionObjectArray()[ID];
//
//	for (size_t i = 0; i < sphereVec.size(); i++)
//	{
//		if (this->sphereVec.back()->ID == tempID)
//		{
//			this->sphereVec.pop_back();
//		}
//		else if (this->sphereVec[i]->ID == tempID)
//		{
//			std::swap(this->sphereVec[ID], this->sphereVec[i]);
//			this->sphereVec.pop_back();
//			return true;
//		}
//	}
//	return false;
//}
//
//bool PhysicsEngine::removeBoxCollider(int ID)
//{
//	int tempID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	delete this->dynWorld->getCollisionObjectArray()[ID];
//
//	for (size_t i = 0; i < boxVec.size(); i++)
//	{
//		if (this->boxVec.back()->ID == tempID)
//		{
//			this->boxVec.pop_back();
//		}
//		else if (this->boxVec[i]->ID == tempID)
//		{
//			std::swap(this->boxVec[ID], this->boxVec[i]);
//			this->boxVec.pop_back();
//			return true;
//		}
//	}
//	return false;
//}
//
//bool PhysicsEngine::removeCapsuleCollider(int ID)
//{
//	int tempID = this->dynWorld->getCollisionObjectArray().size() - 1;
//	delete this->dynWorld->getCollisionObjectArray()[ID];
//
//	for (size_t i = 0; i < capsuleVec.size(); i++)
//	{
//		if (this->capsuleVec.back()->ID == tempID)
//		{
//			this->capsuleVec.pop_back();
//		}
//		else if (this->capsuleVec[i]->ID == tempID)
//		{
//			std::swap(this->capsuleVec[ID], this->capsuleVec[i]);
//			this->capsuleVec.pop_back();
//			return true;
//		}
//	}
//	return false;
//}

void PhysicsEngine::updateColliders()
{
	Scene* scene = this->sceneHandler->getScene();
	auto view = scene->getSceneReg().view<Collider>(entt::exclude<Rigidbody>);
	auto func = [&](const auto& entity, Collider& col)
	{
		// Not assigned collider, create in bullet
		if (col.ColID < 0)
		{
			this->createCollider((int)entity, col);
			Log::write("Created collider!");
		}
		// Change values base on component
		else
		{
			btCollisionObject* object = this->dynWorld->getCollisionObjectArray()[col.ColID];
			object->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger);
		}
	};
	view.each(func);
}

void PhysicsEngine::updateRigidbodies()
{
	Scene* scene = this->sceneHandler->getScene();
	auto view = scene->getSceneReg().view<Rigidbody, Collider>();
	auto func = [&](const auto& entity, Rigidbody& rb, Collider& col)
	{
		// Not assigned rigidbody, create in physics engine
		if (rb.ID < 0)
		{
			this->createRigidbody((int)entity, rb, col);
			Log::write("Created rigidbody!");
		}
		// Change values based on component
		else
		{
			btRigidBody* body = this->dynWorld->getNonStaticRigidBodies()[rb.ID];
			body->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger);

			if (rb.mass != body->getMass())
			{
				btCollisionShape* shape = this->colShapes[col.ShapeID];
				btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
				if (rb.mass != 0.0f) { shape->calculateLocalInertia(rb.mass, localInertia); }
				body->setMassProps(rb.mass, localInertia);
			}

			body->setGravity(this->dynWorld->getGravity() * rb.gravityMult);
			body->setFriction(rb.friction);
			body->setLinearFactor(BulletH::bulletVec(rb.posFactor));
			body->setAngularFactor(BulletH::bulletVec(rb.rotFactor));
			body->applyCentralForce(BulletH::bulletVec(rb.acceleration));
			body->setLinearVelocity(body->getLinearVelocity() + BulletH::bulletVec(rb.velocity));

			rb.acceleration = glm::vec3(0.0f);
			rb.velocity = glm::vec3(0.0f);
		}
	};
	view.each(func);
}

btCollisionShape* PhysicsEngine::createShape(const int& entity, Collider& col)
{
	btCollisionShape* shape = nullptr;
	switch (col.type)
	{
	case ColType::SPHERE:
		shape = new btSphereShape(col.radius);
		break;
	case ColType::BOX:
		shape = new btBoxShape(BulletH::bulletVec(col.extents));
		break;
	case ColType::CAPSULE:
		shape = new btCapsuleShape(col.radius, col.height);
		break;
	default:
		break;
	}
	if (!shape) { return shape; }

	shape->setUserIndex(entity);
	this->colShapes.push_back(shape);
	col.ShapeID = (int)this->colShapes.size() - 1;
	return shape;
}

void PhysicsEngine::createCollider(const int& entity, Collider& col)
{
	// Create shape
	btCollisionShape* shape = this->createShape(entity, col);

	// Create object
	btCollisionObject* object = new btCollisionObject();
	object->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger);
	object->setCollisionShape(shape);
	object->setWorldTransform(BulletH::toBulletTransform(this->sceneHandler->getScene()->getComponent<Transform>(entity)));
	object->setUserIndex(entity);

	this->dynWorld->addCollisionObject(object);
	col.ColID = this->dynWorld->getCollisionObjectArray().size() - 1;
}

void PhysicsEngine::createRigidbody(const int& entity, Rigidbody& rb, Collider& col)
{
	// Remove old collider if it exists
	if (col.ColID >= 0)
	{
		//this->removeCollider(entity, col);
	}

	// Create shape
	btCollisionShape* shape = this->createShape(entity, col);

	// Create Rigidbody
	btTransform transform = BulletH::toBulletTransform(this->sceneHandler->getScene()->getComponent<Transform>(entity));
	btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
	if(rb.mass != 0.0f) { shape->calculateLocalInertia(rb.mass, localInertia); }

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(rb.mass, motionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	body->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger);

	body->setGravity(this->dynWorld->getGravity() * rb.gravityMult);
	body->setFriction(rb.friction);
	body->setLinearFactor(BulletH::bulletVec(rb.posFactor));
	body->setAngularFactor(BulletH::bulletVec(rb.rotFactor));
	body->applyCentralForce(BulletH::bulletVec(rb.acceleration));
	body->setLinearVelocity(BulletH::bulletVec(rb.velocity));
	body->setUserIndex(entity);

	this->dynWorld->addRigidBody(body);
	rb.ID = this->dynWorld->getNonStaticRigidBodies().size() - 1;
	col.ColID = this->dynWorld->getCollisionObjectArray().size() - 1;
}

void PhysicsEngine::removeCollider(btCollisionObject* obj)
{
	// Remove from dynWorld array

	// Change id of previous last element

	


	//int tempID = this->dynWorld->getCollisionObjectArray().size() - 1;
	//	delete this->dynWorld->getCollisionObjectArray()[ID];
	//
	//	for (size_t i = 0; i < sphereVec.size(); i++)
	//	{
	//		if (this->sphereVec.back()->ID == tempID)
	//		{
	//			this->sphereVec.pop_back();
	//		}
	//		else if (this->sphereVec[i]->ID == tempID)
	//		{
	//			std::swap(this->sphereVec[ID], this->sphereVec[i]);
	//			this->sphereVec.pop_back();
	//			return true;
	//		}
	//	}
	//	return false;
}

void PhysicsEngine::removeRigidbody(const int& entity, Rigidbody& rb)
{
}

void PhysicsEngine::cleanup()
{
	// Remove the rigidbodies from the dynamics world and delete them
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

	// Delete collision shapes
	for (int j = 0; j < this->colShapes.size(); j++)
	{
		btCollisionShape* shape = this->colShapes[j];
		this->colShapes[j] = 0;
		delete shape;
	}

	// Delete dynamics world
	delete this->dynWorld;

	// Delete solver
	delete this->solver;

	// Delete broadphase
	delete this->bpInterface;

	// Delete dispatcher
	delete this->collDisp;

	// Delete configuration
	delete this->collconfig;

	// Next line is optional: it will be cleared by the destructor when the array goes out of scope
	this->colShapes.clear();
}

PhysicsEngine::PhysicsEngine()
	:sceneHandler(nullptr), colShapes(), timer(0.f)
{
	this->collconfig = new btDefaultCollisionConfiguration();
	this->collDisp = new btCollisionDispatcher(this->collconfig);
	this->bpInterface = new btDbvtBroadphase();
	this->solver = new btSequentialImpulseConstraintSolver();
	this->dynWorld = new btDiscreteDynamicsWorld(this->collDisp, this->bpInterface, this->solver, this->collconfig);
	this->dynWorld->setGravity(btVector3(0, -10, 0));
}

PhysicsEngine::~PhysicsEngine()
{
	cleanup();
}

void PhysicsEngine::setSceneHandler(SceneHandler* sceneHandler)
{
	this->sceneHandler = sceneHandler;
}

void PhysicsEngine::init()
{
	cleanup();
}

void PhysicsEngine::update()
{
	/*updateSphere();
	updateBox();
	updateCapsule();
	updateRigidBody();*/
	
	Scene* scene = this->sceneHandler->getScene();

	// Update values from components
	updateColliders();
	updateRigidbodies();

	// Update world
	this->dynWorld->stepSimulation(Time::getDT(), 1, this->TIMESTEP);
	this->dynWorld->updateAabbs();
	this->dynWorld->computeOverlappingPairs();

	for (int i = this->dynWorld->getNumCollisionObjects() - 1; i > -1; i--)
	{
		btCollisionObject* object = this->dynWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(object);

		// Rigidbody
		if (body)
		{
			btTransform transform;
			body->getMotionState()->getWorldTransform(transform);

			Transform& t = scene->getComponent<Transform>(object->getUserIndex());
			glm::vec3 scale = t.scale;

			t = BulletH::toTransform(transform);
			t.scale = scale;
		}
		// Colliders
		/*else
		{
			transform = object->getWorldTransform();
		}*/

		int numManifolds = this->dynWorld->getDispatcher()->getNumManifolds();
		for (int i = 0; i < numManifolds; i++)
		{
			btPersistentManifold* man = this->dynWorld->getDispatcher()->getManifoldByIndexInternal(i);
			Log::write("Entity: " + std::to_string(man->getBody0()->getUserIndex()) + " and Entity: " + std::to_string(man->getBody1()->getUserIndex()) + " hit!");
		}
	}

	//this->timer += Time::getDT();
	//while (this->timer >= this->TIMESTEP)
	//{
	//	this->dynWorld->stepSimulation(this->TIMESTEP, 1);
	//	this->dynWorld->updateAabbs();
	//	this->dynWorld->computeOverlappingPairs();

	//	for (int j = this->dynWorld->getNumCollisionObjects() - 1; j > -1; j--)
	//	{
	//		btCollisionObject* obj = this->dynWorld->getCollisionObjectArray()[j];
	//		btRigidBody* body = btRigidBody::upcast(obj);
	//		btTransform trans;

	//		if (capsuleVec.size() != 0 && rigidBodyVec.size() != 0)
	//		{
	//			// std::cout << dynWorld->getNonStaticRigidBodies()[0]->getCollisionShape()->getUserIndex() << std::endl;
	//		}

	//		if (body && body->getMotionState())
	//		{
	//			body->getMotionState()->getWorldTransform(trans);
	//		}
	//		else
	//		{
	//			trans = obj->getWorldTransform();
	//		}
	//		printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));

	//		static float up = 0.f;
	//		static float dir = 1.f;

	//		//  ###### CHECK IF NON COLLIDABLE OBJECT COLLIDED
	//		// int numManifolds = dynWorld->getDispatcher()->getNumManifolds();
	//		// for (size_t i = 0; i < numManifolds; i++)
	//		// {
	//		// 	if (dynWorld->getDispatcher()->getManifoldByIndexInternal(i))
	//		// 	{
	//		// 		std::cout << "DUDSADASD%%%%%%%%%%%%%%%%%%%" << std::endl;
	//		// 	}
	//		// }

	//		// // /all hits
	//		// {
	//		// 	btVector3 from(-30, 1 + up, 0);
	//		// 	btVector3 to(30, 1, 0);
	//		// 	btCollisionWorld::AllHitsRayResultCallback allResults(from, to);
	//		// 	allResults.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
	//		// 	// kF_UseGjkConvexRaytest flag is now enabled by default, use the faster but more approximate algorithm
	//		// 	// allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;
	//		// 	allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

	//		// 	dynWorld->rayTest(from, to, allResults);

	//		// 	for (int i = 0; i < allResults.m_hitFractions.size(); i++)
	//		// 	{
	//		// 		btVector3 p = from.lerp(to, allResults.m_hitFractions[i]);
	//		// 	}
	//		// }

	//	}
	//	this->timer -= this->TIMESTEP;
	//}
}

RayPayload PhysicsEngine::shootRay(Ray ray, float maxDist)
{
	glm::vec3 toPos = ray.pos + glm::normalize(ray.dir) * maxDist;
	btCollisionWorld::ClosestRayResultCallback closestResults(BulletH::bulletVec(ray.pos), BulletH::bulletVec(toPos));
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

	this->dynWorld->rayTest(closestResults.m_rayFromWorld, closestResults.m_rayToWorld, closestResults);

	RayPayload payload {};
	if (closestResults.hasHit())
	{
		payload.hit = true;
		payload.entity = closestResults.m_collisionObject->getUserIndex();
		payload.hitPoint = BulletH::glmvec(closestResults.m_hitPointWorld);
		payload.hitNormal = BulletH::glmvec(closestResults.m_hitNormalWorld);
	}
	return payload;
}

//void PhysicsEngine::applyForce(glm::vec3 force)
//{
//	btVector3 btForce = { force.x, force.y, force.z };
//	this->dynWorld->getNonStaticRigidBodies().at(0)->applyForce(btForce, btVector3(0, 0, 0));
//}