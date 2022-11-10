#pragma once

#include "glm/glm.hpp"

struct PointLight
{
	glm::vec3 positionOffset = glm::vec3(0.0f);
	glm::vec3 color = glm::vec3(0.0f);
};