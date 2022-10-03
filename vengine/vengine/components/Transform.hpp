#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4 matrix;

	Transform():
		position(0.0f), rotation(0.0f), scale(1.0f), matrix(1.0f)
	{ }

	glm::vec3 right() { return glm::normalize(glm::vec3(this->matrix[0])); }
	glm::vec3 up() { return glm::normalize(glm::vec3(this->matrix[1])); }
	glm::vec3 forward() { return glm::normalize(glm::vec3(this->matrix[2])); }

	void updateMatrix() // Only useful before right, up, forward or rendering the object
	{
		this->matrix = glm::translate(glm::mat4(1.0f), this->position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(this->rotation.z), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(this->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(this->rotation.x), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), this->scale);
	}
};