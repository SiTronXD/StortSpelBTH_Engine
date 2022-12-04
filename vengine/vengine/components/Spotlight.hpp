#pragma once 
 #include "op_overload.hpp"

#include "glm/glm.hpp"

struct Spotlight
{
	glm::vec3 positionOffset = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 color = glm::vec3(0.0f);
	float angle = 90.0f;
};