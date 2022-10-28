#pragma once

#include "../graphics/ShaderInput.hpp"

struct AnimationComponent
{
	float timer;
	float timeScale;
	float endTime;
	StorageBufferID boneTransformsID;

	AnimationComponent() :
		timer(0.0f), timeScale(1.0f), endTime(1.0f), boneTransformsID(~0u)
	{ }

};