#include "MeshDataInfo.hpp"
#include "../dev/Log.hpp"

size_t MeshDataInfo::getVertexSize(
    const AvailableVertexData& availableVertexData)
{
    /*Vertex dummyVertex{};
    uint64_t vertexSize = 0;
    vertexSize += sizeof(dummyVertex.pos) *
        (availableVertexData & (uint32_t)VertexData::POSITION);
    vertexSize += sizeof(dummyVertex.col) *
        (availableVertexData & (uint32_t)VertexData::COLOR);
    vertexSize += sizeof(dummyVertex.tex) *
        (availableVertexData & (uint32_t)VertexData::TEX_COORDS);
    vertexSize += sizeof(dummyVertex.weights) *
        (availableVertexData & (uint32_t)VertexData::BONE_WEIGHTS);
    vertexSize += sizeof(dummyVertex.bonesIndex) *
        (availableVertexData & (uint32_t)VertexData::BONE_INDICES);

    return vertexSize;*/

    Log::error("DONT CALL THIS FUNCTION");
    return 0;
}

size_t MeshDataInfo::getVertexSize(const MeshData& meshData)
{
    return MeshDataInfo::getVertexSize(meshData.availableVertexData);
}