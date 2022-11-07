#pragma once
#include "../VengineHelper.h"
#include "OpenAL/al.h"

/*
	"Can't open include file" OpenAL/al.h
	"Can't open include file" OpenAL/al.h
	"Can't open include file" OpenAL/al.h
*/

struct AudioSource
{
	// Multiple sounds?

	AudioSourceId sourceId;
	AudioBufferId bufferId;

	AudioSource(AudioSourceId sourceId)
		:sourceId(sourceId), bufferId(~1u)
	{
	}
	AudioSource(AudioSourceId sourceId, AudioBufferId bufferId)
		:sourceId(sourceId), bufferId(bufferId)
	{
	}
	~AudioSource()
	{
		alDeleteSources(1, &this->sourceId);
	}

	static AudioSource create(AudioBufferId bufferId)
	{
		AudioSourceId sourceId;
		alGenSources(1, &sourceId);

		return AudioSource(sourceId, bufferId);
	}
};