#include "Randomizer.hpp"

glm::vec2 getRandomVec2(int min, int max)
{
    if (min >= max)
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec2(0.0f, 0.0f);
    }
    return glm::vec2(
        (float)(rand() % (max - min) + min),
        (float)(rand() % (max - min) + min)
    );
}

glm::vec2 getRandomVec2(float min, float max)
{
    if (min >= max)
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec2(0.0f, 0.0f);
    }
    return glm::vec2(
        (float)(rand() % ((int)max - (int)min) + min),
        (float)(rand() % ((int)max - (int)min) + min)
    );
}

glm::vec2 getRandomVec2(int xMin, int xMax, int yMin, int yMax)
{
    if (xMin >= xMin || yMin >= yMax) 
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec2(0.0f, 0.0f);
    }
    return glm::vec2(
        (float)(rand() % (xMax - xMin) + xMin),
        (float)(rand() % (yMax - yMin) + yMin)
    );
}

glm::vec2 getRandomVec2(float xMin, float xMax, float yMin, float yMax)
{
    if (xMin >= xMax || yMin >= yMax) 
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec2(0.0f, 0.0f);
    }
    return glm::vec2(
        (float)(rand() % ((int)xMax - (int)xMin) + xMin),
        (float)(rand() % ((int)yMax - (int)yMin) + yMin)
    );
}

glm::vec2 getRandomVec2(glm::vec2 min, glm::vec2 max)
{
    if (min.x >= max.x || min.y >= max.y) 
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec2(0.0f, 0.0f);
    }
    return glm::vec2(
        rand() % ((int)max.x - (int)min.x) + (int)min.x,
        rand() % ((int)max.y - (int)min.y) + (int)min.y
    );
}



glm::vec3 getRandomVec3(int min, int max)
{
    if (min >= max) 
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    return glm::vec3(
        (float)(rand() % (max - min) + min),
        (float)(rand() % (max - min) + min),
        (float)(rand() % (max - min) + min)
    );
}

glm::vec3 getRandomVec3(float min, float max)
{
    if (min >= max) 
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    return glm::vec3(
        (float)(rand() % ((int)max - (int)min) + min),
        (float)(rand() % ((int)max - (int)min) + min),
        (float)(rand() % ((int)max - (int)min) + min)
    );
}

glm::vec3 getRandomVec3(int xMin, int xMax, int yMin, int yMax, int zMin, int zMax)
{
    if (xMin >= xMax || yMin >= yMax || zMin >= zMax)
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    return glm::vec3(
        (float)(rand() % (xMax - xMin) + xMin),
        (float)(rand() % (yMax - yMin) + yMin),
        (float)(rand() % (zMax - zMin) + zMin)
    );
}

glm::vec3 getRandomVec3(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
{
    if (xMin >= xMax || yMin >= yMax || zMin >= zMax)
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    return glm::vec3(
        (float)(rand() % ((int)xMax - (int)xMin) + xMin),
        (float)(rand() % ((int)yMax - (int)yMin) + yMin),
        (float)(rand() % ((int)zMax - (int)zMin) + zMin)
    );
}

glm::vec3 getRandomVec3(glm::vec3 min, glm::vec3 max)
{
    if (min.x >= max.x || min.y >= max.y || min.z >= max.z)
    {
        std::cout << "Max needs to be greater than min" << std::endl;
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    return glm::vec3(
        rand() % ((int)max.x - (int)min.x) + (int)min.x,
        rand() % ((int)max.y - (int)min.y) + (int)min.y,
        rand() % ((int)max.z - (int)min.z) + (int)min.z
    );
}
