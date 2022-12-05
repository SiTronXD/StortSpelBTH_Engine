#pragma once 
 #include "op_overload.hpp"

typedef unsigned int AudioSourceID;
typedef unsigned int AudioBufferID;

/* 
* AudioSource is attachted to an entity if the function:
* AudioHandler::requestAudioSource() 
* succeeds. 
* 
* An instance of AudioSource is made for more permanent usage.
* For example, a walking sound or looping a sound
*/

struct AudioSource
{
	AudioSourceID sourceId;

	AudioSource(AudioSourceID sourceId = ~0u);
	~AudioSource();

	void play();
	void stop();
	void pause();
	
	void setBuffer(AudioBufferID bufferId);
	AudioBufferID getBuffer() const;

	bool isValid() const;
	bool isPlaying() const;

	void setLooping(bool loop);
	bool getLooping() const;

	void setVolume(float volume);
	float getVolume() const;

	void setPitch(float pitch);
	float getPitch() const;
};