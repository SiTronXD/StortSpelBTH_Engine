#include "Model.h"
#include "tracy/Tracy.hpp"
#include <utility>
#include <stack>
#include <span>
#include <set>
#include "vk_mem_alloc.h"
#include "assimp/scene.h"

Model::Model(VmaAllocator *vma,
        vk::PhysicalDevice newPhysicalDevice, 
        vk::Device newDevice,
        vk::Queue transferQueue, 
        vk::CommandPool transferCommandPool,         
        std::vector<Mesh> meshList)
:   vma(vma),
    physicalDevice(newPhysicalDevice), 
    device(newDevice),
    transferQueue(transferQueue),
    transferCommandPool(transferCommandPool), 
    meshList(std::move(meshList)), modelMatrix(glm::mat4(1.F)) /// set position to Origo as default
{     
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    

    std::vector<Vertex> all_vertices;
    std::vector<uint32_t> all_indicies;
    for(auto& mesh : this->meshList)
    {
        auto mesh_vertices = mesh.getVertex_vector();
        auto mesh_indicies = mesh.getIndicies_vector();

        if(this->modelParts.contains(mesh.getTextureId()))
        {
            auto& modelPart = this->modelParts.find(mesh.getTextureId())->second;
            modelPart.vertices.insert(modelPart.vertices.end(), mesh_vertices.begin(), mesh_vertices.end());
            for(auto i = 0; i < mesh_indicies.size(); i++){
                modelPart.indicies.emplace_back(mesh_indicies[i] + modelPart.vertexCount);
            }
            modelPart.vertexCount += mesh_vertices.size();
            modelPart.indexCount += mesh_indicies.size();
        }else
        {
            ModelPart modelPart {
                .textureID = mesh.getTextureId(),
                .vertexCount = 0,
                .indexCount = 0,
                .vertexBuffer {},
                .vertexBufferMemory {},
                .indexBuffer {},
                .indexBufferMemory {},
                .vertices {},
                .indicies {}
            };
            modelPart.vertices.insert(modelPart.vertices.end(), mesh_vertices.begin(), mesh_vertices.end());
            modelPart.indicies.insert(modelPart.indicies.end(), mesh_indicies.begin(), mesh_indicies.end());
            modelPart.vertexCount += mesh_vertices.size();
            modelPart.indexCount += mesh_indicies.size();
            this->modelParts.insert({mesh.getTextureId(),modelPart});
        }
        
    }

    for(auto& modelPart : modelParts)
    {
        createVertexBuffer(modelPart.second);
        createIndexBuffer(modelPart.second);
    }
}


void Model::createVertexBuffer(ModelPart& modelPart)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// Temporary buffer to "Stage" vertex data before transferring to GPU
    vk::Buffer stagingBuffer;
    //vk::DeviceMemory stagingBufferMemory;
    VmaAllocation stagingBufferMemory{};

    VmaAllocationInfo allocInfo_staging;

    /// Get size of buffers needed for Vertices
    vk::DeviceSize bufferSize  =sizeof(Vertex) * modelPart.vertices.size();

    vengine_helper::createBuffer(
        {
            .physicalDevice = physicalDevice, 
            .device         = device, 
            .bufferSize     = bufferSize,  
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       /// This buffers vertex data will be transfered somewhere else!
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer         = &stagingBuffer,
            .bufferMemory   = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = vma
        });    
    

    /// -- Map memory to our Temporary Staging Vertex Buffer -- 
    void * data{};
    if(vmaMapMemory(*this->vma, stagingBufferMemory, &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Mesh Staging Vertex Buffer Using VMA!");
    };

    memcpy(data, modelPart.vertices.data(),sizeof(Vertex) * modelPart.vertices.size());
    vmaUnmapMemory(*this->vma, stagingBufferMemory); 


    VmaAllocationInfo allocInfo_deviceOnly;
    /// Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    /// Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
    vengine_helper::createBuffer(
        {
            .physicalDevice = physicalDevice, 
            .device         = device, 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst            /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    //// This is a Vertex Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .buffer         = &modelPart.vertexBuffer, 
            .bufferMemory   = &modelPart.vertexBufferMemory,
            .allocationInfo = &allocInfo_deviceOnly,
            .vma = this->vma

        });

    /// Copy Staging Buffer to Vertex Buffer on GPU
    vengine_helper::copyBuffer(
        this->device, 
        transferQueue, 
        transferCommandPool, 
        stagingBuffer, 
        modelPart.vertexBuffer, 
        bufferSize);

    /// Clean up Staging Buffer stuff
    this->device.destroyBuffer(stagingBuffer);
    vmaFreeMemory(*this->vma, stagingBufferMemory);
}

