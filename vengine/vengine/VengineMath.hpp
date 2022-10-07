#pragma once
#include "glm/glm.hpp"
#include "glm/common.hpp"
#include "glm/gtc/quaternion.hpp"

class SMath
{
 public:

	 static glm::vec3 getRandomVector(float scalar);

	 static glm::vec3 rotateVectorByQuaternion(const glm::quat& quaternion, const glm::vec3& vector);
};

inline glm::vec3 SMath::getRandomVector(float scalar) 
{
    return glm::vec3(
        ((rand() % 201) * 0.01f - 1.f) * scalar,
        ((rand() % 201) * 0.01f - 1.f) * scalar,
        ((rand() % 201) * 0.01f - 1.f) * scalar);
}

inline glm::vec3 SMath::rotateVectorByQuaternion(const glm::quat& quaternion, const glm::vec3& vector)
{
  return vector + (2.f * glm::cross(glm::vec3(quaternion.x, quaternion.y, quaternion.z), 
        glm::cross(glm::vec3(quaternion.x, quaternion.y, quaternion.z), vector) + quaternion.w * vector));
}