#pragma once

#include "../SMath.h"

class Renderer;

class Camera
{
private:
	const float MOVEMENT_SPEED = 2.0f;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::vec3 position;
	glm::vec3 forwardDir;
	glm::vec3 rightDir;
	glm::vec3 upDir;

	float yaw;
	float pitch;

	Renderer& renderer;

	void updateDirVectors();
	void updateMatrices();

	void recalculate();

public:
	Camera(Renderer& renderer);
	~Camera();

	void update();

	inline const glm::mat4& getViewMatrix() const { return this->viewMatrix; }
	inline const glm::mat4& getProjectionMatrix() const { return this->projectionMatrix; }

	inline const glm::vec3& getPosition() const { return this->position; }
};