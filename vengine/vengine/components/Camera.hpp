#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Transform.hpp"

#include "../dev/Log.hpp"

struct Camera
{
private:
	Transform oldTransform;

	bool flipAspectScale = false;

	void getPerspectiveMatrix(
		float fov,
		float aspectRatio,
		float nearPlane,
		float farPlane,
		glm::mat4& output)
	{
		float tanHalfAngle = std::tan(fov * 0.5f);

		const float DEFAULT_ASP = 16.0f / 9.0f;

		// Modified projection matrix
		if (this->flipAspectScale && aspectRatio < DEFAULT_ASP)
		{
			output = glm::mat4(
				1.0f / (DEFAULT_ASP * tanHalfAngle), 0.0f, 0.0f, 0.0f,
				0.0f, (aspectRatio / DEFAULT_ASP) / (tanHalfAngle), 0.0f, 0.0f,
				0.0f, 0.0f, -farPlane / (farPlane - 2.0f * nearPlane), -1.0f,
				0.0f, 0.0f, -2.0f * (farPlane * nearPlane) / (farPlane - nearPlane), 0.0f
			);
		}
		// Original projection matrix
		else
		{
			// Equivalent to 
			// glm::perspective(fov, aspectRatio, nearPlane, farPlane)
			output = glm::mat4(
				1.0f / (aspectRatio * tanHalfAngle), 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f / (tanHalfAngle), 0.0f, 0.0f,
				0.0f, 0.0f, -farPlane / (farPlane - 2.0f * nearPlane), -1.0f,
				0.0f, 0.0f, -2.0f * (farPlane * nearPlane) / (farPlane - nearPlane), 0.0f
			);
		}
	}

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
		: fov(fov), aspectRatio(0.0f), nearPlane(0.1f), farPlane(1000.0f), view(1.0f), projection(1.0f), invProjection(1.0f), viewAndProj(1.0f), flipAspectScale(false)
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

		this->getPerspectiveMatrix(glm::radians(fov), aspectRatio, nearPlane, farPlane, this->projection);
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

	void flipCameraAspectScale()
	{
		this->flipAspectScale = true;
	}
};