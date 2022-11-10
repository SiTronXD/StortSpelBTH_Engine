#pragma once

typedef unsigned int uint32_t;

struct AudioSource
{
	bool playingb4Inactive; // No touchy

	uint32_t sourceId;
	uint32_t bufferId;

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

	bool isPlaying() const;
};