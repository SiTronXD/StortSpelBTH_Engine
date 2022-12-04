#pragma once 
 #include "op_overload.hpp"

#include "glm/glm.hpp"

struct Rigidbody
{
	float mass = 1.0f;
	float gravityMult = 1.0f;
	float friction = 1.0f;
	glm::vec3 posFactor = glm::vec3(1.0f);
	glm::vec3 rotFactor = glm::vec3(1.0f);

	glm::vec3 acceleration = glm::vec3(0.0f);
	glm::vec3 velocity = glm::vec3(0.0f);

	bool assigned = false;
};