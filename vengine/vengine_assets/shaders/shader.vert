#version 450        //Use GLSL 4.5

//Location 0, has binding = 0! 
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 tex; 

layout(set = 0, binding = 0) uniform UboViewProjection{
    mat4 projection;
    mat4 view;    
} uboViewProjection;


/// NOT IN USE; Left for Reference... This was bound to our Dynamic Uniform Buffer, 
//// in roder to use Different binding for that. However we now use Push constants for this!
layout(set = 0, binding = 1) uniform UboModel{
    mat4 model;
} uboModel;

/// Push Constant to update the model Matrix! 
layout(push_constant)  uniform PushConstant_Model{
    mat4 model;
} pushConstant_Model;

layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragTex;

void main(){
    gl_Position = uboViewProjection.projection * uboViewProjection.view * pushConstant_Model.model  * vec4(pos, 1.0);    

    fragCol = col;
    fragTex = tex;
}