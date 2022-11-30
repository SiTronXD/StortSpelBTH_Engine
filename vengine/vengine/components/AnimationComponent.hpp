#pragma once

#include "../graphics/ShaderInput.hpp"
#include "Transform.hpp"
#include "../graphics/MeshData.hpp"

class Mesh;

#define NUM_MAX_BONE_TRANSFORMS 64u
struct AnimationComponent
{
private:
	friend Mesh;

	glm::mat4 boneTransforms[NUM_MAX_BONE_TRANSFORMS];
	uint32_t numTransforms;

public:
    AnimationSlot aniSlots[NUM_MAX_ANIMATION_SLOTS];
	StorageBufferID boneTransformsID;

	AnimationComponent() :
		boneTransformsID(~0u),
		numTransforms(0),
		boneTransforms{},
		aniSlots{}
	{ }

	inline const glm::mat4* getBoneTransformsData() const { return &this->boneTransforms[0]; }
};