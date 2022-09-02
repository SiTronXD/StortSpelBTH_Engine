#include "Camera.h"
#include "Renderer.h"
#include "../Application/Input.h"
#include "../Application/Time.h"

void Camera::updateDirVectors()
{
	// Forward direction
	this->forwardDir = glm::vec3(
		(float) (sin(this->yaw) * cos(this->pitch)),
		(float) sin(this->pitch),
		(float) (cos(this->yaw) * cos(this->pitch))
	);

	this->forwardDir = glm::normalize(this->forwardDir);

	// Right direction
	this->rightDir = glm::cross(this->forwardDir, glm::vec3(0.0f, 1.0f, 0.0f));
	this->rightDir = glm::normalize(this->rightDir);

	// Up direction
	this->upDir = glm::cross(this->rightDir, this->forwardDir);
	this->upDir = glm::normalize(this->upDir);
}

void Camera::updateMatrices()
{
	this->viewMatrix = glm::lookAt(
		this->position,
		this->position + this->forwardDir,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	this->projectionMatrix = glm::perspective(
		glm::radians(90.0f),
		this->renderer.getSwapchainAspectRatio(),
		0.1f,
		100.0f
	);
}

void Camera::recalculate()
{
	this->updateDirVectors();
	this->updateMatrices();
}

Camera::Camera(Renderer& renderer)
	: renderer(renderer),
	viewMatrix(glm::mat4(1.0f)),
	projectionMatrix(glm::mat4(1.0f)),

	position(0.0f, 1.0f, 2.0f),
	forwardDir(-1.0f, -1.0f, -1.0f),
	
	yaw(SMath::PI),
	pitch(-0.5f)
{
	this->recalculate();
}

Camera::~Camera()
{
}

void Camera::update()
{
	// Keyboard input
	float rightSpeed =
		(float) (Input::isKeyDown(Keys::D) - Input::isKeyDown(Keys::A));
	float forwardSpeed =
		(float) (Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
	float upSpeed =
		(float) (Input::isKeyDown(Keys::E) - Input::isKeyDown(Keys::Q));

	// Move position
	this->position += 
		(rightSpeed * this->rightDir +
		forwardSpeed * this->forwardDir +
		upSpeed * this->upDir) * this->MOVEMENT_SPEED * Time::getDT();

	// Mouse input
	if (Input::isMouseButtonDown(Mouse::RIGHT_BUTTON))
	{
		this->yaw += Input::getMouseDeltaX() * 0.01f;
		this->pitch += Input::getMouseDeltaY() * 0.01f;
	}

	this->recalculate();
}
