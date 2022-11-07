#include "pch.h"
#include "AudioHandler.h"
#include "../application/SceneHandler.hpp"
#include "OpenAL/alc.h"
#include "OpenAL/al.h"


AudioHandler::AudioHandler()
	:sceneHandler(nullptr), musicSourceId(), alBuffers{}
{
	ALCdevice* device = alcOpenDevice(NULL);
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

	alGenSources(1, &this->musicSourceId);
	alGenBuffers(NUM_BUFFERS, this->alBuffers);

	this->audioSamples = new char[BUFFER_SIZE];
	this->state = State::NotPlaying;
}

AudioHandler::~AudioHandler()
{
	delete[] audioSamples;
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
		const AudioSourceId id = sourceView.get<AudioSource>(entity).sourceId;
		const glm::vec3& pos = sourceView.get<Transform>(entity).position;
		alSource3f(id, AL_POSITION, pos.x, pos.y, pos.z);
	}

	const Entity camID = scene->getMainCameraID();
	if (scene->isActive(camID))
	{
		const Transform& camTra = scene->getComponent<Transform>(camID);
		
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

    // while to to fix all processed buffers
    while (buffersProcessed--)
    {
        alSourceUnqueueBuffers(this->musicSourceId, 1, &oldBuffer);
        printf("Old buffer: %u\n", oldBuffer);

        this->mrStreamer.read((short*)this->audioSamples, NUM_SAMPLES_PER_READ);
        alBufferData(oldBuffer, this->alSoundFormat, this->audioSamples, BUFFER_SIZE, this->mrStreamer.getSampleRate());
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

	memset(this->audioSamples, 0, BUFFER_SIZE);
	for (int i = 0; i < NUM_BUFFERS; i++)
    {
        this->mrStreamer.read((short*)this->audioSamples, BUFFER_SIZE);

	    alBufferData(this->alBuffers[i], this->alSoundFormat, this->audioSamples, BUFFER_SIZE, this->mrStreamer.getSampleRate());
        if (alGetError() != AL_NO_ERROR)
		{
			Log::error("Failed filling audio buffers");
		}
    }
}

void AudioHandler::playMusic()
{
	alSourceRewind(this->musicSourceId);
	alSourcePlay(this->musicSourceId);
	this->state = State::Playing;
}

void AudioHandler::stopMusic()
{
	alSourceStop(this->musicSourceId);
	this->state = State::NotPlaying;
}

void AudioHandler::resumeMusic()
{
	alSourcePlay(this->musicSourceId);
	this->state = State::Playing;
}

void AudioHandler::setMusicVolume(float volume)
{
	alSourcef(this->musicSourceId, AL_GAIN, volume);
}

void AudioHandler::cleanUp()
{
	ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

	alDeleteSources(1, &this->musicSourceId);

}