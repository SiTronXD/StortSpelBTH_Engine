#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>

class SceneHandler;

class AudioHandler
{
private:
	
	// Temp
	static std::unordered_map<int, sf::SoundBuffer> soundBuffers;

	SceneHandler* sceneHandler;
	sf::Sound music;

public:
	AudioHandler();
	~AudioHandler();

	// Temp
	static int loadFile(const char* filePath);
	static sf::SoundBuffer* getBuffer(int id);
	// ~Temp


	void setSceneHandler(SceneHandler* sceneHandler);
	void update();

	// Set master volume, value between 0 - 100
	void setMasterVolume(float volume);
	float getMasterVolume() const;

	// Set music buffer
	void setMusicBuffer(const sf::SoundBuffer& musicBuffer);

	// Set music volume, value between 0 - 100
	void setMusicVolume(float volume);
	void playMusic(bool loop);
	void pauseMusic();
	void stopMusic();
};