#pragma once
#include <SFML/Audio.hpp>
#include "glm/glm.hpp"

struct AudioSource
{
	sf::Sound sound;

	AudioSource() = default;
	AudioSource(const sf::SoundBuffer& buffer)
	{
		sound.setBuffer(buffer);
	}

	void setPosition(const glm::vec3& position)
	{
		sound.setPosition(position.x, position.y, position.z);
	}

	// Multiple sounds?
};