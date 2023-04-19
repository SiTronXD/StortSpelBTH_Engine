#include "pch.h"
#include "AudioHandler.h"
#include "../application/SceneHandler.hpp"
#include "alc.h"
#include "al.h"


AudioHandler::AudioHandler()
	:sceneHandler(nullptr), musicSourceId(), alBuffers{}, 
	alSoundFormat(0), sourceUsers{}, sourceBorrowed{}
{
	this->audioSamples = new char[BUFFER_SIZE];
	this->musicState = State::NotPlaying;

#ifdef _CONSOLE
	ALCdevice* device = alcOpenDevice(NULL);
    if (!device)
    {
		Log::error( std::string(__func__) + " | Failed to create OpenAL Device");
    }
    ALCcontext* context = alcCreateContext(device, nullptr);
	if (!context)
	{
		Log::error(std::string(__func__) + " | Failed to create OpenAL Context");
	}
    alcMakeContextCurrent(context);
	alGetError(); // Clear error code

	ALenum error = AL_NO_ERROR;

	alGenSources(1, &this->musicSourceId);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		Log::error(std::string(__func__) + " | Failed generating music source. OpenAL error: " + std::to_string(error));
	}
	alSourcei(this->musicSourceId, AL_SOURCE_RELATIVE, AL_FALSE);

	this->numActiveSources = 0u;
	alGenSources(MAX_SOURCES, this->sources);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		Log::error(std::string(__func__) + " | Failed generating audio sources. OpenAL error: " + std::to_string(error));
	}
#else
	ALCdevice* device = alcOpenDevice(NULL);
	ALCcontext* context = alcCreateContext(device, nullptr);
	alcMakeContextCurrent(context);

	alGenSources(1, &this->musicSourceId);
	alSourcei(this->musicSourceId, AL_SOURCE_RELATIVE, AL_FALSE);

	this->numActiveSources = 0u;
	alGenSources(MAX_SOURCES, this->sources);
#endif

	this->reset();
}

