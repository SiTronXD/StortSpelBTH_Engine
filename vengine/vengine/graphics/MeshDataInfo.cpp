#include "MeshDataInfo.hpp"
#include "../dev/Log.hpp"

size_t MeshDataInfo::getVertexSize(const MeshData& meshData)
{
    size_t vertexSize = 0;
    if (meshData.vertexStreams.positions.size() > 0) vertexSize += sizeof(meshData.vertexStreams.positions[0]);
    if (meshData.vertexStreams.colors.size() > 0) vertexSize += sizeof(meshData.vertexStreams.colors[0]);
    if (meshData.vertexStreams.texCoords.size() > 0) vertexSize += sizeof(meshData.vertexStreams.texCoords[0]);
    if (meshData.vertexStreams.boneWeights.size() > 0) vertexSize += sizeof(meshData.vertexStreams.boneWeights[0]);
    if (meshData.vertexStreams.boneIndices.size() > 0) vertexSize += sizeof(meshData.vertexStreams.boneIndices[0]);

    return vertexSize;
}