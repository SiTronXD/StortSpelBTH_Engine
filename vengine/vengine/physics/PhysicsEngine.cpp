#include "pch.h"
#include "PhysicsEngine.h"
#include "../application/SceneHandler.hpp"
#include "../graphics/DebugRenderer.hpp"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletHelper.hpp"
#include "ContactCallback.h"
#include "CollisionDispatcher.h"

#include <iostream>
#include <string>

void PhysicsEngine::updateColliders()
{
	Scene* scene = this->sceneHandler->getScene();
	auto view = scene->getSceneReg().view<Collider>(entt::exclude<Rigidbody, Inactive>);
	auto func = [&](const auto& entity, Collider& col)
	{
		// Not assigned collider, create in bullet
		if (col.ColID < 0)
		{
			this->createCollider((int)entity, col);
		}
		// Change values base on component
		else
		{
			btCollisionObject* object = this->dynWorld->getCollisionObjectArray()[col.ColID];
			btRigidBody* body = btRigidBody::upcast(object);
			if (body)
			{
				body->setCollisionFlags(
					btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger +
					btCollisionObject::CollisionFlags::CF_KINEMATIC_OBJECT * !col.isTrigger);

				btTransform transform = BulletH::toBulletTransform(this->sceneHandler->getScene()->getComponent<Transform>((int)entity));
				transform.setOrigin(transform.getOrigin() + BulletH::bulletVec(col.offset));
				body->setWorldTransform(transform);
			}
		}
	};
	view.each(func);
}

void PhysicsEngine::updateRigidbodies()
{
	Scene* scene = this->sceneHandler->getScene();
	auto view = scene->getSceneReg().view<Rigidbody, Collider>(entt::exclude<Inactive>);
	auto func = [&](const auto& entity, Rigidbody& rb, Collider& col)
	{
		// Not assigned rigidbody, create in physics engine
		if (!rb.assigned)
		{
			this->createRigidbody((int)entity, rb, col);
		}
		// Change values based on component
		else
		{
			btCollisionObject* object = this->dynWorld->getCollisionObjectArray()[col.ColID];
			btRigidBody* body = btRigidBody::upcast(object);
			if (body && body->getMotionState())
			{
				body->setCollisionFlags(
					btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger +
					btCollisionObject::CollisionFlags::CF_DYNAMIC_OBJECT * !col.isTrigger);

				btTransform transform = BulletH::toBulletTransform(this->sceneHandler->getScene()->getComponent<Transform>((int)entity));
				transform.setOrigin(transform.getOrigin() + BulletH::bulletVec(col.offset));
				body->getMotionState()->setWorldTransform(transform);

				if (rb.mass != body->getMass())
				{
					btCollisionShape* shape = this->colShapes[col.ShapeID].shape;
					btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
					if (rb.mass != 0.0f) { shape->calculateLocalInertia(rb.mass, localInertia); }
					body->setMassProps(rb.mass, localInertia);
				}

				body->setGravity(this->dynWorld->getGravity() * rb.gravityMult);
				body->setFriction(rb.friction);
				body->setLinearFactor(BulletH::bulletVec(rb.posFactor));
				body->setAngularFactor(BulletH::bulletVec(rb.rotFactor));
				body->setLinearVelocity(BulletH::bulletVec(rb.velocity));
				body->applyCentralForce(BulletH::bulletVec(rb.acceleration));

				rb.acceleration = glm::vec3(0.0f);
			}
		}
	};
	view.each(func);
}

btCollisionShape* PhysicsEngine::createShape(const int& entity, Collider& col)
{
	// Loop: Find suitable collision shape
	// Else: Create new shape
	ColShapeInfo shapeInfo;
	switch (col.type)
	{
	case ColType::SPHERE:
		for (int i = this->colShapes.size() - 1; i > -1; i--)
		{
			if (this->colShapes[i].type == col.type &&
				this->colShapes[i].radius == col.radius)
			{
				col.ShapeID = i;
				return this->colShapes[i].shape;
			}
		}
		shapeInfo.shape = new btSphereShape(col.radius);
		shapeInfo.radius = col.radius;
		break;
	case ColType::BOX:
		for (int i = this->colShapes.size() - 1; i > -1; i--)
		{
			if (this->colShapes[i].type == col.type &&
				this->colShapes[i].extents == col.extents)
			{
				col.ShapeID = i;
				return this->colShapes[i].shape;
			}
		}
		shapeInfo.shape = new btBoxShape(BulletH::bulletVec(col.extents));
		shapeInfo.extents = col.extents;
		break;
	case ColType::CAPSULE:
		for (int i = this->colShapes.size() - 1; i > -1; i--)
		{
			if (this->colShapes[i].type == col.type &&
				this->colShapes[i].radius == col.radius &&
				this->colShapes[i].height == col.height)
			{
				col.ShapeID = i;
				return this->colShapes[i].shape;
			}
		}
		shapeInfo.shape = new btCapsuleShape(col.radius, col.height);
		shapeInfo.radius = col.radius;
		shapeInfo.height = col.height;
		break;
	default:
		return nullptr;
	}

	shapeInfo.type = col.type;
	this->colShapes.push_back(shapeInfo);
	col.ShapeID = this->colShapes.size() - 1;
	return shapeInfo.shape;
}

