#pragma once

#include "glm/glm.hpp"

struct Rigidbody
{
	float weight = 1.0f;
	glm::vec3 posConstraints = glm::vec3(0.0f);
	glm::vec3 rotConstraints = glm::vec3(0.0f);

	glm::vec3 acceleration = glm::vec3(0.0f);
	glm::vec3 velocity = glm::vec3(0.0f);

	int ID = -1;
};