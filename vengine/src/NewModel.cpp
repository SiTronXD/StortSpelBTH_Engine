#include "NewModel.hpp"
NewModel::NewModel(MeshData&& meshData, VulkanImportStructs& importStructs)
    : submeshData(meshData.submeshes), device(*importStructs.device),vma(*importStructs.vma)
{  


}



NewModel::NewModel(NewModel&& ref)
:   submeshData(std::move(ref.submeshData)),
    device(ref.device),
    vma(ref.vma),
    vertexBuffer(std::move(ref.vertexBuffer)),
    indexBuffer(std::move(ref.indexBuffer)),
    vertexBufferMemory(std::move(ref.vertexBufferMemory)),
    indexBufferMemory(std::move(ref.indexBufferMemory))
{
}
