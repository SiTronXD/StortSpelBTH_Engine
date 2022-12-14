#pragma once
#include "SFML/Audio/InputSoundFile.hpp"

class SceneHandler;

typedef unsigned int AudioSourceID;
typedef unsigned int AudioBufferID;
struct AudioSource;

class AudioHandler
{
public:
	static const uint32_t BUFFER_SIZE = 1024u * 16u;
	static const uint32_t NUM_BUFFERS = 4u;
	static const size_t NUM_SAMPLES_PER_READ = BUFFER_SIZE / sizeof(short);

	static const uint32_t MAX_SOURCES = 16u;

	enum State {Playing, NotPlaying};

private:
	SceneHandler* sceneHandler;

	// Music streaming
	sf::InputSoundFile mrStreamer;
	int alSoundFormat;
	State musicState;
	AudioSourceID musicSourceId;
	AudioBufferID alBuffers[NUM_BUFFERS];
	char* audioSamples;

	// Sources
	bool sourceBorrowed[MAX_SOURCES];
	Entity sourceUsers[MAX_SOURCES];
	AudioSourceID sources[MAX_SOURCES];
	uint32_t numActiveSources;
	void removeUsedSource(uint32_t index);

	void updateMusic();

public:
	AudioHandler();
	~AudioHandler();

	// Master volume (Scales all in-game volumes)
	void setMasterVolume(float volume);
	float getMasterVolume() const;

	// Music streaming
	void setMusic(const std::string& filePath);
	void playMusic();
	void stopMusic();
	void pauseMusic();
	void setMusicVolume(float volume);
	float getMusicVolume() const;

	// Borrowing sources for more permanent use (ex, player walking or looping sounds)
	bool requestAudioSource(Entity entity, AudioBufferID bufferID = ~0u);
	void releaseAudioSource(Entity entity);

	// Playing a sound once (ex, swarm attack)
	AudioSourceID playSound(Entity entity, AudioBufferID bufferID, float volume = 1.f, float pitch = 1.f);
	void setSourceVolume(AudioSourceID sourceId, float volume);
	void setSourcePitch(AudioSourceID sourceId, float pitch);
	uint32_t getNumActiveSources() const;

	// Other
	void update();
	void setSceneHandler(SceneHandler* sceneHandler);
	void reset();
};