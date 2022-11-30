#pragma once

#include "glm/glm.hpp"

enum class SpawnShape
{
	POINT,
	DISK,
	AABB
};

struct Cone
{
	glm::vec3 localPosition = glm::vec3(0.0f);
	glm::vec3 localDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	float diskRadius = 0.0f;
	float coneAngle = 90.0f;
};

struct ParticleSystem
{
	uint32_t numParticles = 10;
	uint32_t textureIndex = 0;
	float maxlifeTime = 1.0f;
	glm::vec2 startSize = glm::vec2(1.0f);
	glm::vec2 endSize = glm::vec2(1.0f);
	glm::vec3 startColor = glm::vec3(1.0f);
	glm::vec3 endColor = glm::vec3(1.0f);
	glm::vec3 startVelocity = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);

	// Spawn volumes
	Cone coneSpawnVolume;
};