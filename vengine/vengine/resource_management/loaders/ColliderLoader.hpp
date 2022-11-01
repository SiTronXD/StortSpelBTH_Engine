#pragma once
#include "../../components/Collider.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <vector>

class Scene;
class NetworkScene;

struct ColliderDataRes
{
	glm::vec3 position;
	glm::vec3 rotation;
	Collider col;

	ColliderDataRes(const glm::vec3& pos, const glm::vec3& rot, const Collider col)
	{
		this->position = pos;
		this->rotation = rot;
		this->col = col;
	}
};

class ColliderLoader
{
  private:
	Assimp::Importer importer;
	ColType getShapeType(aiMesh* mesh, const std::string& meshName);
	Collider makeCollisionShape(const ColType& type, const aiMesh* mesh);

  public:
	std::vector<ColliderDataRes> loadCollisionShape(const std::string& modelFile);
};

void addCollisionToScene(
    std::vector<ColliderDataRes> colliders, Scene& currentScene, const glm::vec3& offset = glm::vec3(0, 0, 0),
    const glm::vec3& rotationOffset = glm::vec3(0, 0, 0)
);

void addCollisionToNetworkScene(
    std::vector<ColliderDataRes> colliders, NetworkScene& currentScene, const glm::vec3& offset = glm::vec3(0, 0, 0),
    const glm::vec3& rotationOffset = glm::vec3(0, 0, 0)
);
