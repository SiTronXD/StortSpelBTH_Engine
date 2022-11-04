#include "AudioHandler.h"
#include "../application/SceneHandler.hpp"
#include "OpenAL/alc.h"


AudioHandler::AudioHandler()
	:sceneHandler(nullptr)
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
}

AudioHandler::~AudioHandler()
{
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
		AudioSourceId id = sourceView.get<AudioSource>(entity).sourceId;
		const glm::vec3& pos = sourceView.get<Transform>(entity).position;
		alSource3f(id, AL_POSITION, pos.x, pos.y, pos.z);
	}

	const Entity camID = scene->getMainCameraID();
	if (scene->hasComponents<AudioListener>(camID) && scene->isActive(camID))
	{
		const AudioListener& listener = scene->getComponent<AudioListener>(camID);
		const Transform& camTra = scene->getComponent<Transform>(camID);
		
		const glm::vec3 forward = camTra.forward();
		const glm::vec3 up = camTra.up();
		const float orientation[6]{forward.x, forward.y, forward.z, up.x, up.y, up.z};

		alListenerfv(AL_ORIENTATION, orientation);
		alListener3f(AL_POSITION, camTra.position.x, camTra.position.y, camTra.position.z);
	}
	else
	{
		alListenerf(AL_GAIN, 0.f);
	}
}

void AudioHandler::setMasterVolume(float volume)
{
	Scene* scene = this->sceneHandler->getScene();
	const Entity camID = scene->getMainCameraID();
	if (scene->hasComponents<AudioListener>(camID) && scene->isActive(camID))
	{
		scene->getComponent<AudioListener>(scene->getMainCameraID()).setVolume(volume);
	}
	else
	{
		alListenerf(AL_GAIN, volume);
	}
}

float AudioHandler::getMasterVolume() const
{
	float volume = 0.f;
	alGetListenerf(AL_GAIN, &volume);
	return volume;
}

void AudioHandler::setMusicBuffer(const sf::SoundBuffer& musicBuffer)
{
	this->music.setBuffer(musicBuffer);
	this->music.setRelativeToListener(false);
}

void AudioHandler::setMusicVolume(float volume)
{
	this->music.setVolume(volume);
}

void AudioHandler::playMusic(bool loop)
{
	this->music.setLoop(loop);
	this->music.play();
}

void AudioHandler::pauseMusic()
{
	this->music.pause();
}

void AudioHandler::stopMusic()
{
	this->music.stop();
}

void AudioHandler::cleanUp()
{
	ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}