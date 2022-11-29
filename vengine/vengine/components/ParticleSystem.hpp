#pragma once

#include "glm/glm.hpp"

enum class SpawnShape
{
	POINT,
	DISK,
	AABB
};

struct Point
{
	glm::vec3 position = glm::vec3(0.0f);
};

struct Disk
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f);
	float radius = 1.0f;
};

struct AABB
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 halfExtents = glm::vec3(0.5f);
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
	struct Shapes
	{
		Point point{};
		Disk disk{};
		AABB asbb{};

		SpawnShape currentSpawnShape = SpawnShape::POINT;
	} spawnShape{};
};