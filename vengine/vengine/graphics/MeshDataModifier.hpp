#pragma once 
 #include "op_overload.hpp"

#include "MeshData.hpp"

class MeshDataModifier
{
private:
public:
	static void smoothNormals(MeshData& meshData);
};