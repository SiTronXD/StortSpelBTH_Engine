#pragma once
#include <assimp/Importer.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <assimp/scene.h>
#include <iostream>
#include "../../components/Collider.h"

class Scene;
class NetworkScene;

struct ColliderDataRes
{
	glm::vec3 position;
	glm::vec3 rotation;
	Collider col;

	ColliderDataRes(const glm::vec3 &pos, const glm::vec3 &rot, const Collider col) 
	{ 
		this->position = pos;
		this->rotation = rot;
		this->col = col;
	}
};

class ColliderLoader
{
  private:
	enum shapeType
	{
		Sphere,
		Plane,
		Box,
		Cone,
		Cylinder,
		Capsule,
		Error
	};
	Assimp::Importer importer;
	int getShapeType(aiMesh* mesh, const std::string& meshName);
	Collider makeCollisionShape(const shapeType& type, const aiMesh* mesh);

  public:
	std::vector<ColliderDataRes> loadCollisionShape(const std::string& modelFile);
};

void addCollisionToScene(std::vector<ColliderDataRes> colliders, Scene& currentScene, const glm::vec3 &offset = glm::vec3(0, 0, 0), const glm::vec3 &rotationOffset = glm::vec3(0,0,0));
void addCollisionToNetworkScene(std::vector<ColliderDataRes> colliders, NetworkScene& currentScene, glm::vec3 offset = glm::vec3(0, 0, 0));
