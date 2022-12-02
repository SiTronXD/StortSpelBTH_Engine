#include "pch.h"
#include "AudioSource.h"
#include "al.h"

AudioSource::AudioSource(uint32_t bufferId)
		:sourceId(sourceId), playingb4Inactive(false)
{
	ALenum error = 0;

	alGetError(); // Clear error queue
	alGenSources(1, &this->sourceId);
	if ((error = alGetError()) != AL_NO_ERROR)
    {
        Log::error("AudioSource: Failed generating alAudioSource! OpenAL error: " + std::to_string(error));
        return;
    }

	alGetError(); // Clear error queue
	alSourcei(this->sourceId, AL_BUFFER, (int)bufferId);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		Log::error("AudioSource: Failed attaching ALBufferId! OpenAL error: " + std::to_string(error));
		return;
	}
}

AudioSource::AudioSource()
	:playingb4Inactive(false)
{
	ALenum error = 0;

	alGetError(); // Clear error queue
	alGenSources(1, &this->sourceId);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		Log::error("AudioSource: Failed generating alAudioSource! OpenAL error: " + std::to_string(error));
		return;
	}
}

AudioSource::~AudioSource()
{
	alSourcei(this->sourceId, AL_BUFFER, NULL);
	alDeleteSources(1, &this->sourceId);
}

void AudioSource::setVolume(float volume)
{
	alSourcef(this->sourceId, AL_GAIN, volume);
}

float AudioSource::getVolume() const
{
	float volume = 0.f;
	alGetSourcef(this->sourceId, AL_GAIN, &volume);
	return volume;
}

void AudioSource::setBuffer(uint32_t bufferId)
{
	alSourcei(this->sourceId, AL_BUFFER, NULL);
	alSourcei(this->sourceId, AL_BUFFER, (int)bufferId);
}

int AudioSource::getBuffer() const
{
	int bufferId = -1;
	alGetSourcei(this->sourceId, AL_BUFFER, &bufferId);
	return bufferId;
}

void AudioSource::setLooping(bool loop)
{
	alSourcei(this->sourceId, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

bool AudioSource::getLooping() const
{
	int looping;
	alGetSourcei(this->sourceId, AL_LOOPING, &looping);
	return looping;
}

void AudioSource::play()
{
	alSourcePlay(this->sourceId);
}

void AudioSource::stop()
{
	alSourceStop(this->sourceId);
}

void AudioSource::pause()
{
	alSourcePause(this->sourceId);
}

bool AudioSource::isPlaying() const
{
	ALint state;
	alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}
