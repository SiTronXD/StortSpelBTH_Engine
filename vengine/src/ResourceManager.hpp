#pragma once 

#include <cstdint>
#include <unordered_map>
#include <string>

class Model;

class ResourceManager{
private:
    ResourceManager(); // Private Constructor, do not allow ResourceManager to be created
    static std::unordered_map<std::string, uint32_t> mesh_paths;
    static std::unordered_map<uint32_t, Model> meshes;

public:

    static uint32_t addMesh(std::string&& mesh_path);
};

/*
- (meshloader) Komponent för mesh: interfaces till assimp
- - inkludera assimp i translation units ...
- - Mesh data klass => agnostisk från vulkan, m.m.
- - MeshLoader Gör ett interface för Assimp 
- (imageLoader) komponent för images: interface till .. stb_image?
- - imageLoader gör ett interface för stb_image
- Log status of resource; was it already loaded? Collision? 
*/