#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4 matrix;

	Transform():
		position(0.0f), rotation(0.0f), scale(1.0f), matrix(1.0f)
	{ }

	glm::vec3 right() { return glm::vec3(this->matrix[0]); }
	glm::vec3 up() { return glm::vec3(this->matrix[1]); }
	glm::vec3 forward() { return glm::vec3(this->matrix[2]); }
};