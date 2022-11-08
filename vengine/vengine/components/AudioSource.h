#pragma once

typedef unsigned int uint32_t;

struct AudioSource
{
	// Multiple sounds?

	uint32_t sourceId;
	uint32_t bufferId;

	AudioSource();
	AudioSource(uint32_t bufferId);
	~AudioSource();

	void setBuffer(uint32_t bufferId);

	void setLooping(bool loop);
	bool getLooping() const;

	void setVolume(float volume);
	float getVolume() const;

	void play();
	void stop();
	void pause();
};