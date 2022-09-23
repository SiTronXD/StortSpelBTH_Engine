#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>

class AudioEngine
{
private:
	
	// Temp
	std::unordered_map<int, sf::SoundBuffer> soundBuffers;

public:
	AudioEngine();
	~AudioEngine();

	// Temp
	int loadFile(const char* filePath);

	sf::SoundBuffer* getBuffer(int id);
};