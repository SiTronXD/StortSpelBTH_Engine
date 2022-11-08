#pragma once
#include "../VengineHelper.h"

/*
	"Can't open include file" OpenAL/al.h
	"Can't open include file" OpenAL/al.h
	"Can't open include file" OpenAL/al.h
*/

struct AudioSource
{
	// Multiple sounds?

	AudioSourceId sourceId;
	ALBufferId bufferId;

	AudioSource();
	AudioSource(ALBufferId bufferId);
	~AudioSource();

	void setBuffer(ALBufferId bufferId);

	void setLooping(bool loop);
	bool getLooping() const;

	void setVolume(float volume);
	float getVolume() const;

	void play();
	void stop();
	void pause();
};