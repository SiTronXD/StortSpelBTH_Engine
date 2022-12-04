#pragma once 
 #include "op_overload.hpp"
#include <cstdint>
#include <vector>
#include "../components/AudioSource.h"

#define NUM_MAX_MULTI_AUDIOSOURCE 5
struct MultipleAudioSources
{
	AudioSource audioSource[NUM_MAX_MULTI_AUDIOSOURCE];

	MultipleAudioSources() = default;
	~MultipleAudioSources() = default;
};