#include "MeshLoader.hpp"

#include "../Configurator.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <span>
#include <stack>
#include "TextureLoader.hpp"
#include "tracy/Tracy.hpp"
#include "../ResourceManager.hpp" // Importing mesh with Assimp needs to add Textures Sampler index
#include "../../graphics/Mesh.hpp"
#include "../../graphics/MeshData.hpp"

void MeshLoader::init(VmaAllocator *vma,
    vk::PhysicalDevice *physiscalDev,
    Device *dev, vk::Queue *transQueue,
    vk::CommandPool *transCmdPool,
    ResourceManager* resourceMan) 
{
    this->importStructs.vma = vma;
    this->importStructs.physicalDevice = physiscalDev;
    this->importStructs.device = dev;
    this->importStructs.transferQueue = transQueue;
    this->importStructs.transferCommandPool = transCmdPool;
    this->resourceMan = resourceMan;
}

void MeshLoader::setTextureLoader(TextureLoader* textureLoader)
{
    this->textureLoader = textureLoader;
}


Mesh MeshLoader::createMesh(std::string modelFile) 
{
    auto meshData = this->assimpImport(modelFile);

    return std::move(Mesh(std::move(meshData), this->importStructs));
}

MeshData MeshLoader::assimpImport(const std::string &modelFile) 
{

    // Import Model Scene
    using namespace vengine_helper::config;
    const aiScene *scene = this->importer.ReadFile(
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
    this->textureLoader->assimpTextureImport(scene, materailToTexture);
    
    MeshData meshData = this->assimpMeshImport(scene, materailToTexture);
    this->importer.FreeScene();
    return meshData;
}



MeshData MeshLoader::assimpMeshImport(const aiScene *scene,
    std::vector<uint32_t> &materailToTexture)
{  
    // Load in all meshes
    std::vector<MeshData> modelMeshes =
        this->getMeshesFromNodeTree(scene, materailToTexture);

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

        data.aniVertices.insert(data.aniVertices.end(), mesh.aniVertices.begin(),
                                mesh.aniVertices.end());

        data.bones.insert(data.bones.end(), mesh.bones.begin(), mesh.bones.end());
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

            if (scenes_meshes[node_meshes[j]]->HasBones()) {
                loadBones(scene, scenes_meshes[node_meshes[j]], meshList.back());
            }
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
      indices.push_back(faceIndicies[j] + initialVertex);
      index_index++;
    }
  }

  lastVertice += vertex_index;
  lastIndex += indices.size();
  
  /// Construct MeshData and return it
  MeshData meshData{
      .submeshes = std::vector<SubmeshData>{
          SubmeshData{
          .materialIndex =
              this->resourceMan->getTexture(matToTex[mesh->mMaterialIndex])
                  .descriptorLocation,
          .startIndex = initialIndex,
          .numIndicies = static_cast<uint32_t>(indices.size()),
      }},
      .vertices = vertices,
      .indicies = indices,
  };

  return meshData;
}

