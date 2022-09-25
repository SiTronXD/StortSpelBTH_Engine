#include "MeshLoader.hpp"

#include "Configurator.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <span>
#include <stack>

#include "tracy/Tracy.hpp"
#include "ResourceManager.hpp" // Importing mesh with Assimp needs to add Textures Sampler index
#include "NewModel.hpp"
#include "TextureLoader.hpp"


Assimp::Importer MeshLoader::importer; 
VulkanImportStructs MeshLoader::importStructs;

void MeshLoader::init(VmaAllocator *vma,
                                     vk::PhysicalDevice *physiscalDev,
                                     Device *dev, vk::Queue *transQueue,
                                     vk::CommandPool *transCmdPool) {
  MeshLoader::importStructs.vma = vma;
  MeshLoader::importStructs.physicalDevice = physiscalDev;
  MeshLoader::importStructs.device = dev;
  MeshLoader::importStructs.transferQueue = transQueue;
  MeshLoader::importStructs.transferCommandPool = transCmdPool;
}


NewModel MeshLoader::createMesh(std::string modelFile) 
{
    auto meshData = MeshLoader::assimpImport(modelFile);

    return std::move(NewModel(std::move(meshData), MeshLoader::importStructs));
}

MeshData MeshLoader::assimpImport(const std::string &modelFile) 
{

    // Import Model Scene
    using namespace vengine_helper::config;
    const aiScene *scene = MeshLoader::importer.ReadFile(
        (modelFile).c_str(),
        aiProcess_Triangulate   // Ensures that ALL objects will be represented as
                                // Triangles
            | aiProcess_FlipUVs // Flips the texture UV values, to be same as how
                                // we use them
            | aiProcess_JoinIdenticalVertices // Saves memory by making sure no
                                            // dublicate vertices exists
    );
    if (scene == nullptr) {
    throw std::runtime_error("Failed to load model (" + modelFile + ")");
    }
    std::vector<uint32_t> materailToTexture;
    TextureLoader::assimpTextureImport(scene, materailToTexture);
    
    MeshData meshData = MeshLoader::assimpMeshImport(scene, materailToTexture);
    MeshLoader::importer.FreeScene();
    return meshData;
}



MeshData MeshLoader::assimpMeshImport(const aiScene *scene,
                             std::vector<uint32_t> &materailToTexture) 
{  
    // Load in all meshes
    std::vector<MeshData> modelMeshes =
        MeshLoader::getMeshesFromNodeTree(scene, materailToTexture);

    MeshData data;
    for (auto mesh : modelMeshes) {
    data.vertices.insert(data.vertices.end(), mesh.vertices.begin(),
                            mesh.vertices.end());
    data.indicies.insert(data.indicies.end(), mesh.indicies.begin(),
                            mesh.indicies.end());

    data.submeshes.push_back(SubmeshData{
        .materialIndex = mesh.submeshes[0].materialIndex,
        .startIndex = mesh.submeshes[0].startIndex,
        .numIndicies = static_cast<uint32_t>(mesh.indicies.size()),
    });
    }
    return data;
}

std::vector<MeshData> MeshLoader::getMeshesFromNodeTree(const aiScene *scene,
                                  const std::vector<uint32_t> &matToTex) 
{
#ifndef VENGINE_NO_PROFILING
  ZoneScoped; //: NOLINT
#endif

    std::vector<MeshData> meshList;
    auto *node = scene->mRootNode;
    auto scenes_meshes = std::span<aiMesh *>(scene->mMeshes, scene->mNumMeshes);
    auto node_children = std::span<aiNode *>(node->mChildren, node->mNumChildren);
    auto node_meshes = std::span<unsigned int>(node->mMeshes, node->mNumMeshes);

    uint32_t vertice_index = 0;
    uint32_t index_index = 0;

    std::stack<aiNode *> nodeStack;
    nodeStack.push(node);

    while (!nodeStack.empty()) {
    node_meshes = std::span<unsigned int>(node->mMeshes, node->mNumMeshes);
    for (size_t j = 0; j < node->mNumMeshes; j++) {
        meshList.push_back(loadMesh(scenes_meshes[node_meshes[j]], vertice_index,
                                    index_index, matToTex));
    }

    node_children = std::span<aiNode *>(node->mChildren, node->mNumChildren);

    for (auto *child : node_children) {
        nodeStack.push(child);
    }

    node = nodeStack.top();
    nodeStack.pop();
    }

    return meshList;
}

MeshData MeshLoader::loadMesh(aiMesh *mesh, uint32_t &lastVertice,
                              uint32_t &lastIndex,
                              std::vector<uint32_t> matToTex) 
{
#ifndef VENGINE_NO_PROFILING
  ZoneScoped; //: NOLINT
#endif

  uint32_t initialIndex = lastIndex;
  uint32_t initialVertex = lastVertice;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  const unsigned int sizeOTextureCoordsElement = 8;
  auto textureCordinates =
      std::span<aiVector3D *>(static_cast<aiVector3D **>(mesh->mTextureCoords),
                              sizeOTextureCoordsElement);

  /// Resize vertex list to hold all vertices for mesh
  vertices.resize(mesh->mNumVertices);
  int vertex_index = 0;
  int index_index = 0;

  /// For each veretx in our assimp mesh, define the Vertex in our format
  for (auto ai_vertice :
       std::span<aiVector3D>(mesh->mVertices, mesh->mNumVertices)) {
    /// copy the position vertex
    vertices[vertex_index].pos = {ai_vertice.x, ai_vertice.y, ai_vertice.z};
    /// copy the tex coordinate, if model have textures
    if (mesh->mTextureCoords[0] != nullptr) {
      vertices[vertex_index].tex = {textureCordinates[0][vertex_index].x,
                                    textureCordinates[0][vertex_index].y};
    } else { /// If no texture exists, then use 0,0 as default
      vertices[vertex_index].tex = {0.F, 0.F};
    }

    /// set Color, if we want to use it later...
    vertices[vertex_index].col = {1.F, 1.F, 1.F}; /// Color is white
    vertex_index++;
  }

  auto meshFaces = std::span<aiFace>(mesh->mFaces, mesh->mNumFaces);
  /// Copy Indicies
  for (size_t i = 0; i < mesh->mNumFaces; i++) {
    /// For every face, add it's indicies to our indicies list
    aiFace face = meshFaces[i];
    auto faceIndicies =
        std::span<unsigned int>(face.mIndices, face.mNumIndices);
    for (size_t j = 0; j < face.mNumIndices; j++) {
      // indices.push_back(face.mIndices[j]);
      indices.push_back(faceIndicies[j] + initialVertex);
      index_index++;
    }
  }

  lastVertice += vertex_index;
  lastIndex += indices.size();

  /// Construct MeshData and return it
  MeshData meshData{
      .submeshes = std::vector<SubmeshData>{{
          //.materialIndex = matToTex[mesh->mMaterialIndex],
          .materialIndex =
              ResourceManager::getTexture(matToTex[mesh->mMaterialIndex])
                  .descriptorLocation,
          .startIndex = initialIndex,
          .numIndicies = static_cast<uint32_t>(indices.size()),
      }},
      .vertices = vertices,
      .indicies = indices,
  };

  return meshData;
}