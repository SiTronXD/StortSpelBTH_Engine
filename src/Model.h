#pragma once
#include "Mesh.h"
#include <vector>
#include "glm/glm.hpp"
#include <map>


struct aiScene;
struct aiMesh;

// Contains all parts of a model that share the same texture
    struct ModelPart{
        int textureID = 0;
        uint32_t vertexCount = 0;    
        uint32_t indexCount = 0;
        vk::Buffer vertexBuffer {};
        VmaAllocation vertexBufferMemory {};
        vk::Buffer indexBuffer{};        
        VmaAllocation indexBufferMemory{}; 
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indicies; 
    };

class Model {
public:
    Model() = default;
    explicit Model(VmaAllocator *vma, vk::PhysicalDevice newPhysicalDevice,  vk::Device newDevice,
        vk::Queue transferQueue, vk::CommandPool transferCommandPool,  std::vector<Mesh> meshList);

    Model( Model const& ref)            = default;
    Model( Model && ref) noexcept ;
    Model& operator=( Model const& ref) = default;
    Model& operator=( Model && ref) noexcept ;
    ~Model() = default;
    

    std::map<int, ModelPart>& getModelParts();

    glm::mat4 getModelMatrix();
    void setModelMatrix(glm::mat4 newModel);

    void destroryMeshModel();

    static std::vector<std::string> loadMaterials(const aiScene* scene);
    static std::vector<Mesh> getMeshesFromNodeTree(VmaAllocator *vma, VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, const aiScene * scene, const std::vector<int>& matToTex);    
    static Mesh loadMesh(VmaAllocator *vma,VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiMesh* mesh,  std::vector<int> matToTex);

private:    
    void createVertexBuffer(ModelPart& modelPart);
    void createIndexBuffer(ModelPart& modelPart);

    VmaAllocator *vma = nullptr;


    vk::PhysicalDevice physicalDevice{};
    vk::Device device{};

    vk::Queue transferQueue{};
    vk::CommandPool transferCommandPool{};

    std::vector<Mesh> meshList;
    glm::mat4 modelMatrix{1.F};
    
    std::map<int, ModelPart> modelParts;

};