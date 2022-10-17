#pragma once

#include <glm/glm.hpp>

enum class ColType { SPHERE, BOX, CAPSULE, COUNT };

struct Collider
{
	ColType type;
	int ID = -1;
	bool isTrigger = false;

	// Type specific
	float radius; // Sphere and Capsule
	float height; // Capsule
	glm::vec3 extents; // Box, half extents

	// Constructor functions
	static Collider createSphere(float radius)
	{
		Collider col;
		col.type = ColType::SPHERE;
		col.radius = radius;
		return col;
	}

	static Collider createBox(glm::vec3 extents)
	{
		Collider col;
		col.type = ColType::BOX;
		col.extents = extents;
		return col;
	}

	static Collider createCapsule(float radius, float height)
	{
		Collider col;
		col.type = ColType::CAPSULE;
		col.radius = radius;
		col.height = height;
		return col;
	}
};