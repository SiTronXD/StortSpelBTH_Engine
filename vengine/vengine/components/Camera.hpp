#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Transform.hpp"

struct Camera
{
private:
	Transform oldTransform;
public:
	float fov;
	float nearPlane;
	float farPlane;
    float aspectRatio;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 invProjection;
	glm::mat4 viewAndProj;

	Camera(float fov = 90.0f) 
		: fov(fov), aspectRatio(0.0f), nearPlane(0.1f), farPlane(1000.0f), view(1.0f), projection(1.0f), invProjection(1.0f), viewAndProj(1.0f)
	{
		oldTransform.position = glm::vec3(~0u);
	}

	void calculateProjectionMatrix(
		float aspectRatio, 
		float nearPlane = 0.1f, 
		float farPlane = 1000.0f)
	{
        this->aspectRatio = aspectRatio;
		this->nearPlane = nearPlane;
		this->farPlane = farPlane;

		this->projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		this->invProjection = glm::inverse(this->projection);
		this->viewAndProj = this->projection * this->view;
	}

	void updateMatrices(Transform& transform)
	{
		if (transform.position != oldTransform.position ||
			transform.rotation != oldTransform.rotation)
		{
			this->view = glm::lookAt(
				transform.position,
				transform.position + transform.forward(),
				transform.up()
			);
			this->viewAndProj = this->projection * this->view;
			oldTransform = transform;
		}
	}
};