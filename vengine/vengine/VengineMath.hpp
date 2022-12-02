#pragma once
#include "glm/glm.hpp"
#include "glm/common.hpp"
#include "glm/gtc/quaternion.hpp"

class SMath
{
public:
    static inline const float PI = 3.141592f;

	static glm::vec3 getRandomVector(float scalar);
	static glm::vec3 rotateVectorByQuaternion(const glm::quat& quaternion, const glm::vec3& vector);
    static glm::vec3 extractTranslation(const glm::mat4& mat);

    static glm::mat4 rotateTowards(const glm::vec3& dir);
    static glm::mat4 rotateTowards(const glm::vec3& dir, const glm::vec3& up);
    static glm::mat4 rotateEuler(const glm::vec3& angles);
	static glm::vec3 rotateVector(const glm::vec3& angles, const glm::vec3 vector);

    static void normalizeScale(glm::mat4& matrix);
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

inline glm::vec3 SMath::extractTranslation(
    const glm::mat4& mat)
{
    return glm::vec3(
        mat[3][0],
        mat[3][1],
        mat[3][2]
    );
}

inline glm::mat4 SMath::rotateTowards(
    const glm::vec3& dir)
{
    // Forward
    glm::vec3 forwardVec = dir;
    forwardVec = glm::normalize(forwardVec);

    // World up
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(worldUp, forwardVec)) >= 0.95f)
        worldUp = glm::vec3(1.0f, 0.0f, 0.0f);

    // Right
    glm::vec3 rightVec = glm::cross(forwardVec, worldUp);
    rightVec = glm::normalize(rightVec);

    // Up
    glm::vec3 upVec = glm::cross(rightVec, forwardVec);
    upVec = glm::normalize(upVec);

    return glm::mat4(
        glm::vec4(rightVec, 0.0f),
        glm::vec4(upVec, 0.0f),
        glm::vec4(forwardVec, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

inline glm::mat4 SMath::rotateTowards(
    const glm::vec3& dir, 
    const glm::vec3& up)
{
    // Forward
    glm::vec3 forwardVec = dir;
    forwardVec = glm::normalize(forwardVec);

    // Right
    glm::vec3 rightVec = glm::cross(forwardVec, up);
    rightVec = glm::normalize(rightVec);

    // Up
    glm::vec3 upVec = glm::cross(rightVec, forwardVec);
    upVec = glm::normalize(upVec);

    return glm::mat4(
        glm::vec4(rightVec, 0.0f),
        glm::vec4(upVec, 0.0f),
        glm::vec4(forwardVec, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

inline glm::mat4 SMath::rotateEuler(const glm::vec3& angles)
{
    return 
        glm::rotate(glm::mat4(1.0f), glm::radians(angles.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(angles.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(angles.x), glm::vec3(1.0f, 0.0f, 0.0f));
}

inline glm::vec3 SMath::rotateVector(const glm::vec3& angles, glm::vec3 vector) {
	vector = glm::mat3(cos(glm::radians(angles.z)), -sin(glm::radians(angles.z)), 0.f, sin(glm::radians(angles.z)), cos(glm::radians(angles.z)), 0.f, 0.f, 0.f, 1.f) * vector;
	vector = glm::mat3(1.f, 0.f, 0.f, 0.f, cos(glm::radians(angles.x)), -sin(glm::radians(angles.x)), 0.f, sin(glm::radians(angles.x)), cos(glm::radians(angles.x))) * vector;
	vector = glm::mat3(cos(glm::radians(angles.y)), 0.f, sin(glm::radians(angles.y)), 0.f, 1.f, 0.f, -sin(glm::radians(angles.y)), 0.f, cos(glm::radians(angles.y))) * vector;

    return vector;
}

inline void SMath::normalizeScale(glm::mat4& matrix)
{
    glm::vec3 tempVec;
    for (uint32_t i = 0; i < 3; ++i)
    {
        tempVec = matrix[i];

        // Normalize vectors that are not zero vectors
        if (glm::dot(tempVec, tempVec) > 0.0f)
        {
            tempVec = glm::normalize(tempVec);
            matrix[i] = glm::vec4(tempVec, 0.0f);
        }
    }
}