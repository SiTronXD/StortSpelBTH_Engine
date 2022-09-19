#include "Randomizer.hpp"

glm::vec2 getRandomVec2(int min, int max)
{
    if (min >= max)
    {
        std::cout << "Max needs to be greater than min.\nReturning (0, 0)" << std::endl;
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
        std::cout << "Max needs to be greater than min\nReturning (0, 0)" << std::endl;
        return glm::vec2(0.0f, 0.0f);
    }
    return glm::vec2(
        (float)(rand() % ((int)max - (int)min) + min),
        (float)(rand() % ((int)max - (int)min) + min)
    );
}

glm::vec2 getRandomVec2(int xMin, int xMax, int yMin, int yMax)
{
    int x = 0;
    int y = 0;
    if (xMin < xMin)
    {
        x = rand() % (xMax - xMin) + xMin;
    }
    if (yMin < yMax)
    {
        y = rand() % (yMax - yMin) + yMin;
    }
    return glm::vec2((float)x, (float)y);
}

glm::vec2 getRandomVec2(float xMin, float xMax, float yMin, float yMax)
{
    float x = 0.0f;
    float y = 0.0f;
    if (xMin < xMin)
    {
        x = rand() % ((int)xMax - (int)xMin) + xMin;
    }
    if (yMin < yMax)
    {
        y = rand() % ((int)yMax - (int)yMin) + yMin;
    }
    return glm::vec2(x, y);
}

glm::vec2 getRandomVec2(glm::vec2 min, glm::vec2 max)
{
    float x = 0.0f;
    float y = 0.0f;
    if (min.x < max.x)
    {
        x = rand() % ((int)max.x - (int)min.x) + min.x;
    }
    if (min.y < max.y)
    {
        y = rand() % ((int)max.y - (int)min.y) + min.y;
    }
    return glm::vec2(x, y);
}



glm::vec3 getRandomVec3(int min, int max)
{
    if (min >= max) 
    {
        std::cout << "Max needs to be greater than min\nReturning (0, 0)" << std::endl;
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
        std::cout << "Max needs to be greater than min\nReturning (0, 0)" << std::endl;
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

    int x = 0;
    int y = 0;
    int z = 0;
    if (xMin < xMin)
    {
        x = rand() % (xMax - xMin) + xMin;
    }
    if (yMin < yMax)
    {
        y = rand() % (yMax - yMin) + yMin;
    }
    if (zMin < zMax)
    {
        z = rand() % (zMax - zMin) + zMin;
    }

    return glm::vec3(x, y, z);
}

glm::vec3 getRandomVec3(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (xMin < xMax)
    {
        x = rand() % ((int)xMax - (int)xMin) + xMin;
    }
    if (yMin < yMax)
    {
        y = rand() % ((int)yMax - (int)yMin) + yMin;
    }
    if (zMin < zMax)
    {
        z = rand() % ((int)zMax - (int)zMin) + zMin;
    }
    return glm::vec3(x, y, z);
}

glm::vec3 getRandomVec3(glm::vec3 min, glm::vec3 max)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (min.x < max.x)
    {
        x = rand() % ((int)max.x - (int)min.x) + min.x;
    }
    if (min.y < max.y)
    {
        y = rand() % ((int)max.y - (int)min.y) + min.y;
    }
    if (min.z < max.z)
    {
        z = rand() % ((int)max.z - (int)min.z) + min.z;
    }
    return glm::vec3(x, y, z);
}
