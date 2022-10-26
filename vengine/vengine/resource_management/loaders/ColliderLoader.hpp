#pragma once
#include <assimp/Importer.hpp>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <assimp/scene.h>
#include <iostream>

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
	int getShapeType(aiMesh* mesh);
	btCollisionShape* makeCollisionShape(const shapeType &type, const aiMesh* mesh);

  public:
	std::vector<btCollisionShape*> loadCollisionShape(const std::string& modelFile);
};
