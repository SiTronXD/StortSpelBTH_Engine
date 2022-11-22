#pragma once

#include "../graphics/ShaderInput.hpp"
#include "Transform.hpp"
#include "../graphics/MeshData.hpp" // change to include only for AnimationPlayer

class Mesh;
struct AnimationPlayer;

#define NUM_MAX_BONE_TRANSFORMS 64u
struct AnimationComponent
{
private:
	friend Mesh;

	glm::mat4 boneTransforms[NUM_MAX_BONE_TRANSFORMS];
	uint32_t boneAniIndex[NUM_MAX_BONE_TRANSFORMS]; // remove?
	uint32_t numTransforms;

public:
    AnimationPlayer aniSlots[NUM_MAX_ANIMATION_SLOTS]; // move to private ?
	uint32_t numSlots; // should be able to remove

	StorageBufferID boneTransformsID;
	//uint32_t animationIndex;

	AnimationComponent() :
		boneTransformsID(~0u),
		//animationIndex(0),
		numTransforms(0),
		boneTransforms{},
		aniSlots{}, boneAniIndex{}
	{ }

	inline const glm::mat4* getBoneTransformsData() const { return &this->boneTransforms[0]; }
};