void PhysicsEngine::createCollider(const int& entity, Collider& col)
{
	// Create shape
	btCollisionShape* shape = this->createShape(entity, col);

	btTransform transform = BulletH::toBulletTransform(this->sceneHandler->getScene()->getComponent<Transform>(entity));
	transform.setOrigin(transform.getOrigin() + BulletH::bulletVec(col.offset));
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, nullptr, shape, btVector3(0.0f, 0.0f, 0.0f));

	btRigidBody* body = new btRigidBody(rbInfo);
	body->setCollisionFlags(
		btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger +
		btCollisionObject::CollisionFlags::CF_KINEMATIC_OBJECT * !col.isTrigger);
	body->setCollisionShape(shape);
	body->setWorldTransform(transform);
	body->setUserIndex(entity);

	this->dynWorld->addRigidBody(body);
	col.ColID = this->dynWorld->getCollisionObjectArray().size() - 1;
}

void PhysicsEngine::createRigidbody(const int& entity, Rigidbody& rb, Collider& col)
{
	// Remove old collider if it exists
	if (col.ColID >= 0)
	{
		this->removeObject(this->dynWorld->getCollisionObjectArray()[col.ColID], col.ColID);
	}

	// Create shape
	btCollisionShape* shape = this->createShape(entity, col);

	// Create Rigidbody
	btTransform transform = BulletH::toBulletTransform(this->sceneHandler->getScene()->getComponent<Transform>(entity));
	transform.setOrigin(transform.getOrigin() + BulletH::bulletVec(col.offset));
	btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
	if (rb.mass != 0.0f) { shape->calculateLocalInertia(rb.mass, localInertia); }

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(rb.mass, motionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	body->setCollisionFlags(
		btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE * col.isTrigger +
		btCollisionObject::CollisionFlags::CF_DYNAMIC_OBJECT * !col.isTrigger);
	body->setActivationState(DISABLE_DEACTIVATION);

	body->setGravity(this->dynWorld->getGravity() * rb.gravityMult);
	body->setFriction(rb.friction);
	body->setLinearFactor(BulletH::bulletVec(rb.posFactor));
	body->setAngularFactor(BulletH::bulletVec(rb.rotFactor));
	body->setLinearVelocity(BulletH::bulletVec(rb.velocity));
	body->applyCentralForce(BulletH::bulletVec(rb.acceleration));
	body->setUserIndex(entity);

	this->dynWorld->addRigidBody(body);
	rb.assigned = true;
	col.ColID = this->dynWorld->getCollisionObjectArray().size() - 1;
}

void PhysicsEngine::removeObject(btCollisionObject* obj, int index)
{
	Scene* scene = this->sceneHandler->getScene();
	btCollisionObject* lastObj = this->dynWorld->getCollisionObjectArray()[this->dynWorld->getNumCollisionObjects() - 1];
	this->dynWorld->removeCollisionObject(obj);

	// Not last object, ID needs to be changed
	if (lastObj != obj && scene->hasComponents<Collider>(lastObj->getUserIndex()))
	{
		Collider& col = scene->getComponent<Collider>(lastObj->getUserIndex());
		col.ColID = index;
	}

	btRigidBody* body = btRigidBody::upcast(obj);
	if (body && body->getMotionState())
	{
		delete body->getMotionState();
	}
	delete obj;
}

void PhysicsEngine::cleanup()
{
	this->renderDebug = false;

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
		btCollisionShape* shape = this->colShapes[j].shape;
		this->colShapes[j].shape = 0;
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
	:sceneHandler(nullptr), colShapes(), timer(0.f), renderDebug(false)
{
	this->collconfig = new btDefaultCollisionConfiguration();
	this->collDispCallbacks = new CollisionDispatcher(this->collconfig);
	this->collDisp = this->collDispCallbacks;
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
	this->collconfig = new btDefaultCollisionConfiguration();
	this->collDispCallbacks = new CollisionDispatcher(this->collconfig);
	this->collDisp = this->collDispCallbacks;
	this->bpInterface = new btDbvtBroadphase();
	this->solver = new btSequentialImpulseConstraintSolver();
	this->dynWorld = new btDiscreteDynamicsWorld(this->collDisp, this->bpInterface, this->solver, this->collconfig);
	this->dynWorld->setGravity(btVector3(0, -10, 0));
}

void PhysicsEngine::update(float dt)
{
	Scene* scene = this->sceneHandler->getScene();

	// Update values from components
	updateColliders();
	updateRigidbodies();

	// Update world
	this->dynWorld->stepSimulation(dt, 1, this->TIMESTEP);
	this->dynWorld->updateAabbs();
	this->dynWorld->computeOverlappingPairs();

	for (int i = this->dynWorld->getNumCollisionObjects() - 1; i > -1; i--)
	{
		btCollisionObject* object = this->dynWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(object);

		if (body)
		{
			Entity entity = body->getUserIndex();
			Collider* col = nullptr;
			Rigidbody* rb = nullptr;

			// Entity removed
			if (!scene->entityValid(entity))
			{
				this->removeIndicies.push_back(i);
				continue;
			}
			else
			{
				if (scene->hasComponents<Collider>(entity)) { col = &scene->getComponent<Collider>(entity); }
				if (scene->hasComponents<Rigidbody>(entity)) { rb = &scene->getComponent<Rigidbody>(entity); }
			}

			// Entity inactive
			if (!scene->isActive(entity))
			{
				if (col) { col->ColID = -1; }
				if (rb) { rb->assigned = false; }
				this->removeIndicies.push_back(i);
			}
			// No instance of collider and rigidbody
			else if (!col && !rb)
			{
				this->removeIndicies.push_back(i);
			}
			// Has collider, but removed rigidbody
			else if (body->getMass() != 0.0f && !rb)
			{
				col->ColID = -1;
				this->removeIndicies.push_back(i);
			}
			// Has rigidbody, but removed collider
			else if (body->getLocalInertia() != btVector3(0.0f, 0.0f, 0.0f) && !col)
			{
				rb->assigned = false;
				this->removeIndicies.push_back(i);
			}
			// Update positions
			else if (body)
			{
				btTransform transform;
				if (body->getMotionState())
				{
					body->getMotionState()->getWorldTransform(transform);
					rb->velocity = BulletH::glmvec(body->getLinearVelocity());
				}
				else
				{
					transform = body->getWorldTransform();
				}

				Transform& t = scene->getComponent<Transform>(entity);
				const glm::vec3 scale = t.scale;
				const glm::vec3 rotation = t.rotation;

				// Apply transform, but keep scale
				t = BulletH::toTransform(transform);
				t.position -= col->offset;
				t.scale = scale;

				// Keep rotation if necessary
				if (rb != nullptr)
				{
					if (glm::dot(rb->rotFactor, rb->rotFactor) <= 0.01f)
					{
						t.rotation = rotation;
					}
				}
				else // Colliders also keep rotations
				{
					t.rotation = rotation;
				}
			}
		}
	}

	int numManifolds = this->dynWorld->getDispatcher()->getNumManifolds();
	ScriptHandler* scriptHandler = this->sceneHandler->getScriptHandler();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* man = this->dynWorld->getDispatcher()->getManifoldByIndexInternal(i);
		CallbackType& type = this->collDispCallbacks->getTypes()[i];
		if (!man->getNumContacts())
		{
			if (type == CallbackType::STAY) { type = CallbackType::EXIT; }
			else { continue; }
		}

		const btCollisionObject* b1 = man->getBody0();
		const btCollisionObject* b2 = man->getBody1();

		Entity e1 = b1->getUserIndex(), e2 = b2->getUserIndex();
		if (scene->entityValid(e1) && scene->entityValid(e2))
		{
			bool t1 = b1->getCollisionFlags() == btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE,
				t2 = b2->getCollisionFlags() == btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE;
			// Triggers
			if (t1 || t2)
			{
				if (type == CallbackType::STAY)
				{
					scene->onTriggerStay(e1, e2);
				}
				else if (type == CallbackType::ENTER)
				{
					scene->onTriggerEnter(e1, e2);
				}
				else // Exit
				{
					scene->onTriggerExit(e1, e2);
				}
			}
			// Collisions
			else
			{
				if (type == CallbackType::STAY)
				{
					scene->onCollisionStay(e1, e2);
				}
				else if (type == CallbackType::ENTER)
				{
					scene->onCollisionEnter(e1, e2);
				}
				else // Exit
				{
					scene->onCollisionExit(e1, e2);
				}
			}

			// Scripts (still valid entities)
			if (scene->entityValid(e1) && scene->entityValid(e2))
			{
				if (scene->hasComponents<Script>(e1))
				{
					scriptHandler->runCollisionFunction(scene->getComponent<Script>(e1), e1, e2, t1, type);
				}
			}
			if (scene->entityValid(e1) && scene->entityValid(e2))
			{
				if (scene->hasComponents<Script>(e2))
				{
					scriptHandler->runCollisionFunction(scene->getComponent<Script>(e2), e2, e1, t2, type);
				}
			}
		}
		if (type == CallbackType::ENTER) { type = CallbackType::STAY; }
	}

	for (const auto& exit : this->collDispCallbacks->getExits())
	{
		Entity e1 = exit.first, e2 = exit.second;
		if (scene->entityValid(e1) && scene->entityValid(e2))
		{
			if (!scene->hasComponents<Collider>(e1) || !scene->hasComponents<Collider>(e2)) { continue; }
			bool t1 = scene->getComponent<Collider>(e1).isTrigger, t2 = scene->getComponent<Collider>(e2).isTrigger;

			// Triggers
			if (t1 || t2)
			{
				scene->onTriggerExit(e1, e2);
			}
			// Collisions
			else
			{
				scene->onCollisionExit(e1, e2);
			}

			// Scripts (still valid entities)
			if (scene->entityValid(e1) && scene->entityValid(e2))
			{
				if (scene->hasComponents<Script>(e1))
				{
					scriptHandler->runCollisionFunction(scene->getComponent<Script>(e1), e1, e2, t1, CallbackType::EXIT);
				}
			}
			if (scene->entityValid(e1) && scene->entityValid(e2))
			{
				if (scene->hasComponents<Script>(e2))
				{
					scriptHandler->runCollisionFunction(scene->getComponent<Script>(e2), e2, e1, t2, CallbackType::EXIT);
				}
			}
		}
	}
	this->collDispCallbacks->getExits().clear();

	// Remove objects
	for (const auto& index : this->removeIndicies)
	{
		this->removeObject(this->dynWorld->getCollisionObjectArray()[index], index);
	}
	this->removeIndicies.clear();

	// Render debug shapes
#if defined(_CONSOLE) // Debug/Release, but not distribution (debug rendering is disabled)
	if (this->renderDebug)
	{
		DebugRenderer* debugRenderer = this->sceneHandler->getDebugRenderer();
		glm::vec3 color = glm::vec3(0.0f, 0.5f, 0.5f);
		for (int i = this->dynWorld->getNumCollisionObjects() - 1; i > -1; i--)
		{
			btCollisionObject* object = this->dynWorld->getCollisionObjectArray()[i];
			Transform& transform = scene->getComponent<Transform>(object->getUserIndex());
			Collider& col = scene->getComponent<Collider>(object->getUserIndex());

			if (col.type == ColType::SPHERE)
			{
				debugRenderer->renderSphere(transform.position + col.offset, col.radius, color);
			}
			else if (col.type == ColType::BOX)
			{
				debugRenderer->renderBox(transform.position + col.offset, transform.rotation, col.extents * 2.0f, color);
			}
			else if (col.type == ColType::CAPSULE)
			{
				debugRenderer->renderCapsule(transform.position + col.offset, transform.rotation, col.height, col.radius, color);
			}
		}
	}
#endif
}

void PhysicsEngine::renderDebugShapes(bool renderDebug)
{
	this->renderDebug = renderDebug;
}

RayPayload PhysicsEngine::raycast(Ray ray, float maxDist)
{
	glm::vec3 toPos = ray.pos + glm::normalize(ray.dir) * maxDist;
	btCollisionWorld::ClosestRayResultCallback closestResults(BulletH::bulletVec(ray.pos), BulletH::bulletVec(toPos));
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

	this->dynWorld->rayTest(closestResults.m_rayFromWorld, closestResults.m_rayToWorld, closestResults);

	RayPayload payload{};
	if (closestResults.hasHit())
	{
		payload.hit = true;
		payload.entity = closestResults.m_collisionObject->getUserIndex();
		payload.hitPoint = BulletH::glmvec(closestResults.m_hitPointWorld);
		payload.hitNormal = BulletH::glmvec(closestResults.m_hitNormalWorld);
	}
	return payload;
}

// Optimize (reduce dynamic allocations)
std::vector<Entity> PhysicsEngine::testContact(Collider& col, glm::vec3 position, glm::vec3 rotation)
{
	btCollisionShape* shape = nullptr;
	btCollisionObject* object = new btCollisionObject();
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
		return std::vector<int>();
	}
	object->setCollisionShape(shape);

	btTransform transform;
	transform.setOrigin(BulletH::bulletVec(position + col.offset));
	transform.setRotation(BulletH::bulletQuat(rotation));
	object->setWorldTransform(transform);

	ContactCallback closestResults;
	this->dynWorld->contactTest(object, closestResults);
	delete shape;
	delete object;
	return closestResults.entitiesHit;
}
