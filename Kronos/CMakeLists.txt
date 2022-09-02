CMAKE_MINIMUM_REQUIRED(VERSION 3.23)
project(kronos LANGUAGES CXX )


file(GLOB ENGINE_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_SOURCE_DIR}/Engine/*.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Application/*.cpp
    ${CMAKE_SOURCE_DIR}/Engine/DataStructures/*.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Dev/*.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Graphics/*.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Graphics/Shaders/*.cpp
    ${CMAKE_SOURCE_DIR}/Engine/Graphics/Vulkan/*.cpp    
)
file(GLOB ENGINE_HEADERS CONFIGURE_DEPENDS 
    ${CMAKE_SOURCE_DIR}/Engine/*.h
    ${CMAKE_SOURCE_DIR}/Engine/Application/*.h
    ${CMAKE_SOURCE_DIR}/Engine/DataStructures/*.h
    ${CMAKE_SOURCE_DIR}/Engine/Dev/*.h
    ${CMAKE_SOURCE_DIR}/Engine/Graphics/*.h
    ${CMAKE_SOURCE_DIR}/Engine/Graphics/Shaders/*.h
    ${CMAKE_SOURCE_DIR}/Engine/Graphics/Vulkan/*.h
)

add_executable(
    engine     
)

target_sources(engine PRIVATE Main.cpp ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_compile_features(engine PUBLIC cxx_std_20)
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)



target_include_directories(engine PUBLIC ${Vulkan_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/Linking/Include)
target_link_libraries(engine PUBLIC ${Vulkan_LIBRARIES} glfw )