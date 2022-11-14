#pragma once

enum Headers
{
	BUFFER_FULL,
	MESSAGE,
	MESH_NEW,
	MESH_TRANSFORM,
	MESH_REMOVE,
	MESH_MODIFIED,
	TOPOLOGY_CHANGED,
	CAMERA,
	CAMERA_UPDATE,
	MATERIAL_DATA,
	MESH_MATERIAL_CONNECTION,
};

enum CameraTypes
{
	SCENE_CAMERA_PERSPECTIVE = 1,
	SCENE_CAMERA_ORTHOGRAPHIC = 2,
};

struct MessageHeader
{
	size_t messageType;
	size_t messageLength;
};

struct MeshHeader
{
	char name[64];
	int vertexCount;
	int indexCount;
};

struct MeshTransformHeader
{
	char name[64];
	float position[3];
	float rotation[3];
	float scale[3];
	float transform[4][4];
};

struct RemoveMeshHeader
{
	char name[64];
};

struct PointMovedHeader
{
	char meshName[64];
	int nrVertices;
};

struct NumberedVertexHeader
{
	int vertexNumber;
	float position[3];
	float normal[3];
};

struct VertexHeader
{
	float position[3];
	float normal[3];
	float uv[2];
};

struct CameraHeader
{
	char name[64];
	float transform[4][4];

	CameraTypes type;
	float fieldOfView;
	float orthoWidth;
	bool active = false;

	float orthoHeight;
	float aspectRatio;
	float nearPlane;
	float farPlane;
};

struct CameraUpdateHeader
{
	char name[64];
	float transform[4][4];
	float fieldOfView;
	float orthoWidth;
	float orthoHeight;
	float aspectRatio;
};

struct MaterialHeader
{
	char materialName[64];
	float ambientColor[3];
	char diffusePath[64];
	char normalMapPath[64];
};

struct MeshMaterialConnectionHeader
{
	char meshName[64];
	char materialName[64];
};