#pragma once 
 #include "op_overload.hpp"

#include "glm/glm.hpp"

class ParticleSystemHandler;
class VulkanRenderer;

enum class RespawnSetting
{
	CONTINUOUS,
	EXPLOSION
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
private:
	friend ParticleSystemHandler;
	friend VulkanRenderer;

	uint32_t particleSystemIndex = ~0u;
	uint32_t baseInstanceOffset = 0;

public:
	char name[16];
	uint32_t numParticles = 10;
	uint32_t textureIndex = 0;
	float maxlifeTime = 1.0f;
	float velocityStrength = 10.0f;
	glm::vec2 startSize = glm::vec2(1.0f);
	glm::vec2 endSize = glm::vec2(1.0f);
	glm::vec4 startColor = glm::vec4(1.0f);
	glm::vec4 endColor = glm::vec4(1.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);

	// Spawn
	Cone coneSpawnVolume;
	RespawnSetting respawnSetting = RespawnSetting::CONTINUOUS;
	bool spawn = true;
	float spawnRate = 0.0f;
};