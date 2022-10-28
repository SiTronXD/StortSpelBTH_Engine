#pragma once
#include <assimp/Importer.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <assimp/scene.h>
#include <iostream>
#include "../../components/Collider.h"

class Scene;

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
	std::vector<std::pair<glm::vec3, Collider>> loadCollisionShape(const std::string& modelFile);
};

void addCollisionToScene(std::vector<std::pair<glm::vec3, Collider>> colliders, Scene* currentScene, glm::vec3 offset = glm::vec3(0,0,0));
