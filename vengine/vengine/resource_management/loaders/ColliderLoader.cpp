#include "ColliderLoader.hpp"
#include "../../dev/Log.hpp"
#include <span>
#include <stack>
#include <assimp/postprocess.h>
#include <map>


int ColliderLoader::getShapeType(aiMesh* mesh)
{
	switch (mesh->mNumVertices)
	{
		case 2:
			//Two points that are the width of the sphere and the middle of those points are the middle
			return shapeType::Sphere;
			break;
		case 4:
			//four points that make the plane
			return shapeType::Plane;
			break;
		case 8:
			//Normal box shape
			return shapeType::Box;
			break;
		case 3:
			//first two points are the circle the other two are where they are going to
			return shapeType::Cone;
			break;
		case 6:
			//two circles and then the height
			return shapeType::Cylinder;
			break;
		default:
			Log::warning("Didn't load the collision in the right way");
			return shapeType::Error;
			break;
	}
	return 0;
}

btCollisionShape* ColliderLoader::makeCollisionShape(const shapeType& type, const aiMesh* mesh)
{
	if (type == shapeType::Sphere)
	{
		float radius = (mesh->mVertices[0] - mesh->mVertices[1]).Length()/2;
		return new btSphereShape(radius);
	}
	else if (type == shapeType::Box)
	{
		btVector3 whd(
			(mesh->mVertices[0] - mesh->mVertices[1]).Length(), 
			(mesh->mVertices[0] - mesh->mVertices[2]).Length(),
		    (mesh->mVertices[0] - mesh->mVertices[4]).Length()
		);
		return new btBoxShape(whd);
	}
	else if (type == shapeType::Plane)
	{
		//calculate the whole fucking plane
	}
	return new btBoxShape(btVector3(1,1,1));
}

std::vector<std::vector<aiVector3D>> joinIdenticalVertices(std::vector<aiMesh*> meshes)
{
	std::vector<std::vector<aiVector3D>> theReturn;
	for (int i = 0; i < meshes.size(); i++)
	{
		std::vector<aiVector3D> vertecies;
		for (int v = 0; v < meshes[i]->mNumVertices; v++)
		{
			
			meshes[i]->mVertices
		}
	}
}

std::vector<btCollisionShape*> ColliderLoader::loadCollisionShape(const std::string& modelFile)
{
	const aiScene* scene = this->importer.ReadFile((modelFile).c_str(),
		aiProcess_JoinIdenticalVertices |
		aiProcess_RemoveComponent | 
		aiProcess_GlobalScale | 
		aiProcess_Triangulate
	);
	
	if (scene == nullptr)
	{
		Log::warning("Failed to load Collision (" + modelFile + ")");
		return std::vector<btCollisionShape*>();
	}

	std::vector<btCollisionShape*> collisionList;
	std::vector<aiMesh*> scenes_meshes;
	scenes_meshes.reserve(scene->mNumMeshes);
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		scenes_meshes.push_back(scene->mMeshes[i]);
	}
	//double check if aiProcess_JoinIdenticalVertices did work
	void joinIdenticalVertices();


	{
		node_meshes = std::span<unsigned int>(node->mMeshes, node->mNumMeshes);
		for (size_t j = 0; j < node->mNumMeshes; j++)
		{
			std::string meshName(scenes_meshes[node_meshes[j]]->mName.C_Str());
			if (meshName.length() > 7 && meshName.substr(scenes_meshes[node_meshes[j]]->mName.length - 7) == "collide")
			{
				//get what kind of shape it is
				shapeType theShapeType = (shapeType)getShapeType(scenes_meshes[node_meshes[j]]);
				if (theShapeType == shapeType::Error)
				{
					Log::error("can't load the collision box");
					for (int i = 0; i < scenes_meshes[node_meshes[j]]->mNumVertices; i++)
					{
						std::cout << scenes_meshes[node_meshes[j]]->mVertices[i].x << ", " << scenes_meshes[node_meshes[j]]->mVertices[i].y << ", "
						          << scenes_meshes[node_meshes[j]]->mVertices[i].z << std::endl;
					}
					//return collisionList;
				}
				//get its fetures
				collisionList.push_back(makeCollisionShape(theShapeType, scenes_meshes[node_meshes[j]]));
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