bool MeshLoader::loadBones(const aiScene* scene, aiMesh* mesh, MeshData& outBoneData)
{
    aiAnimation* ani = scene->mAnimations[0];
    if (!ani) {
        return false;
    }

    outBoneData.aniVertices.resize(outBoneData.vertices.size());
    outBoneData.bones.resize(mesh->mNumBones);

    // boneIndices used for getting parentIndex
    std::unordered_map<std::string_view, int> boneIndices;

    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        boneIndices[mesh->mBones[i]->mName.C_Str()] = i;
        Bone& bone = outBoneData.bones[i];

        // Get the inverse bind pose matrix
        memcpy(&bone.inverseBindPoseMatrix, &mesh->mBones[i]->mOffsetMatrix, sizeof(glm::mat4));
        bone.inverseBindPoseMatrix = glm::transpose(bone.inverseBindPoseMatrix);

        // Set weights & boneIndex
        for (unsigned int k = 0; k < mesh->mBones[i]->mNumWeights; k++) {
            aiVertexWeight& aiWeight = mesh->mBones[i]->mWeights[k];
            AnimVertex&     vertex   = outBoneData.aniVertices[aiWeight.mVertexId];

            if      (vertex.weights[0] < 0.f) { vertex.weights[0] = aiWeight.mWeight; vertex.bonesIndex[0] = i; }
            else if (vertex.weights[1] < 0.f) { vertex.weights[1] = aiWeight.mWeight; vertex.bonesIndex[1] = i; }
            else if (vertex.weights[2] < 0.f) { vertex.weights[2] = aiWeight.mWeight; vertex.bonesIndex[2] = i; }
            else if (vertex.weights[3] < 0.f) { vertex.weights[3] = aiWeight.mWeight; vertex.bonesIndex[3] = i; }
        }

        // The order of aiMesh::mBones does not always match aiAnimation::mChannels
        aiNodeAnim* nodeAnim = findAnimationNode(ani->mChannels, ani->mNumChannels, mesh->mBones[i]->mName.C_Str());
        if (!nodeAnim) {
            Log::error("Could not find animation node");
            
            // Create and default stamp[0] to avoid future errors
            bone.translationStamps.emplace_back(0.f, glm::vec3(0.f));
            bone.rotationStamps.emplace_back(0.f, glm::vec4(0.f));
            bone.scaleStamps.emplace_back(0.f, glm::vec3(1.f));

            continue;
        }

        // Get all poses
        bone.translationStamps.resize(nodeAnim->mNumPositionKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++) {
            bone.translationStamps[k].first = (float)nodeAnim->mPositionKeys[k].mTime;
            memcpy(&bone.translationStamps[k].second, &nodeAnim->mPositionKeys[k].mValue, sizeof(glm::vec3));
        }

        bone.rotationStamps.resize(nodeAnim->mNumRotationKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++) {
            bone.rotationStamps[k].first = (float)nodeAnim->mRotationKeys[k].mTime;
            memcpy(&bone.rotationStamps[k].second, &nodeAnim->mRotationKeys[k].mValue, sizeof(glm::quat));
        }

        bone.scaleStamps.resize(nodeAnim->mNumScalingKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; k++) {
            bone.scaleStamps[k].first = (float)nodeAnim->mScalingKeys[k].mTime;
            memcpy(&bone.scaleStamps[k].second, &nodeAnim->mScalingKeys[k].mValue, sizeof(glm::vec3));
        }
    }

    // Find parent index
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiNode* boneNode = findNode(scene->mRootNode, mesh->mBones[i]->mName.C_Str());
        if (!boneNode) {
            outBoneData.bones[i].parentIndex = -1;
            continue;
        }

        aiNode* parent = findParentBoneNode(boneIndices, boneNode);
        outBoneData.bones[i].parentIndex = parent ? boneIndices[parent->mName.C_Str()] : -1;
    }

    // Normalize & fix invalid weights
    for (AnimVertex& vertex : outBoneData.aniVertices) {

         if (vertex.weights[0] < 0.f) vertex.weights[0] = 0.f;
         if (vertex.weights[1] < 0.f) vertex.weights[1] = 0.f;
         if (vertex.weights[2] < 0.f) vertex.weights[2] = 0.f;
         if (vertex.weights[3] < 0.f) vertex.weights[3] = 0.f;

        float inverseSum = vertex.weights[0] + vertex.weights[1] + vertex.weights[2] + vertex.weights[3];
        if (inverseSum == 0.f) {
            continue;
        }

        inverseSum = 1.f / inverseSum;

        vertex.weights[0] *= inverseSum;
        vertex.weights[1] *= inverseSum;
        vertex.weights[2] *= inverseSum;
        vertex.weights[3] *= inverseSum;
    }

    return true;
}

aiNodeAnim* MeshLoader::findAnimationNode(aiNodeAnim** nodeAnims, unsigned int numNodes, std::string_view name)
{
    for (unsigned int i = 0; i < numNodes; i++) {
        if (nodeAnims[i]->mNodeName.C_Str() == name) {
            return nodeAnims[i];
        }
    }

    return nullptr;
}

aiNode* MeshLoader::findNode(aiNode* rootNode, std::string_view boneName)
{
    aiNode* ptr = nullptr;

    for (unsigned int i = 0; i < rootNode->mNumChildren; i++) {

        if (rootNode->mChildren[i]->mName.C_Str() == boneName) {
            ptr = rootNode->mChildren[i];
        }

        else if (!ptr) {
            ptr = findNode(rootNode->mChildren[i], boneName);
        }
    }

    return ptr;
}

aiNode* MeshLoader::findParentBoneNode(std::unordered_map<std::string_view, int>& bones, aiNode* node)
{
    if (!node) {
        return nullptr;
    }

    if (!node->mParent) {
        return nullptr;
    }

    if (bones.find(node->mParent->mName.C_Str()) != bones.end()) {
        return node->mParent;
    }

    return findParentBoneNode(bones, node->mParent);
}