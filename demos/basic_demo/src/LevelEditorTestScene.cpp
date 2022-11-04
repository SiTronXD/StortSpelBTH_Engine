#include "LevelEditorTestScene.h"

LevelEditorTestScene::LevelEditorTestScene()
	:camEntity(-1)
{
}

LevelEditorTestScene::~LevelEditorTestScene()
{
	if (comLib)
		delete comLib;
	if (msg)
		delete msg;
	if (header)
		delete header;
}

void LevelEditorTestScene::init()
{
	comLib = new Comlib(L"MayaBuffer", 150 * (1 << 20), Consumer);

	// Camera
	this->camEntity = this->createEntity();
	this->setComponent<Camera>(this->camEntity);
	this->setMainCamera(this->camEntity);

	//reference object
	this->testEntity = this->createEntity();
	this->setComponent<MeshComponent>(this->testEntity, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->getComponent<Transform>(this->testEntity).scale = glm::vec3(0.1);
	this->getComponent<Transform>(this->testEntity).position = glm::vec3(10);
}

void LevelEditorTestScene::update()
{
	while (comLib->Recieve(msg, header))
	{
		if (header->messageType == NEW_MESH)
		{
			MeshHeader mesh;
			int offset = 0;

			memcpy(&mesh, msg + offset, sizeof(MeshHeader));
			offset += sizeof(MeshHeader);

			VertexHeader* vertices = new VertexHeader[mesh.vertexCount];
			memcpy(vertices, msg + offset, sizeof(VertexHeader) * mesh.vertexCount);
			offset += sizeof(VertexHeader) * mesh.vertexCount;

			int* indices = new int[mesh.indexCount];
			memcpy(indices, msg + offset, sizeof(int) * mesh.indexCount);

			MeshData meshData;
			for (int i = 0; i < mesh.vertexCount; i++)
			{
				meshData.vertexStreams.positions.push_back(glm::vec3(vertices[i].position[0], vertices[i].position[1], vertices[i].position[3]));
				meshData.vertexStreams.normals.push_back(glm::vec3(vertices[i].normal[0], vertices[i].normal[1], vertices[i].normal[3]));
				meshData.vertexStreams.texCoords.push_back(glm::vec2(vertices[i].uv[0], vertices[i].uv[1]));
			}
			for (int i = 0; i < mesh.indexCount; i++)
				meshData.indicies.push_back(static_cast<uint32_t>(indices[i]));

			meshData.submeshes.push_back(SubmeshData{
			.materialIndex = 1,
			.startIndex = 0,
			.numIndicies = static_cast<uint32_t>(mesh.indexCount),
				});

			this->mayaObjects.insert({ mesh.name, this->createEntity() });
			this->setComponent<MeshComponent>(this->mayaObjects[mesh.name], (int)this->getResourceManager()->addMesh(mesh.name, meshData));
		}
	}

	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), Input::isKeyDown(Keys::Q) - Input::isKeyDown(Keys::E), Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += moveVec * 25.0f * Time::getDT();
	}

}
