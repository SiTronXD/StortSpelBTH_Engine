#pragma once

#include <glm/glm.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "../components/Transform.hpp"

namespace BulletH
{
	static glm::vec3 glmvec(const btVector3& vec)
	{
	    return glm::vec3{ vec.x(), vec.y(), vec.z() };
	}

	static glm::vec3 glmvec(const btQuaternion& quat)
	{
		btScalar x, y, z;
		quat.getEulerZYX(z, y, x);
		return glm::vec3{ x, y, z };
	}

	static btVector3 bulletVec(const glm::vec3& vec)
	{
	    return btVector3(vec.x, vec.y, vec.z);
	}

	static btQuaternion bulletQuat(const glm::vec3& vec)
	{
		return btQuaternion(vec.y, vec.x, vec.z);
	}
	
	static Transform toTransform(const btTransform& transform) 
	{
		Transform t;
		t.position = glmvec(transform.getOrigin());

		btQuaternion q = transform.getRotation();
		t.rotation = glm::degrees(glmvec(q));

		return t;
	}

	static btTransform toBulletTransform(const Transform& transform)
	{
		btTransform t;
		glm::vec3 rot = glm::radians(transform.rotation);

		t.setIdentity();
		t.setOrigin(bulletVec(transform.position));
		t.setRotation(bulletQuat(rot));

		return t;
	}
}
