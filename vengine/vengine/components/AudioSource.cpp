#include "pch.h"
#include "AudioSource.h"
#include "al.h"

AudioSource::AudioSource(uint32_t bufferId)
		:sourceId(sourceId), bufferId(bufferId), playingb4Inactive(false)
{
	alGenSources(1, &this->sourceId);
	if (alGetError() != AL_NO_ERROR)
    {
        Log::warning("AudioSource: Failed generating alAudioSource!\n");
        return;
    }

	alSourcei(this->sourceId, AL_BUFFER, bufferId);
	if (alGetError() != AL_NO_ERROR)
    {
        Log::warning("AudioSource: Failed attaching ALBufferId!\n");
        return;
    }
}

AudioSource::~AudioSource()
{
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
	alSourcei(this->sourceId, AL_BUFFER, bufferId);
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
