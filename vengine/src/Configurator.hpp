#pragma once
#include <iostream>
#include <map>
#include <any>


// #define def_rule(name, type) vengine_helper::config::DEF<type>(name)
// #define CAMERA_FOV def_rule("camera_fov",float)

constexpr std::string_view CAM_FOV      = "camera_fov" ;
constexpr std::string_view CAM_NP       = "camera_nearplane" ;
constexpr std::string_view CAM_FP       = "camera_farplane" ;
constexpr std::string_view CAM_EYE_X    = "camera_eye_x" ;
constexpr std::string_view CAM_EYE_Y    = "camera_eye_y" ;
constexpr std::string_view CAM_EYE_Z    = "camera_eye_z" ;
constexpr std::string_view CAM_TARGET_X = "camera_target_x" ;
constexpr std::string_view CAM_TARGET_Y = "camera_target_y" ;
constexpr std::string_view CAM_TARGET_Z = "camera_target_z" ;
constexpr std::string_view W_WIDTH      = "window_width" ;
constexpr std::string_view W_HEIGHT     = "window_height" ;
constexpr std::string_view P_ASSETS     = "path_assets" ;
constexpr std::string_view P_MODELS     = "path_models" ;
constexpr std::string_view P_TEXTURES   = "path_textures" ;
constexpr std::string_view P_SHADERS    = "path_shaders" ;
constexpr std::string_view SAMPL_MAX_ANISOSTROPY    = "max_anisotropy" ;
constexpr std::string_view USE_BUILTIN_VALIDATION   = "use_builtin_validation_layers" ;


namespace vengine_helper::config::defaults
{       
    const float CAMERA_FOV  = 45.F;    
    const float CAMERA_NP   = 0.01F;
    const float CAMERA_FP   = 1000000.F ;
    const float CAM_EYE_X   = 0.F ;
    const float CAM_EYE_Y   = 0.F ;
    const float CAM_EYE_Z   = 0.F ;
    const float CAM_TARGET_X= 0.F ;
    const float CAM_TARGET_Y= 10.F;
    const float CAM_TARGET_Z= 0.F ;
    const int W_WIDTH  = 800;    
    const int W_HEIGHT = 600;    
    const std::string P_ASSETS      = "assets/" ;           //:NOLINT: Nothing but const std::string works as intended...
    const std::string P_MODELS      = "assets/models/";     //:NOLINT:
    const std::string P_TEXTURES    = "assets/textures/";   //:NOLINT:
    const std::string P_SHADERS     = "assets/shaders/";    //:NOLINT:
    const float SAMPL_MAX_ANISOSTROPY = 16.F ;
    const bool USE_BUILTIN_VALIDATION = false ;

}

namespace vengine_helper::config
{   
    enum class Type{
        int_t,
        float_t,
        douible_t,
        string_t,
        bool_t
    };

    struct ConfVal{
        //std::string name;
        std::any value;
        Type type;
    };
    // Contains Default values if config.cfg is not generated yet
    struct configHolder{    
        std::map<std::string_view,ConfVal> rules
        {
            {CAM_FOV,{defaults::CAMERA_FOV, Type::float_t }},
            {CAM_NP,{defaults::CAMERA_NP, Type::float_t }},
            {CAM_FP,{defaults::CAMERA_FP, Type::float_t }},
            {CAM_EYE_X,{defaults::CAM_EYE_X, Type::float_t }},
            {CAM_EYE_Y,{defaults::CAM_EYE_Y, Type::float_t }},
            {CAM_EYE_Z,{defaults::CAM_EYE_Z, Type::float_t }},
            {CAM_TARGET_X,{defaults::CAM_TARGET_X, Type::float_t }},
            {CAM_TARGET_Y,{defaults::CAM_TARGET_Y, Type::float_t }},
            {CAM_TARGET_Z,{defaults::CAM_TARGET_Z, Type::float_t }},            
            {W_WIDTH, {defaults::W_WIDTH,  Type::int_t }},
            {W_HEIGHT,{defaults::W_HEIGHT, Type::int_t }},
            {P_ASSETS,{(defaults::P_ASSETS), Type::string_t }},
            {P_MODELS,{(defaults::P_MODELS), Type::string_t }},
            {P_TEXTURES,{(defaults::P_TEXTURES), Type::string_t }},
            {P_SHADERS,{(defaults::P_SHADERS), Type::string_t }},
            {SAMPL_MAX_ANISOSTROPY,{(defaults::SAMPL_MAX_ANISOSTROPY), Type::float_t }},
            {USE_BUILTIN_VALIDATION,{(defaults::USE_BUILTIN_VALIDATION), Type::bool_t }},
        };
    };
    extern configHolder conf;
}


namespace vengine_helper::config
{    
    void loadConfIntoMemory();
    float camera_fov();

    // template<typename T>
    // T DEF(std::string&& name)
    // {
    //     std::cout << std::any_cast<T>(conf.rules.find(name)->second.value) << std::endl;
    //     return std::any_cast<T>(conf.rules.find(name)->second.value);
    // }
        
    template<typename T >
    T DEF(std::string_view name)
    {
        //std::cout << std::any_cast<T>(conf.rules.find(name)->second.value) << std::endl;
        return std::any_cast<T>(conf.rules.find(name)->second.value);
    }

    template<typename T>
    T get_rule_val(std::string&& ruleName);


}