void Model::createIndexBuffer(ModelPart& modelPart)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Get size of buffer needed for indices
    vk::DeviceSize bufferSize = sizeof(uint32_t) * modelPart.indicies.size();

    /// Temporary buffer to "Stage" index data before transferring to GPU
    vk::Buffer stagingBuffer{};
    VmaAllocation stagingBufferMemory{};
    VmaAllocationInfo allocInfo_staging;

    vengine_helper::createBuffer(
        {
            .physicalDevice = this->physicalDevice, 
            .device         = this->device, 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc, 
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer         = &stagingBuffer, 
            .bufferMemory   = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = this->vma
        });

    /// Map Memory to Index Buffer! 
    void* data{}; 
    vmaMapMemory(*this->vma, stagingBufferMemory, &data);
    memcpy(data, modelPart.indicies.data(), sizeof(uint32_t) * modelPart.indicies.size());
    vmaUnmapMemory(*this->vma, stagingBufferMemory);
    
    VmaAllocationInfo allocInfo_device;

    /// Create Buffers for INDEX data on GPU access only area
    vengine_helper::createBuffer(
        {
            .physicalDevice = this->physicalDevice, 
            .device         = this->device, 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eIndexBuffer,     /// This is a Index Buffer, will be used as a Index Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,    /// Buffer will be local to the device
            .buffer         = &modelPart.indexBuffer, 
            .bufferMemory   = &modelPart.indexBufferMemory,
            .allocationInfo = &allocInfo_device,
            .vma = this->vma
        });

    vengine_helper::copyBuffer(
        this->device, 
        transferQueue, 
        this->transferCommandPool, 
        stagingBuffer, 
        modelPart.indexBuffer, 
        bufferSize);

    /// Destroy + Release Staging Buffer resources
    this->device.destroyBuffer(stagingBuffer);
    vmaFreeMemory(*this->vma,stagingBufferMemory);

}

Model::Model(Model && ref) noexcept 
:   vma(ref.vma),
    physicalDevice(ref.physicalDevice),device(ref.device),transferQueue(ref.transferQueue),
    transferCommandPool(ref.transferCommandPool),
    meshList(std::move(ref.meshList)), modelMatrix(ref.modelMatrix)
{    
    this->modelParts.insert(ref.modelParts.begin(), ref.modelParts.end());
}

Model& Model::operator=(Model && ref) noexcept 
{
    this->meshList = std::move(ref.meshList);
    this->modelMatrix = ref.modelMatrix;
    return *this;
}

std::map<int, ModelPart>& Model::getModelParts()
{
    return this->modelParts;
}



glm::mat4 Model::getModelMatrix()
{ 
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return modelMatrix;
}

void Model::setModelMatrix(glm::mat4 newModel)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    this->modelMatrix = newModel;
}

void Model::destroryMeshModel()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    for(Mesh& mesh : this->meshList)
    {
        mesh.destroyBuffers();
    }

    for(auto modelPart : this->modelParts)
    {
        this->device.destroyBuffer(modelPart.second.vertexBuffer);    
        vmaFreeMemory(*this->vma,modelPart.second.vertexBufferMemory);
        this->device.destroyBuffer(modelPart.second.indexBuffer);
        vmaFreeMemory(*this->vma, modelPart.second.indexBufferMemory);
    }

}

