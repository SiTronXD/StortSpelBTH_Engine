#pragma once

#include <glm/glm.hpp>

enum class ColType { SPHERE, BOX, CAPSULE, COUNT };

struct Collider
{
	bool isTrigger = false;

	// NOTE: Can't be changed after creation, create new collider component instead
	ColType type;
	// Type specific 
	float radius; // Sphere and Capsule
	float height; // Capsule
	glm::vec3 extents; // Box, half extents

	int ColID = -1;
	int ShapeID = -1;

	// Constructor functions
	static Collider createSphere(float radius, bool isTrigger = false)
	{
		Collider col;
		col.type = ColType::SPHERE;
		col.radius = radius;
		col.isTrigger = isTrigger;
		return col;
	}

	static Collider createBox(glm::vec3 extents, bool isTrigger = false)
	{
		Collider col;
		col.type = ColType::BOX;
		col.extents = extents;
		col.isTrigger = isTrigger;
		return col;
	}

	static Collider createCapsule(float radius, float height, bool isTrigger = false)
	{
		Collider col;
		col.type = ColType::CAPSULE;
		col.radius = radius;
		col.height = height;
		col.isTrigger = isTrigger;
		return col;
	}
};