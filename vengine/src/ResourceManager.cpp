#include "ResourceManager.hpp"
#include "Model.hpp"
#include "Log.hpp"
#include "MeshLoader.hpp"

std::unordered_map<std::string, uint32_t>   ResourceManager::mesh_paths;
std::unordered_map<uint32_t, Model>         ResourceManager::meshes;

uint32_t ResourceManager::addMesh(std::string&& mesh_path)
{
    uint32_t prevSize = ResourceManager::mesh_paths.size();
    ResourceManager::mesh_paths.insert({mesh_path,prevSize}); //NOTE: prevSize as key only works if we never remove resources the map...

    // If exists, return key of actual mesh
    if(ResourceManager::mesh_paths.size() == prevSize)
    {
        Log::warning("Mesh [TODO: insert name of mesh file] was already added!"); //TODO: should be able to log what mesh
        
        return ResourceManager::meshes.find(
                ResourceManager::mesh_paths.find(mesh_path)->second
        )->first;
    }

    // Create mesh, insert into map of meshes

    Model createdMesh = MeshLoader::createMesh(mesh_path);

    meshes.insert({meshes.size(), std::move(createdMesh)}); //NOTE: meshes.size as key only works if we never remove resources the map...
    

    return meshes.size() -1;
}
