#pragma once 
 #include "op_overload.hpp"

#include <glm/glm.hpp>

enum class ColType { SPHERE, BOX, CAPSULE, COUNT, COLERROR };

struct Collider
{
	bool isTrigger = false;
	glm::vec3 offset = glm::vec3(0.0f);

	// NOTE: Can't be changed after creation, create new(__FILE__, __LINE__) collider component instead
	ColType type;
	// Type specific 
	float radius; // Sphere and Capsule
	float height; // Capsule
	glm::vec3 extents; // Box, half extents

	int ColID = -1;
	int ShapeID = -1;

	// Constructor functions
	static Collider createSphere(float radius, glm::vec3 offset = glm::vec3(0.0f), bool isTrigger = false)
	{
		Collider col;
		col.type = ColType::SPHERE;
		col.radius = radius;
		col.offset = offset;
		col.isTrigger = isTrigger;
		return col;
	}

	static Collider createBox(glm::vec3 extents, glm::vec3 offset = glm::vec3(0.0f), bool isTrigger = false)
	{
		Collider col;
		col.type = ColType::BOX;
		col.extents = extents;
		col.offset = offset;
		col.isTrigger = isTrigger;
		return col;
	}

	static Collider createCapsule(float radius, float height, glm::vec3 offset = glm::vec3(0.0f), bool isTrigger = false)
	{
		Collider col;
		col.type = ColType::CAPSULE;
		col.radius = radius;
		col.height = height;
		col.offset = offset;
		col.isTrigger = isTrigger;
		return col;
	}
};