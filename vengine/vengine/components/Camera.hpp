#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct Camera
{
	float fov;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 invProjection;

	Camera(float fov = 90.0f):
		fov(fov), view(1.0f), projection(1.0f), invProjection(1.0f)
	{ }

	void calculateProjectionMatrix(
		float aspectRatio, 
		float nearPlane = 0.1f, 
		float farPlane = 1000.0f)
	{
		this->projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		this->invProjection = glm::inverse(this->projection);
	}
};