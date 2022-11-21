#include "pch.h"
#include "Random.hpp"

void VRandom::srand(const uint32_t& seed)
{
    VRandom::seed = seed;
}

uint32_t VRandom::rand()
{
    // Wang hash
    uint32_t newSeed = uint32_t(this->seed ^ uint32_t(61)) ^ uint32_t(this->seed >> uint32_t(16));
    newSeed *= 9u;
    newSeed = newSeed ^ (newSeed >> 4);
    newSeed *= uint32_t(0x27d4eb2d);
    newSeed = newSeed ^ (newSeed >> 15);

    // Next seed
    this->seed++;

    return newSeed;
}