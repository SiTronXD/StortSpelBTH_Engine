#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct Camera
{
	float aspectRatio;
	float fov;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 invProjection;

	Camera(float aspectRatio = 1.0f, float fov = 90.0f):
		aspectRatio(aspectRatio), fov(fov), view(1.0f)
	{
		this->calculateProjectionMatrix();
	}

	void calculateProjectionMatrix()
	{
		this->projection = glm::perspective(fov, aspectRatio, 0.01f, 1000.0f);
		this->invProjection = glm::inverse(this->projection);
	}
};