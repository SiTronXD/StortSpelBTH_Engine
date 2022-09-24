#pragma once 

#include <cstdint>
#include <unordered_map>
#include <string>

//class Model;
class NewModel;
class Engine;
struct ImageData;   //Defined in MeshLoader

class ResourceManager{
private:
    /// Engine should take care of cleanups
    friend class Engine; 
    friend class VulkanRenderer; 
    ResourceManager(); // Private Constructor, do not allow ResourceManager to be created
    static std::unordered_map<std::string, uint32_t> meshPaths;
    static std::unordered_map<std::string, uint32_t> texturePaths;
    
    static std::unordered_map<uint32_t, NewModel>  meshes;
    static std::unordered_map<uint32_t, ImageData> textures;

    static void cleanup();
public:

    static uint32_t addMesh(std::string&& meshPath);
    static uint32_t addTexture(std::string&& texturePath);

    static NewModel&     getMesh(uint32_t id);      //TODO: inline when not static
    static ImageData&    getTexture(uint32_t id);   //TODO: inline when not static
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