AudioHandler::~AudioHandler()
{
	if (this->audioSamples)
	{
		delete[] this->audioSamples;
		this->audioSamples = nullptr;
	}

	alDeleteSources(1, &this->musicSourceId);
	alDeleteSources(MAX_SOURCES, this->sources);
	alDeleteBuffers(NUM_BUFFERS, this->alBuffers);

    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void AudioHandler::setSceneHandler(SceneHandler* sceneHandler)
{
	this->sceneHandler = sceneHandler;
}

void AudioHandler::update()
{
	Scene* scene = this->sceneHandler->getScene();

	// Check source status and update accordingly
	ALint state = AL_STOPPED;
	for (uint32_t i = 0; i < this->numActiveSources; i++)
	{
		if (scene->entityValid(this->sourceUsers[i]))
		{
			alGetSourcei(this->sources[i], AL_SOURCE_STATE, &state);
			if (state == AL_PLAYING)
			{
				const glm::vec3& pos = scene->getComponent<Transform>(this->sourceUsers[i]).position;
				alSource3f(this->sources[i], AL_POSITION, pos.x, pos.y, pos.z);
			}
			else if (!this->sourceBorrowed[i])
			{
				this->removeUsedSource(i);

				// Make sure the last source is also updated
				i--; 
			}
		}
		else
		{
			this->removeUsedSource(i);
		}
	}

	// Position and orient the AL listener with the camera
	const Entity camID = scene->getMainCameraID();
	if (scene->isActive(camID))
	{
		Transform& camTra = scene->getComponent<Transform>(camID);
		
		const glm::vec3 forward = camTra.forward();
		const glm::vec3 up = camTra.up();
		const float orientation[6]{forward.x, forward.y, forward.z, up.x, up.y, up.z};

		alListenerfv(AL_ORIENTATION, orientation);
		alListener3f(AL_POSITION, camTra.position.x, camTra.position.y, camTra.position.z);
	}

	if (this->musicState == State::Playing)
	{
		this->updateMusic();
	}
}

void AudioHandler::removeUsedSource(uint32_t index)
{
	alSourcei(this->sources[index], AL_BUFFER, NULL);

	this->numActiveSources--;
	std::swap(this->sources[index], this->sources[this->numActiveSources]);
	std::swap(this->sourceUsers[index], this->sourceUsers[this->numActiveSources]);
	std::swap(this->sourceBorrowed[index], this->sourceBorrowed[this->numActiveSources]);
}

void AudioHandler::updateMusic()
{
	ALint buffersProcessed = 0;

    alGetSourcei(this->musicSourceId, AL_BUFFERS_PROCESSED, &buffersProcessed);
    if (buffersProcessed < 1)
	{
        return;
	}

    ALuint oldBuffer;
	size_t actualRead = 0;

	// Requeue all proccessed buffers and restart music if necessary
    while (buffersProcessed--)
    {
        alSourceUnqueueBuffers(this->musicSourceId, 1, &oldBuffer);

        actualRead = this->mrStreamer.read((short*)this->audioSamples, NUM_SAMPLES_PER_READ);
        alBufferData(oldBuffer, this->alSoundFormat, this->audioSamples, actualRead * sizeof(short), this->mrStreamer.getSampleRate());
        alSourceQueueBuffers(this->musicSourceId, 1, &oldBuffer);

        if (this->mrStreamer.getSampleOffset() >= this->mrStreamer.getSampleCount())
        {
            this->mrStreamer.seek(0ull);
        }
    }

	// Resume music if all buffers were processed
	ALint alMusicState = AL_STOPPED;
	alGetSourcei(this->musicSourceId, AL_SOURCE_STATE, &alMusicState);
	if (alMusicState == AL_STOPPED)
	{
		alSourcePlay(this->musicSourceId);
	}
}

void AudioHandler::setMasterVolume(float volume)
{
	alListenerf(AL_GAIN, volume);
}

float AudioHandler::getMasterVolume() const
{
	float volume = 0.f;
	alGetListenerf(AL_GAIN, &volume);
	return volume;
}

void AudioHandler::setMusic(const std::string& filePath)
{
#ifdef _CONSOLE
	ALenum error = 0;
	alGetError(); // Clears error code
#endif // _CONSOLE

	if (!this->mrStreamer.openFromFile(filePath))
	{
#ifdef _CONSOLE
		Log::warning(std::string(__func__) + " | Failed loading music file");
#endif
		return;
	}

#ifdef _CONSOLE
	alSourceStop(this->musicSourceId);
	if ((error = alGetError()) != AL_NO_ERROR) 
		{ Log::error(std::string(__func__) + " | Failed stopping music. OpenAL error: " + std::to_string(error)); return; }

	alSourcei(this->musicSourceId, AL_BUFFER, NULL);
	if ((error = alGetError()) != AL_NO_ERROR) 
		{ Log::error(std::string(__func__) + " | Failed deattching buffers. OpenAL error: " + std::to_string(error)); return; }

	alDeleteBuffers(NUM_BUFFERS, this->alBuffers);
	if ((error = alGetError()) != AL_NO_ERROR) 
		{ Log::error(std::string(__func__) + " | Failed deleting old buffers. OpenAL error: " + std::to_string(error)); return; }

	alGenBuffers(NUM_BUFFERS, this->alBuffers);
	if ((error = alGetError()) != AL_NO_ERROR) 
		{ Log::error(std::string(__func__) + " | Failed generating new buffers. OpenAL error: " + std::to_string(error)); return; }
#else
	alSourceStop(this->musicSourceId);
	alSourcei(this->musicSourceId, AL_BUFFER, NULL);
	alDeleteBuffers(NUM_BUFFERS, this->alBuffers);
	alGenBuffers(NUM_BUFFERS, this->alBuffers);
#endif

	this->alSoundFormat = this->mrStreamer.getChannelCount() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	memset(this->audioSamples, 0, BUFFER_SIZE);

#ifdef _CONSOLE
	alGetError(); // Clear error queue (just in case)
#endif

	size_t numSamplesRead = 0;
	for (int i = 0; i < NUM_BUFFERS; i++)
    {
        numSamplesRead = this->mrStreamer.read((short*)this->audioSamples, NUM_SAMPLES_PER_READ);

	    alBufferData(this->alBuffers[i], this->alSoundFormat, this->audioSamples, numSamplesRead * sizeof(short), this->mrStreamer.getSampleRate());
#ifdef _CONSOLE
        if ((error = alGetError()) != AL_NO_ERROR)
		{
			Log::error(std::string(__func__) + " | Failed filling music buffers. ALError: " + std::to_string(error));
		}
#endif
    }

	alSourceQueueBuffers(this->musicSourceId, NUM_BUFFERS, this->alBuffers);
#ifdef _CONSOLE
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		Log::error(std::string(__func__) + " | Failed filling queueing music buffers. ALError: " + std::to_string(error));
	}
#endif // _CONSOLE
}

void AudioHandler::playMusic()
{
	alSourcePlay(this->musicSourceId);
	this->musicState = State::Playing;
}

