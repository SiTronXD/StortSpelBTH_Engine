#pragma once 
 #include "op_overload.hpp"

#include <vector>

class VRandom
{
private:
    uint32_t seed = 0;

public:
    // Replace srand/rand
    void srand(const uint32_t& seed);
    uint32_t rand();
};
