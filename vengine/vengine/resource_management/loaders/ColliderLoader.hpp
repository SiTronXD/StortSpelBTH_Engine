#pragma once
#include "../../components/Collider.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <iostream>
#include <vector>
#include "../../ai/lib/path_finder.h"

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

std::vector<int> addCollisionToScene(
    std::vector<ColliderDataRes> colliders, Scene& currentScene, const glm::vec3& offset = glm::vec3(0, 0, 0),
    const glm::vec3& rotationOffset = glm::vec3(0, 0, 0), const float& scaleOffset = 1
);

std::vector<std::vector<NavMesh::Point>> getPolygonsFromTile(
    std::vector<ColliderDataRes> colliders, 
    const glm::vec3& offset = glm::vec3(0, 0, 0), 
    const glm::vec3& rotationOffset = glm::vec3(0, 0, 0)
);

//probably don't need this
void addPolygonRoomToNetworkScene(
    NetworkScene* scene, 
    std::vector<std::vector<NavMesh::Point>> polygons,
    const glm::vec3& posOffset,
    const glm::vec3& rotationOffset
);
