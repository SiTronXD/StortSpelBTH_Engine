#include "pch.h"
#include "AudioSource.h"
#include "al.h"

AudioSource::AudioSource(AudioSourceID sourceId)
	:sourceId(sourceId)
{

}

AudioSource::~AudioSource()
{
}

bool AudioSource::isValid() const
{
	return this->sourceId != ~0u;
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

void AudioSource::setPitch(float pitch)
{
	alSourcef(this->sourceId, AL_PITCH, pitch);
}

float AudioSource::getPitch() const
{
	float pitch = 0.f;
	alGetSourcef(this->sourceId, AL_PITCH, &pitch);
	return pitch;
}

void AudioSource::setBuffer(AudioBufferID bufferId)
{
	alSourcei(this->sourceId, AL_BUFFER, (int)bufferId);
}

AudioBufferID AudioSource::getBuffer() const
{
	ALint bufferId = 0;
	alGetSourcei(this->sourceId, AL_BUFFER, &bufferId);
	return (AudioBufferID)bufferId;
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
