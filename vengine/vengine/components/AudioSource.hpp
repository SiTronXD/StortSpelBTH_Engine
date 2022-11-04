#pragma once
#include "glm/glm.hpp"

struct AudioSource
{
	// Multiple sounds?

	const AudioSourceId sourceId;
	AudioBufferId bufferId;

	AudioSource(AudioSourceId sourceId)
		:sourceId(sourceId)
	{
	}
	AudioSource(AudioSourceId sourceId, AudioBufferId bufferId)
		:sourceId(sourceId), bufferId(bufferId)
	{
	}

	static AudioSource create(AudioBufferId bufferId)
	{
		AudioSourceId sourceId;
		alGenSources(1, &sourceId);

		return AudioSource(sourceId, bufferId);
	}
};