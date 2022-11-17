#pragma once

#include "glm/glm.hpp"

struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 color;

    // Shadow map
    float shadowMapFrustumHalfWidth = 50.0f;
    float shadowMapFrustumHalfHeight = 50.0f;
    float shadowMapFrustumDepth = 400.0f;
    float shadowMapMinBias = 0.0001f;
    float shadowMapAngleBias = 0.0015f;
};