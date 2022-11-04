#pragma once
#include "OpenAL/al.h"
#include "glm/glm.hpp"

struct AudioListener
{
	// Ranges from 0.0f - 1.0f
	float volume;

	AudioListener()
	{
		alGetListenerf(AL_GAIN, &this->volume);
	}

	void setVolume(float volume)
	{
		this->volume = volume;
		alListenerf(AL_GAIN, volume);
	}

	float getVolume() const
	{
		return this->volume;
	}
};