#include "pch.h"
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
#include "../../graphics/MeshDataInfo.hpp"

void MeshLoader::init(VmaAllocator *vma,
    PhysicalDevice* physicalDevice,
    Device* dev, 
    vk::Queue* transQueue,
    vk::CommandPool* transCmdPool,
    ResourceManager* resourceMan) 
{
    this->importStructs.vma = vma;
    this->importStructs.physicalDevice = physicalDevice;
    this->importStructs.device = dev;
    this->importStructs.transferQueue = transQueue;
    this->importStructs.transferCommandPool = transCmdPool;
    this->resourceMan = resourceMan;
}

void MeshLoader::setTextureLoader(TextureLoader* textureLoader)
{
    this->textureLoader = textureLoader;
}

MeshData MeshLoader::importMeshData(
    const std::string& modelPath,
    const std::string& texturesFolderPath)
{
    return this->assimpImport(modelPath, texturesFolderPath);
}

Mesh MeshLoader::createMesh(MeshData& data) 
{
    return std::move(Mesh(std::move(data), this->importStructs));
}

void MeshLoader::loadAnimations(const std::vector<std::string>& paths, const std::string& textures, MeshData& outMeshData)
{
    const aiScene* scene = nullptr;
    bool meshLoaded = false;

    for (const std::string& path : paths)
    {
        scene = this->importer.ReadFile(path.c_str(), 
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
        
        if (!scene) 
        { 
            Log::warning("Failed loading file \"" + path + "\"!");
            this->importer.FreeScene();
            continue;
        }
        if (!scene->mNumMeshes || !scene->mNumAnimations)
        {
            Log::warning("\"" + path + "\" contains no meshes or animations!");
            this->importer.FreeScene();
            continue;
        }

        // On first successful file read: Read mesh + skeleton and reserve memory for bones + animations
        if (!meshLoaded)
        {
            meshLoaded = true;

            std::vector<uint32_t> materialToTexture;
            this->textureLoader->assimpTextureImport(scene, textures, materialToTexture);

            uint32_t lastVertice = 0, lastIndex = 0;
            outMeshData = loadMesh(scene->mMeshes[0], lastVertice, lastIndex, materialToTexture);

            outMeshData.vertexStreams.boneWeights.resize((size_t)scene->mMeshes[0]->mNumVertices);
            outMeshData.vertexStreams.boneIndices.resize((size_t)scene->mMeshes[0]->mNumVertices);
            outMeshData.bones.resize((size_t)scene->mMeshes[0]->mNumBones);
            outMeshData.animations.reserve(paths.size());

            loadSkeleton(scene, outMeshData);
        }

        loadAnimation(scene, outMeshData);

        this->importer.FreeScene();
    }
}

MeshData MeshLoader::assimpImport(
    const std::string& modelFile,
    const std::string& texturesFolderPath) 
{
    // Import Model Scene
    using namespace vengine_helper::config;
    const aiScene *scene = this->importer.ReadFile(
        (modelFile).c_str(),
        aiProcess_Triangulate   // Ensures that ALL objects will be represented as
                                // Triangles
            | aiProcess_FlipUVs // Flips the texture UV values, to be same as how
                                // we use them
             | aiProcess_JoinIdenticalVertices // This causes issues 
                // for skeletal animations when using the latest version of assimp, but not
                // when using an earlier version... 
    );
    if (scene == nullptr)
    {
        Log::warning("Failed to load model (" + modelFile + ")");

        return MeshData();
    }
    std::vector<uint32_t> materialToTexture;
    this->textureLoader->assimpTextureImport(scene, texturesFolderPath, materialToTexture);
    
    MeshData meshData = this->assimpMeshImport(scene, materialToTexture);
    this->importer.FreeScene();

    // Make sure the vertex data streams are valid
    if (!MeshDataInfo::areStreamsValid(meshData.vertexStreams))
    {
        Log::error("There are different non-zero number of elements in certain vertex streams. This will cause issues when rendering the mesh...");
    }

    // Print out a warning if the index count is not divisible by 3 in a submesh...
    for (size_t i = 0; i < meshData.submeshes.size(); ++i)
    {
        // Check if this submesh numindices is divisible by 3
        SubmeshData& currentSubmesh = meshData.submeshes[i];
        if (currentSubmesh.numIndicies % 3 != 0)
        {
            Log::warning("Submesh [" + std::to_string(i) + "] in " + modelFile + " has an index count which is not divisible by 3. Index count: " + std::to_string(currentSubmesh.numIndicies));
            currentSubmesh.numIndicies = currentSubmesh.numIndicies - (currentSubmesh.numIndicies % 3);
            Log::warning("Submesh [" + std::to_string(i) + "] in " + modelFile + " had it's index count truncated to " + std::to_string(currentSubmesh.numIndicies));
        }
    }

    // We only allow 1 submesh for meshes with skeletal animations
    if (meshData.submeshes.size() > 1 && meshData.bones.size() > 0)
    {
        Log::warning("The engine does not support multiple submeshes for meshes with skeletal animations. Please merge the submeshes into 1 single submesh for " + modelFile);
    }

    return meshData;
}

MeshData MeshLoader::assimpMeshImport(const aiScene *scene, std::vector<uint32_t>& materialToTexture)
{  
    // Load in all meshes
    std::vector<MeshData> modelMeshes =
        this->getMeshesFromNodeTree(scene, materialToTexture);

    MeshData data;
    for (auto mesh : modelMeshes) 
    {
        // Vertex streams
        this->insertStream(
            mesh.vertexStreams.positions,
            data.vertexStreams.positions
        );
        this->insertStream(
            mesh.vertexStreams.normals,
            data.vertexStreams.normals
        );
        this->insertStream(
            mesh.vertexStreams.texCoords,
            data.vertexStreams.texCoords
        );
        this->insertStream(
            mesh.vertexStreams.boneWeights,
            data.vertexStreams.boneWeights
        );
        this->insertStream(
            mesh.vertexStreams.boneIndices,
            data.vertexStreams.boneIndices
        );

        // Indices
        data.indicies.insert(
            data.indicies.end(), 
            mesh.indicies.begin(),
            mesh.indicies.end()
        );

        // Submeshes
        data.submeshes.push_back(SubmeshData{
            .materialIndex = mesh.submeshes[0].materialIndex,
            .startIndex = mesh.submeshes[0].startIndex,
            .numIndicies = static_cast<uint32_t>(mesh.indicies.size()),
        });

        // Bones
        data.bones.insert(
            data.bones.end(), 
            mesh.bones.begin(), 
            mesh.bones.end()
        );
    }

    return data;
}

template <typename T>
void MeshLoader::insertStream(
    std::vector<T>& inStream, 
    std::vector<T>& outputStream)
{
    outputStream.insert(
        outputStream.end(),
        inStream.begin(),
        inStream.end()
    );
}

void MeshLoader::topologicallySortBones(
    aiMesh* mesh, aiNode* node, uint32_t& globalIndex
)
{
    // See if this node is a bone
	unsigned int currentBoneIndex = ~0u;
	bool isBone = false;
	for (unsigned int i = 0; i < mesh->mNumBones; ++i)
	{
		if (mesh->mBones[i]->mName == node->mName)
		{
			currentBoneIndex = i;
			isBone = true;
			break;
        }
    }

    // This node is a bone
    if (isBone)
	{
        // Swap bones
		aiBone* tempBone = mesh->mBones[currentBoneIndex];
		mesh->mBones[currentBoneIndex] = mesh->mBones[globalIndex];
		mesh->mBones[globalIndex] = tempBone;

		globalIndex++;
    }

    // Traverse through children
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		this->topologicallySortBones(mesh, node->mChildren[i], globalIndex);
    }
}

