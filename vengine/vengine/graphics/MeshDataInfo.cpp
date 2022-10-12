#include "MeshDataInfo.hpp"
#include "../dev/Log.hpp"

bool MeshDataInfo::areStreamsValid(const VertexStreams& vertexStreams)
{
    size_t maxVerts = 0;
    maxVerts = std::max(maxVerts, vertexStreams.positions.size());
    maxVerts = std::max(maxVerts, vertexStreams.colors.size());
    maxVerts = std::max(maxVerts, vertexStreams.texCoords.size());
    maxVerts = std::max(maxVerts, vertexStreams.boneWeights.size());
    maxVerts = std::max(maxVerts, vertexStreams.boneIndices.size());

    bool valid = true;
    if (vertexStreams.positions.size() > 0 && vertexStreams.positions.size() != maxVerts) valid = false;
    if (vertexStreams.colors.size() > 0 && vertexStreams.colors.size() != maxVerts) valid = false;
    if (vertexStreams.texCoords.size() > 0 && vertexStreams.texCoords.size() != maxVerts) valid = false;
    if (vertexStreams.boneWeights.size() > 0 && vertexStreams.boneWeights.size() != maxVerts) valid = false;
    if (vertexStreams.boneIndices.size() > 0 && vertexStreams.boneIndices.size() != maxVerts) valid = false;

    return valid;
}

size_t MeshDataInfo::getVertexSize(const VertexStreams& vertexStreams)
{
    size_t vertexSize = 0;
    if (vertexStreams.positions.size() > 0) vertexSize += sizeof(vertexStreams.positions[0]);
    if (vertexStreams.colors.size() > 0) vertexSize += sizeof(vertexStreams.colors[0]);
    if (vertexStreams.texCoords.size() > 0) vertexSize += sizeof(vertexStreams.texCoords[0]);
    if (vertexStreams.boneWeights.size() > 0) vertexSize += sizeof(vertexStreams.boneWeights[0]);
    if (vertexStreams.boneIndices.size() > 0) vertexSize += sizeof(vertexStreams.boneIndices[0]);

    return vertexSize;
}