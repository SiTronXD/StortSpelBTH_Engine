#pragma once
#include "glm/glm.hpp"
#include <iostream>


glm::vec2 getRandomVec2(int min, int max);
glm::vec2 getRandomVec2(float min, float max);
glm::vec2 getRandomVec2(int xMin, int xMax, int yMin, int yMax);
glm::vec2 getRandomVec2(float xMin, float xMax, float yMin, float yMax);
glm::vec2 getRandomVec2(glm::vec2 min, glm::vec2 max);

glm::vec3 getRandomVec3(int min, int max);
glm::vec3 getRandomVec3(float min, float max);
glm::vec3 getRandomVec3(int xMin, int xMax, int yMin, int yMax, int zMin, int zMax);
glm::vec3 getRandomVec3(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
glm::vec3 getRandomVec3(glm::vec3 min, glm::vec3 max);