#pragma once

#include "../graphics/ShaderInput.hpp"

struct AnimationComponent
{
	float timer;
	float timeScale;
	StorageBufferID boneTransformsID;
	uint32_t animationIndex;

	AnimationComponent() :
		timer(0.0f), timeScale(1.0f), boneTransformsID(~0u), animationIndex(0)
	{ }

};