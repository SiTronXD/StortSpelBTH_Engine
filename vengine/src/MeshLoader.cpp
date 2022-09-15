#include "MeshLoader.hpp"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Configurator.hpp"


Assimp::Importer     MeshLoader::importer;
VmaAllocator*        MeshLoader::vma                 = nullptr; 
vk::PhysicalDevice*  MeshLoader::physicalDevice      = nullptr; 
vk::Device*          MeshLoader::device              = nullptr; 
vk::Queue*           MeshLoader::transferQueue       = nullptr; 
vk::CommandPool*     MeshLoader::transferCommandPool = nullptr; 

Model&& MeshLoader::createMesh(std::string modelFile)
{
    // Import Model Scene
    using namespace vengine_helper::config;
    const aiScene* scene = MeshLoader::importer.ReadFile(
        (DEF<std::string>(P_MODELS)+modelFile).c_str(),
        aiProcess_Triangulate               // Ensures that ALL objects will be represented as Triangles
        | aiProcess_FlipUVs                 // Flips the texture UV values, to be same as how we use them
        | aiProcess_JoinIdenticalVertices   // Saves memory by making sure no dublicate vertices exists
        );

    if(scene == nullptr)
    {
        throw std::runtime_error("Failed to load model ("+modelFile+")");
    }

    // Get vector of all materials 
    std::vector<std::string> textureNames = Model::loadMaterials(scene);

    // Handle empty texture 
    std::vector<int> matToTexture(textureNames.size());

    for(size_t i = 0; i < textureNames.size(); i++){
        
        if(textureNames[i].empty())
        {
            matToTexture[i] = 0; // Use default textures for models if textures are missing
        }
        else
        {
            // Create texture, use the index returned by our createTexture function
            
            
            //TODO:  the following line must be replaced...
            
            //matToTexture[i] = createTexture(textureNames[i]); 
            
            
        }
    }

    // Load in all meshes
    std::vector<Mesh> modelMeshes = Model::getMeshesFromNodeTree(
        MeshLoader::vma,
        *MeshLoader::physicalDevice, 
        *MeshLoader::device, 
        *MeshLoader::transferQueue,
        *MeshLoader::transferCommandPool, 
        scene, 
        matToTexture
    );

    // Create Model, add to list
    Model model = Model(
        MeshLoader::vma,
        *MeshLoader::physicalDevice, 
        *MeshLoader::device, 
        *MeshLoader::transferQueue,
        *MeshLoader::transferCommandPool, 
        modelMeshes
    );
    //modelList.emplace_back(model);

    return std::move(model);
}
