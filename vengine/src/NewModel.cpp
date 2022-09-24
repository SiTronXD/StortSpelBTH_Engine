NewModel::NewModel(MeshData&& meshData, VulkanImportStructs& importStructs)
    : submeshData(meshData.submeshes), device(*importStructs.device),vma(*importStructs.vma)
{  


}