std::vector<MeshData> MeshLoader::getMeshesFromNodeTree(
    const aiScene* scene,
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

    while (!nodeStack.empty()) 
    {
        node_meshes = std::span<unsigned int>(node->mMeshes, node->mNumMeshes);
        for (size_t j = 0; j < node->mNumMeshes; j++)
        {
			std::string meshName(scenes_meshes[node_meshes[j]]->mName.C_Str());
			if (meshName.length() > 7 && meshName.substr(0, 8) == "Collider")
			{
                //its a collision mesh
			    break;
			}
            meshList.push_back(loadMesh(scenes_meshes[node_meshes[j]], vertice_index,
                                        index_index, matToTex));
        }

        node_children = std::span<aiNode *>(node->mChildren, node->mNumChildren);

        for (auto *child : node_children) 
        {
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

    VertexStreams vertexStreams;
    std::vector<uint32_t> indices;

    const unsigned int sizeOTextureCoordsElement = 8;
    auto textureCordinates =
        std::span<aiVector3D *>(static_cast<aiVector3D **>(mesh->mTextureCoords),
                                sizeOTextureCoordsElement);

    /// Resize vertex list to hold all vertices for mesh
    vertexStreams.positions.resize(mesh->mNumVertices);
    vertexStreams.normals.resize(mesh->mNumVertices);
    vertexStreams.texCoords.resize(mesh->mNumVertices);
    int vertex_index = 0;
    int index_index = 0;

    /// For each veretx in our assimp mesh, define the Vertex in our format
    for (auto aiVertex :
        std::span<aiVector3D>(mesh->mVertices, mesh->mNumVertices)) 
    {
        /// copy the position vertex
        vertexStreams.positions[vertex_index] = { aiVertex.x, aiVertex.y, aiVertex.z };
        
        /// copy the tex coordinate, if model have textures
        if (mesh->mTextureCoords[0] != nullptr) 
        {
            vertexStreams.texCoords[vertex_index] =
            {
                textureCordinates[0][vertex_index].x,
                textureCordinates[0][vertex_index].y
            };
        } 
        else 
        { 
            /// If no texture exists, then use 0,0 as default
            vertexStreams.texCoords[vertex_index] = {0.F, 0.F};
        }

        /// copy the normals, if model have normals
        if (mesh->HasNormals())
        {
            vertexStreams.normals[vertex_index] = 
            { 
                mesh->mNormals[vertex_index].x,
                mesh->mNormals[vertex_index].y,
                mesh->mNormals[vertex_index].z 
            };
        }
        else
        {
            vertexStreams.normals[vertex_index] = { 1.f, 1.f, 1.f };
        }

        vertex_index++;
    }

    auto meshFaces = std::span<aiFace>(mesh->mFaces, mesh->mNumFaces);
    /// Copy Indicies
    for (size_t i = 0; i < mesh->mNumFaces; i++) 
    {
        /// For every face, add it's indicies to our indicies list
        aiFace face = meshFaces[i];
        auto faceIndicies =
            std::span<unsigned int>(face.mIndices, face.mNumIndices);
        for (size_t j = 0; j < face.mNumIndices; j++) 
        {
            indices.push_back(faceIndicies[j] + initialVertex);
            index_index++;
        }
    }

    lastVertice += vertex_index;
    lastIndex += indices.size();
  
    /// Construct MeshData and return it
    MeshData meshData
    {
        .submeshes = std::vector<SubmeshData>
        {
            SubmeshData
            {
                .materialIndex = matToTex[mesh->mMaterialIndex],
                .startIndex = initialIndex,
                .numIndicies = static_cast<uint32_t>(indices.size()),
            }
        },
        .vertexStreams = vertexStreams,
        .indicies = indices,
    };

    return meshData;
}

void MeshLoader::loadAnimation(const aiScene* scene, MeshData& outMeshData)
{
    // Assuming single mesh per fbx
    aiMesh* mesh = scene->mMeshes[0];
    aiAnimation* ani = scene->mAnimations[0];
    Animation& animation = outMeshData.animations.emplace_back();

    uint32_t globalBoneIndex = 0;
	this->topologicallySortBones(mesh, scene->mRootNode, globalBoneIndex);

    animation.endTime = (float)ani->mDuration;
    animation.boneStamps.resize((size_t)mesh->mNumBones);

    for (unsigned int i = 0; i < mesh->mNumBones; i++) 
    {
        BonePoses& poses = animation.boneStamps[i];

        // The order of aiMesh::mBones does not always match aiposes::mChannels
        aiNodeAnim* nodeAnim = findAnimationNode(ani->mChannels, ani->mNumChannels, mesh->mBones[i]->mName.C_Str());
        if (!nodeAnim) 
        {
            Log::warning("Could not find animation node: \"" + std::string(mesh->mBones[i]->mName.C_Str()) + "\", defaulting to identity transform");
            
            // Create and default stamp[0] to avoid future errors
            poses.translationStamps.emplace_back(0.f, glm::vec3(0.f));
            poses.rotationStamps.emplace_back(0.f, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
            poses.scaleStamps.emplace_back(0.f, glm::vec3(1.f));

            continue;
        }

        // Get poses

        // Translation
        poses.translationStamps.resize(nodeAnim->mNumPositionKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++) 
        {
            poses.translationStamps[k].first = (float)nodeAnim->mPositionKeys[k].mTime;
            memcpy(&poses.translationStamps[k].second, &nodeAnim->mPositionKeys[k].mValue, sizeof(glm::vec3));
        }

        // Rotation
        poses.rotationStamps.resize(nodeAnim->mNumRotationKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++)
        {
            poses.rotationStamps[k].first = (float)nodeAnim->mRotationKeys[k].mTime;
            memcpy(&poses.rotationStamps[k].second, &nodeAnim->mRotationKeys[k].mValue, sizeof(glm::quat));
        }

        // Scaling
        poses.scaleStamps.resize(nodeAnim->mNumScalingKeys);
        for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; k++)
        {
            poses.scaleStamps[k].first = (float)nodeAnim->mScalingKeys[k].mTime;
            memcpy(&poses.scaleStamps[k].second, &nodeAnim->mScalingKeys[k].mValue, sizeof(glm::vec3));
        }
    }

}

void MeshLoader::loadSkeleton(const aiScene* scene, MeshData& outMeshData)
{
    aiMesh* mesh = scene->mMeshes[0];

    // Topologically sort bones for iterative tree traversal
    // when playing the animation
    uint32_t globalBoneIndex = 0;
	this->topologicallySortBones(mesh, scene->mRootNode, globalBoneIndex);

    // boneIndices used for getting parentIndex
    std::unordered_map<std::string_view, int> boneIndices;

    // Get the inverse bind pose matrix
    for (unsigned int i = 0; i < mesh->mNumBones; i++) 
    {
        boneIndices[mesh->mBones[i]->mName.C_Str()] = i;

        Bone& bone = outMeshData.bones[i];

        // Bone name
#if defined(_DEBUG) || defined(DEBUG)
        bone.boneName = mesh->mBones[i]->mName.C_Str();
#endif

        memcpy(&bone.inverseBindPoseMatrix, &mesh->mBones[i]->mOffsetMatrix, sizeof(glm::mat4));
        bone.inverseBindPoseMatrix = glm::transpose(bone.inverseBindPoseMatrix);
    
        // Set weights & boneIndex
        for (unsigned int k = 0; k < mesh->mBones[i]->mNumWeights; k++) 
        {
            aiVertexWeight& aiWeight = mesh->mBones[i]->mWeights[k];
            glm::vec4& vertexBoneWeights = outMeshData.vertexStreams.boneWeights[aiWeight.mVertexId];
            glm::uvec4& vertexBoneIndices = outMeshData.vertexStreams.boneIndices[aiWeight.mVertexId];
    
            if      (vertexBoneWeights.x <= 0.f) { vertexBoneWeights.x = aiWeight.mWeight; vertexBoneIndices.x = i; }
            else if (vertexBoneWeights.y <= 0.f) { vertexBoneWeights.y = aiWeight.mWeight; vertexBoneIndices.y = i; }
            else if (vertexBoneWeights.z <= 0.f) { vertexBoneWeights.z = aiWeight.mWeight; vertexBoneIndices.z = i; }
            else if (vertexBoneWeights.w <= 0.f) { vertexBoneWeights.w = aiWeight.mWeight; vertexBoneIndices.w = i; }
        }
    }

    // Find parent index
    for (unsigned int i = 0; i < mesh->mNumBones; i++) 
    {
        aiNode* boneNode = findNode(scene->mRootNode, mesh->mBones[i]->mName.C_Str());
        if (!boneNode)
        {
            outMeshData.bones[i].parentIndex = -1;
            continue;
        }

        aiNode* parent = findParentBoneNode(boneIndices, boneNode);
        outMeshData.bones[i].parentIndex = parent ? boneIndices[parent->mName.C_Str()] : -1;
    }

    // Normalize weights
    for (glm::vec4& boneWeights : outMeshData.vertexStreams.boneWeights)
    {
        float inverseSum = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
        if (inverseSum == 0.f) 
        {
            continue;
        }

        inverseSum = 1.f / inverseSum;

        // Apply to all components
        boneWeights *= inverseSum;
    }
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