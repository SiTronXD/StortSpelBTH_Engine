#pragma once

#include "glm/glm.hpp"

struct RigidBody
{
	int ID = -1;

	glm::vec3 pos;
	glm::vec3 rot = { 0.f, 0.f, 0.f };
	float weight = 1.f;
};