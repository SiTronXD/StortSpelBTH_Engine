#include "pch.h"
#include "AudioHandler.h"
#include "../application/SceneHandler.hpp"
#include "OpenAL/alc.h"
#include "OpenAL/al.h"


AudioHandler::AudioHandler()
	:sceneHandler(nullptr), musicSourceId(), alBuffers{}, alSoundFormat(0)
{
	ALCdevice* device = alcOpenDevice(nullptr);
    if (!device)
    {
		Log::error("Failed creating OpenAL Device");
    }

    ALCcontext* context = alcCreateContext(device, nullptr);
	if (!context)
    {
		Log::error("Failed creating OpenAL Context");
    }
	
    alcMakeContextCurrent(context);
	ALboolean eax = alIsExtensionPresent("EAX2.0");
	
	alGetError(); // Clears error code

	alGenSources(1, &this->musicSourceId);
	alGenBuffers(NUM_BUFFERS, this->alBuffers);

	this->audioSamples = new char[BUFFER_SIZE];
	this->state = State::NotPlaying;
}

AudioHandler::~AudioHandler()
{
	if (this->audioSamples)
	{
		delete[] this->audioSamples;
		this->audioSamples = nullptr;
	}

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

	const auto& sourceView = scene->getSceneReg().view<AudioSource, Transform>(entt::exclude<Inactive>);
	for (const entt::entity& entity : sourceView)
	{
		const uint32_t id = sourceView.get<AudioSource>(entity).sourceId;
		const glm::vec3& pos = sourceView.get<Transform>(entity).position;
		alSource3f(id, AL_POSITION, pos.x, pos.y, pos.z);
	}

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

	if (this->state == State::Playing)
	{
		this->updateMusic();
	}
}

void AudioHandler::updateMusic()
{
	ALint buffersProcessed = 0;

    alGetSourcei(this->musicSourceId, AL_BUFFERS_PROCESSED, &buffersProcessed);
    if (buffersProcessed < 1)
        return;

    ALuint oldBuffer;
	size_t actualRead = 0;
    // while to to fix all processed buffers
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
	if (!this->mrStreamer.openFromFile(filePath))
	{
		Log::warning("Failed loading music file");
		return;
	}

	this->alSoundFormat = this->mrStreamer.getChannelCount() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	memset(this->audioSamples, 0, BUFFER_SIZE);

	size_t actualRead = 0;
	for (int i = 0; i < NUM_BUFFERS; i++)
    {
        actualRead = this->mrStreamer.read((short*)this->audioSamples, NUM_SAMPLES_PER_READ);

	    alBufferData(this->alBuffers[i], this->alSoundFormat, this->audioSamples, actualRead * sizeof(short), this->mrStreamer.getSampleRate());
		ALenum error = alGetError();
        if (error != AL_NO_ERROR)
		{
			Log::error("Failed filling audio buffers. ALError: " + std::to_string(error));
		}
    }
	alSourceQueueBuffers(this->musicSourceId, NUM_BUFFERS, this->alBuffers);
}

void AudioHandler::playMusic()
{
	alSourcePlay(this->musicSourceId);
	this->state = State::Playing;
}

void AudioHandler::stopMusic()
{
	alSourceStop(this->musicSourceId);
	this->state = State::NotPlaying;
}

void AudioHandler::pauseMusic()
{
	alSourcePause(this->musicSourceId);
	this->state = State::NotPlaying;
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

void AudioHandler::cleanUp()
{
	if (this->audioSamples)
	{
		delete[] this->audioSamples;
		this->audioSamples = nullptr;
	}

	const auto& sourceView = this->sceneHandler->getScene()->getSceneReg().view<AudioSource>();
	sourceView.each([&](AudioSource& source)
	{
		alDeleteSources(1, &source.sourceId);
	});
	
	alDeleteSources(1, &this->musicSourceId);
	alDeleteBuffers(NUM_BUFFERS, this->alBuffers);
}