std::vector<std::string> Model::loadMaterials(const aiScene* scene)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    std::vector<std::string> textureList(scene->mNumMaterials);    
    int textureIndex = 0; /// index of the texture that corresponds to the Diffuse material

    /// For each material, if texture file name exists, store it in vector
    for(auto* material : std::span<aiMaterial*>(scene->mMaterials,scene->mNumMaterials)){
        
        /// Check for a Diffure Texture
        if(material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
        {
            /// get the Path of the texture file
            aiString path; 
            if(material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS )
            {                            
                textureList[textureIndex] = std::string(path.C_Str());
            }
        }
        textureIndex++; /// Which Material the 
    }
    
    return textureList;
}

std::vector<Mesh> Model::getMeshesFromNodeTree(VmaAllocator* vma, VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, const aiScene * scene, const std::vector<int>& matToTex)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    std::vector<Mesh> meshList;
    auto* node = scene->mRootNode;
    auto scenes_meshes  = std::span<aiMesh*>(scene->mMeshes,scene->mNumMeshes);    
    auto node_children  = std::span<aiNode*>(node->mChildren, node->mNumChildren);
    auto node_meshes    = std::span<unsigned int>(node->mMeshes,node->mNumMeshes);

    std::stack<aiNode*> nodeStack;
    nodeStack.push(node);

    while(!nodeStack.empty())
    {

        node_meshes    = std::span<unsigned int>(node->mMeshes,node->mNumMeshes);
        for(size_t j = 0; j < node->mNumMeshes; j++ )
        {
            meshList.push_back(loadMesh(vma, newPhysicalDevice, newDevice, transferQueue, transferCommandPool, scenes_meshes[node_meshes[j]], matToTex));
        }

        node_children  = std::span<aiNode*>(node->mChildren, node->mNumChildren);
        for(auto* child : node_children){
            nodeStack.push(child);
        }

        node = nodeStack.top();
        nodeStack.pop();                

    }

    return meshList;
}

Mesh Model::loadMesh(VmaAllocator* vma, VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiMesh* mesh, std::vector<int> matToTex)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    std::vector<Vertex>     vertices;
    std::vector<uint32_t>   indices;

    const unsigned int sizeOTextureCoordsElement = 8;
    auto textureCordinates = std::span<aiVector3D*>(static_cast<aiVector3D**>(mesh->mTextureCoords),sizeOTextureCoordsElement);

    /// Resize vertex list to hold all vertices for mesh
    vertices.resize(mesh->mNumVertices);
    int vertex_index = 0;

    /// For each veretx in our assimp mesh, define the Vertex in our format
    for(auto ai_vertice : std::span<aiVector3D>(mesh->mVertices, mesh->mNumVertices))
    {
        /// copy the position vertex        
        vertices[vertex_index].pos = {ai_vertice.x, ai_vertice.y, ai_vertice.z};
        /// copy the tex coordinate, if model have textures
        if(mesh->mTextureCoords[0] != nullptr) 
        {            
            vertices[vertex_index].tex = {textureCordinates[0][vertex_index].x, textureCordinates[0][vertex_index].y};
        }
        else
        {   /// If no texture exists, then use 0,0 as default
            vertices[vertex_index].tex = {0.F, 0.F};
        }

        /// set Color, if we want to use it later...
        vertices[vertex_index].col = {1.F,1.F,1.F}; /// Color is white
        vertex_index++;
    }
    
    auto meshFaces = std::span<aiFace>(mesh->mFaces, mesh->mNumFaces);
    /// Copy Indicies 
    for(size_t i = 0; i < mesh->mNumFaces; i++)
    {
        /// For every face, add it's indicies to our indicies list
        aiFace face = meshFaces[i];
        auto faceIndicies = std::span<unsigned int>(face.mIndices, face.mNumIndices);
        for(size_t j = 0; j < face.mNumIndices; j++)
        {            
            //indices.push_back(face.mIndices[j]);
            indices.push_back(faceIndicies[j]);
        }
    }
    /// Create new mesh with details and return it
    Mesh newMesh = Mesh(vma, newPhysicalDevice, newDevice, transferQueue, transferCommandPool, &vertices, &indices, matToTex[mesh->mMaterialIndex]);

    return newMesh;
}

