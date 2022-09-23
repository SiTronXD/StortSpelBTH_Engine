#include "AudioEngine.h"
#include <iostream>
#include "../dev/Log.hpp"
#include <fstream>

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
}

int AudioEngine::loadFile(const char* filePath)
{
	static int ids = 0;

	std::ifstream reader(filePath);
	if (!reader.is_open())
		printf("File not found\n");
	else
		printf("File found\n");
	reader.close();

	if (!this->soundBuffers[ids].loadFromFile(filePath))
	{
		Log::error("Could not find file...");
		this->soundBuffers.erase(ids);
		return -1;
	}

	printf("Working?\n");
	// Should check if compatiable with 3D sound or not

	return ids++;
}

sf::SoundBuffer* AudioEngine::getBuffer(int id)
{
	if (this->soundBuffers.find(id) == this->soundBuffers.end())
		return nullptr;

	return &soundBuffers[id];
}
