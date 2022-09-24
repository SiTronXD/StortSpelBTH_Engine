#include "MeshLoader.hpp"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Configurator.hpp"



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
    MeshData meshData; // TODO: remove this temp...
    //MeshData meshData = MeshLoader::assimpMeshImport(scene, materailToTexture);
    //MeshLoader::importer.FreeScene();
    return meshData;
}
