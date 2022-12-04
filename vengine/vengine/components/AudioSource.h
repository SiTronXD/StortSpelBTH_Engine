#pragma once 
 #include "op_overload.hpp"

typedef unsigned int uint32_t;

struct AudioSource
{
	bool playingb4Inactive; // No touchy

	uint32_t sourceId;

	AudioSource(uint32_t bufferId);
	AudioSource();
	~AudioSource();

	void setBuffer(uint32_t bufferId);
	int getBuffer() const;

	void setLooping(bool loop);
	bool getLooping() const;

	void setVolume(float volume);
	float getVolume() const;

	void play();
	void stop();
	void pause();

	bool isPlaying() const;
};