void AudioHandler::stopMusic()
{
	alSourceStop(this->musicSourceId);
	this->musicState = State::NotPlaying;
}

void AudioHandler::pauseMusic()
{
	alSourcePause(this->musicSourceId);
	this->musicState = State::NotPlaying;
}

void AudioHandler::setMusicVolume(float volume)
{
	alSourcef(this->musicSourceId, AL_GAIN, volume);
}

float AudioHandler::getMusicVolume() const
{
	float volume;
	alGetSourcef(this->musicSourceId, AL_GAIN, &volume);
	return volume;
}

bool AudioHandler::requestAudioSource(Entity entity, AudioBufferID bufferID)
{
	if (this->numActiveSources >= MAX_SOURCES)
	{
#ifdef _CONSOLE
		Log::error(std::string(__func__) + " | No avaliable sources!");
#endif
		return false;
	}

	Scene* scene = this->sceneHandler->getScene();

	scene->setComponent<AudioSource>(entity, this->sources[this->numActiveSources]);
	alSourcei(this->sources[this->numActiveSources], AL_BUFFER, bufferID);
	this->sourceUsers[this->numActiveSources] = entity;
	this->sourceBorrowed[this->numActiveSources] = true;

	this->numActiveSources++;
	return true;
}

void AudioHandler::releaseAudioSource(Entity entity)
{
	Scene* scene = this->sceneHandler->getScene();
	if (scene->hasComponents<AudioSource>(entity))
	{
		AudioSource& source =scene->getComponent<AudioSource>(entity);
		source.stop();
		source.setBuffer(0u);

		for (uint32_t i = 0; i < this->numActiveSources; i++)
		{
			if (entity == this->sourceUsers[i])
			{
				this->numActiveSources--;
				std::swap(this->sources[i], this->sources[this->numActiveSources]);
				std::swap(this->sourceUsers[i], this->sourceUsers[this->numActiveSources]);
				std::swap(this->sourceBorrowed[i], this->sourceBorrowed[this->numActiveSources]);
				scene->removeComponent<AudioSource>(entity);

				return;
			}
		}
	}
#ifdef _CONSOLE
	else
	{
		Log::warning(std::string(__func__) + " | Entity doesn't have an AudioSource");
	}
#endif
}

AudioSourceID AudioHandler::playSound(Entity entity, AudioBufferID bufferID, float volume, float pitch)
{
	if (this->numActiveSources >= MAX_SOURCES)
	{
#ifdef _CONSOLE
		// Log::warning(std::string(__func__) + " | No avaliable sources!");
#endif
		return ~0u;
	}

	if (!this->sceneHandler->getScene()->entityValid(entity))
	{
		return ~0u;
	}

	const AudioSourceID curSourceId = this->sources[this->numActiveSources];
	const glm::vec3& pos = this->sceneHandler->getScene()->getComponent<Transform>(entity).position;

	// Save user and reset borrowed
	this->sourceUsers[this->numActiveSources] = entity;
	this->sourceBorrowed[this->numActiveSources] = false;
	this->numActiveSources++;

	// Set parameters 
	alSource3f(curSourceId, AL_POSITION, pos.x, pos.y, pos.z);
	alSourcef(curSourceId, AL_GAIN, volume);
	alSourcef(curSourceId, AL_PITCH, pitch);
	alSourcei(curSourceId, AL_BUFFER, bufferID);
	alSourcePlay(curSourceId);

	return curSourceId;
}

void AudioHandler::setSourceVolume(AudioSourceID sourceId, float volume)
{
	alSourcef(sourceId, AL_GAIN, volume);
}
void AudioHandler::setSourcePitch(AudioSourceID sourceId, float pitch)
{
	alSourcef(sourceId, AL_PITCH, pitch);
}

uint32_t AudioHandler::getNumActiveSources() const
{
	return this->numActiveSources;
}

void AudioHandler::reset()
{
	this->numActiveSources = 0u;
	for (uint32_t i = 0; i < MAX_SOURCES; i++)
	{
		alSourceStop(this->sources[i]);
		alSource3f(this->sources[i], AL_POSITION, 0.f, 0.f, 0.f);
		alSourcef(this->sources[i], AL_GAIN, 1.f);
		alSourcef(this->sources[i], AL_PITCH, 1.f);
		alSourcei(this->sources[i], AL_BUFFER, NULL);
		
		this->sourceBorrowed[i] = false;
		this->sourceUsers[i] = -1;
	}
}
