#pragma once

#include "glm/glm.hpp"

struct CapsuleCollider
{
	int ID = -1;

	glm::vec3 pos;
	glm::vec3 rot = { 0.f, 0.f, 0.f };
	float height;
	float radius;
	float weight = 1.f;
	bool isTrigger = false;
	bool hasRigidBody = false;
};