#pragma once

#include "Utilities.h"



/// Model used to Define the Model Matrix of a mesh...
//// Previously it was a UniformBuffer (we called it UboModel)
//// Right now it's a Push Constant, but we will just call it Model from now on...
struct ModelMatrix { 
    glm::mat4 model; 
};

class Mesh 
{
public:
    //~Mesh();
    Mesh() = default;
    Mesh(VmaAllocator *vma,
        vk::PhysicalDevice newPhysicalDevice, vk::Device newDevice,
        vk::Queue transferQueue, vk::CommandPool transferCommandPool, 
        std::vector<Vertex> * vertices, 
        std::vector<uint32_t> * indicies,
        int newTextureID);

    void setModelMatrix(glm::mat4 newModel);
    ModelMatrix* getModelMatrix();

    [[nodiscard]] 
    int getTextureId() const;

    [[nodiscard]] 
    uint32_t getVertexCount() const;
    vk::Buffer getVertexBuffer();

    [[nodiscard]] 
    std::vector<Vertex>& getVertex_vector();
    std::vector<uint32_t>& getIndicies_vector();

    [[nodiscard]] 
    uint32_t getIndexCount() const;
    vk::Buffer getIndexBuffer();

    void destroyBuffers();

private: 
    ModelMatrix model{}; 

    int texId = -1; //index of the Texture
    VmaAllocator *vma;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indicies;

    uint32_t vertexCount = 0;    
    vk::Buffer vertexBuffer{};
    //vk::DeviceMemory vertexBufferMemory{};
    VmaAllocation vertexBufferMemory{};


    uint32_t indexCount = 0;
    vk::Buffer indexBuffer{};
    //vk::DeviceMemory indexBufferMemory{};
    VmaAllocation indexBufferMemory{};

    vk::PhysicalDevice physicalDevice{};
    vk::Device device{};

    vk::Queue transferQueue{};
    vk::CommandPool transferCommandPool{};

    void createVertexBuffer(std::vector<Vertex>* vertices);    
    void createIndexBuffer(std::vector<uint32_t>* indicies);
};