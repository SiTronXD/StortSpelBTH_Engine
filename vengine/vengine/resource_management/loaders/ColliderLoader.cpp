#include "ColliderLoader.hpp"
#include "../../dev/Log.hpp"
#include <span>
#include <stack>
#include <assimp/postprocess.h>
#include <map>
#include "../../application/Scene.hpp"
 
btVector3 aiVectorToBtVector(const aiVector3D &a, btVector3 &b) 
{
	b = btVector3(a.x, a.y, a.z);
	return b;
}

int ColliderLoader::getShapeType(aiMesh* mesh, const std::string &meshName)
{
	static const std::string SphereName("Sphere");
	static const std::string PlaneName("Plane");
	static const std::string BoxName("Box");
	static const std::string ConeName("Cone");
	static const std::string CylinderName("Cylinder");
	static const std::string CapsuleName("Capsule");
	static const uint8_t SphereNameSize =	(uint8_t)SphereName.length();
	static const uint8_t PlaneNameSize =	(uint8_t)PlaneName.length();
	static const uint8_t BoxNameSize =		(uint8_t)BoxName.length();
	static const uint8_t ConeNameSize =		(uint8_t)ConeName.length();
	static const uint8_t CylinderNameSize = (uint8_t)CylinderName.length();
	static const uint8_t CapsuleNameSize =	(uint8_t)CapsuleName.length();
	//static const uint8_t PolygonNameSize = std::string("Polygon").length(); //not supported yet
	
	if (meshName.size() >= 8 + SphereNameSize && meshName.substr(8, SphereNameSize) == SphereName)
	{
		return shapeType::Sphere;
	}
	else if (meshName.size() >= 8 + PlaneNameSize && meshName.substr(8, PlaneNameSize) == PlaneName)
	{
		return shapeType::Plane;
	}
	else if (meshName.size() >= 8 + BoxNameSize && meshName.substr(8, BoxNameSize) == BoxName)
	{
		return shapeType::Box;
	}
	else if (meshName.size() >= 8 + ConeNameSize && meshName.substr(8, ConeNameSize) == ConeName)
	{
		return shapeType::Cone;
	}
	else if (meshName.size() >= 8 + CylinderNameSize && meshName.substr(8, CylinderNameSize) == CylinderName)
	{
		return shapeType::Cylinder;
	}
	else if (meshName.size() >= 8 + CapsuleNameSize && meshName.substr(8, CapsuleNameSize) == CapsuleName)
	{
		return shapeType::Capsule;
	}
	std::cout << meshName.substr(8, SphereNameSize) << std::endl;
	return shapeType::Error;
}

Collider ColliderLoader::makeCollisionShape(const shapeType& type, const aiMesh* mesh, glm::vec3 position)
{
	if (type == shapeType::Sphere)
	{
		float radius = sqrt(
			mesh->mVertices[0].x * mesh->mVertices[0].x + 
			mesh->mVertices[0].y * mesh->mVertices[0].y + 
			mesh->mVertices[0].z * mesh->mVertices[0].z
		) * 100;
		return Collider::createSphere(radius);
	}
	else if (type == shapeType::Plane)//not supported by engine
	{
		//calculate the whole fucking plane
		btVector3 a = aiVectorToBtVector((mesh->mVertices[0] - mesh->mVertices[1]), a);
		btVector3 b = aiVectorToBtVector((mesh->mVertices[0] - mesh->mVertices[2]), b);
		btVector3 normal = btCross(a, b);
		//return new btStaticPlaneShape(normal, btDot(-normal, a));
	}
	else if (type == shapeType::Box)
	{
		glm::vec3 whd(
			abs(mesh->mVertices[0].x) * 100,
			abs(mesh->mVertices[0].z) * 100, 
			abs(mesh->mVertices[0].y) * 100
		);

		return Collider::createBox(whd);
	}
	else if (type == shapeType::Cone)//not supported by engine
	{
	}
	else if (type == shapeType::Cylinder)//not supported by engine
	{
	}
	else if (type == shapeType::Capsule)
	{
		float radius = aiVector3D(mesh->mVertices[0].x, 0, mesh->mVertices[0].z).Length();
		float height = aiVector3D(0,mesh->mVertices[0].y, 0).Length() * 2;//vet ej om det ska vara 2 eller inte
		return Collider::createCapsule(radius, height);
	}
	return Collider::createBox(glm::vec3(1));
}

std::vector<std::pair<glm::vec3, Collider>> ColliderLoader::loadCollisionShape(const std::string& modelFile)
{
	//this->setComponent<Collider>(camEntity, Collider::createBox(glm::vec3(1)));
	const aiScene* scene = this->importer.ReadFile((modelFile).c_str(),
		aiProcess_JoinIdenticalVertices 
		| aiProcess_RemoveComponent 
		| aiProcess_DropNormals  
	);
	
	if (scene == nullptr)
	{
		Log::warning("Failed to load Collision (" + modelFile + ")");
		return std::vector<std::pair<glm::vec3, Collider>>();
	}

	std::vector<std::pair<glm::vec3, Collider>> collisionList;

	auto* node = scene->mRootNode;
	auto scenes_meshes = std::span<aiMesh*>(scene->mMeshes, scene->mNumMeshes);
	auto node_children = std::span<aiNode*>(node->mChildren, node->mNumChildren);
	auto node_meshes = std::span<unsigned int>(node->mMeshes, node->mNumMeshes);

	std::stack<aiNode*> nodeStack;
	nodeStack.push(node);

	while (!nodeStack.empty())
	{
		node_meshes = std::span<unsigned int>(node->mMeshes, node->mNumMeshes);
		for (size_t j = 0; j < node->mNumMeshes; j++)
		{
			std::string meshName(scenes_meshes[node_meshes[j]]->mName.C_Str());
			if (meshName.length() > 7 && meshName.substr(0, 8) == "Collider")
			{
				//get what kind of shape it is
				shapeType theShapeType = (shapeType)getShapeType(scenes_meshes[node_meshes[j]], meshName);
				if (theShapeType == shapeType::Error)
				{
					Log::error("can't load the collision box");
					return collisionList;
				}
				//get its fetures
				collisionList.push_back(std::pair<glm::vec3, Collider>(
				    glm::vec3(
						node->mTransformation.a4, 
						node->mTransformation.b4, 
						node->mTransformation.c4
					), 
					makeCollisionShape(
				        theShapeType,
				        scenes_meshes[node_meshes[j]], 
						glm::vec3(
							node->mTransformation.a4, 
							node->mTransformation.b4, 
							node->mTransformation.c4
						)
					)
				)
				);
			}
		}

		node_children = std::span<aiNode*>(node->mChildren, node->mNumChildren);

		for (auto* child : node_children)
		{
			nodeStack.push(child);
		}

		node = nodeStack.top();
		nodeStack.pop();
	}
	
	importer.FreeScene();

	return collisionList;
}

void addCollisionToScene(std::vector<std::pair<glm::vec3, Collider>> colliders, Scene& currentScene, glm::vec3 offset)
{
	for (int i = 0; i < colliders.size(); i++)
	{
		int e = currentScene.createEntity();
		Transform& t = currentScene.getComponent<Transform>(e);
		t.position = offset + colliders[i].first;
		currentScene.setComponent<Collider>(e, colliders[i].second);
	}
}
