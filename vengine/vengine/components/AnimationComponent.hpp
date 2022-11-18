#pragma once

#include "../graphics/ShaderInput.hpp"
#include "Transform.hpp"

class Mesh;

#define NUM_MAX_BONE_TRANSFORMS 64u
struct AnimationComponent
{
private:
	friend Mesh;

	glm::mat4 boneTransforms[NUM_MAX_BONE_TRANSFORMS];
	uint32_t numTransforms;

public:
	float timer;
	float timeScale;
	StorageBufferID boneTransformsID;
	uint32_t animationIndex;

	AnimationComponent() :
		timer(0.0f),
		timeScale(1.0f),
		boneTransformsID(~0u),
		animationIndex(0),
		numTransforms(0),
		boneTransforms{}
	{ }

	/*inline glm::mat4 getJointTransform(
		Transform& animatedMeshTransform, 
		const uint32_t& index)
	{
		// Update matrix
		animatedMeshTransform.updateMatrix();
		return this->boneTransforms[index];
	}*/
	inline const glm::mat4* getBoneTransformsData() const { return &this->boneTransforms[0]; }
};