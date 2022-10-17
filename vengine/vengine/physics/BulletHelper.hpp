#pragma once

#include <glm/glm.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>

#include "../components/Transform.hpp"

namespace BulletH
{
	static glm::vec3 glmvec(btVector3& vec)
	{
	    return glm::vec3{ vec.x(), vec.y(), vec.z() };
	}

	static glm::vec3 glmvec(btQuaternion& quat)
	{
		btScalar x, y, z;
		quat.getEulerZYX(y, x, z);
		return glm::vec3{ x, y, z };
	}

	static btVector3 bulletVec(glm::vec3& vec)
	{
	    return btVector3(vec.x, vec.y, vec.z);
	}

	static btQuaternion bulletQuat(glm::vec3& vec)
	{
		return btQuaternion(vec.y, vec.x, vec.z);
	}
	
	static Transform toTransform(btTransform& transform) 
	{
		Transform t;
		t.position = glmvec(transform.getOrigin());

		btQuaternion q = transform.getRotation();
		t.rotation = glm::degrees(glmvec(q));

		return t;
	}

	static btTransform toBulletTransform(Transform& transform)
	{
		btTransform t;
		t.setOrigin(bulletVec(transform.position));

		glm::vec3 rot = glm::radians(transform.rotation);
		t.setRotation(bulletQuat(rot));
	}
}
