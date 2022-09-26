#include "AudioHandler.h"
#include "../dev/Log.hpp"
#include "../application/SceneHandler.hpp"

#include <iostream>

std::unordered_map<int, sf::SoundBuffer> AudioHandler::soundBuffers;

AudioHandler::AudioHandler()
	:sceneHandler(nullptr)
{
}

AudioHandler::~AudioHandler()
{
}

void AudioHandler::setSceneHandler(SceneHandler* sceneHandler)
{
	this->sceneHandler = sceneHandler;
	this->sceneHandler->getScene()->setComponent<AudioListener>(this->sceneHandler->getScene()->getMainCameraID());
}

void AudioHandler::update()
{
	Scene* scene = sceneHandler->getScene();
	const auto& sourceView = scene->getSceneReg().view<AudioSource, Transform>();

	for (entt::entity entity : sourceView)
	{
		sourceView.get<AudioSource>(entity).setPosition(sourceView.get<Transform>(entity).position);
	}

	const int camID = scene->getMainCameraID();
	AudioListener& listener = scene->getComponent<AudioListener>(camID);
	Transform& camTra = scene->getComponent<Transform>(camID);

	listener.setPosition(camTra.position);
	listener.setOrientation(camTra.forward(), camTra.up());
}

int AudioHandler::loadFile(const char* filePath)
{
	static int ids = 0;

	if (!soundBuffers[ids].loadFromFile(filePath))
	{
		Log::error("Could not find file...");
		soundBuffers.erase(ids);
		return -1;
	}

	// Should check if file compatiable with 3D sound or not (mono or stereo)

	return ids++;
}

void AudioHandler::setMasterVolume(float volume)
{
	sf::Listener::setGlobalVolume(volume);
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

sf::SoundBuffer* AudioHandler::getBuffer(int id)
{
	if (soundBuffers.find(id) == soundBuffers.end())
		return nullptr;

	return &soundBuffers[id];
}