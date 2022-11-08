#pragma once
#include "SFML/Audio/InputSoundFile.hpp"
#include "../VengineHelper.h"

class SceneHandler;

class AudioHandler
{
public:
	static const uint32_t BUFFER_SIZE = 1024u * 16u;
	static const uint32_t NUM_BUFFERS = 4u;
	static const size_t NUM_SAMPLES_PER_READ = BUFFER_SIZE / sizeof(short);

	enum State {Playing, NotPlaying};

private:
	SceneHandler* sceneHandler;

	// Music streaming
	sf::InputSoundFile mrStreamer;
	int alSoundFormat;

	State state;
	AudioSourceId musicSourceId;
	uint32_t alBuffers[NUM_BUFFERS];
	char* audioSamples;

	void updateMusic();

public:
	AudioHandler();
	~AudioHandler();

	void setSceneHandler(SceneHandler* sceneHandler);
	void cleanUp();

	// Update for source & listener position
	void update();

	// Set master volume, value between 0.0 - 1.0
	void setMasterVolume(float volume);
	float getMasterVolume() const;

	// Music streaming
	void setMusic(const std::string& filePath);
	void playMusic();
	void stopMusic();
	void pauseMusic();
	void setMusicVolume(float volume);
	float getMusicVolume() const;
};