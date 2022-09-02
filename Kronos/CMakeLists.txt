CMAKE_MINIMUM_REQUIRED(VERSION 3.23)
project(kronos LANGUAGES CXX )

option(KRONOS_BUILD_EXEC "Build exec to test kronos_engine" OFF) 

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


add_library(
    engine  SHARED   
)

#target_sources(engine PRIVATE Main.cpp ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_sources(engine PRIVATE ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_compile_features(engine PUBLIC cxx_std_20)
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)



target_include_directories(engine PUBLIC ${Vulkan_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/Linking/Include)
target_link_libraries(engine PUBLIC ${Vulkan_LIBRARIES} glfw )

# add_custom_command(TARGET engine POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:engine> $<TARGET_FILE_DIR:engine>
#         COMMENT "Copying dll-files to location of executable..."
#         COMMAND_EXPAND_LISTS
#         )

# If we will build the executable for devs to test...
if(${KRONOS_BUILD_EXEC})
    add_executable(
        engine_demo     
    )
    target_sources(engine_demo PRIVATE ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    target_compile_features(engine_demo PUBLIC cxx_std_20)
    target_include_directories(engine_demo PUBLIC ${Vulkan_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/Linking/Include)
    target_link_libraries(engine_demo PUBLIC ${Vulkan_LIBRARIES} glfw )
endif()


add_custom_command(TARGET engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:engine> ${CMAKE_CURRENT_LIST_DIR}/../prebuilt_libs/
        COMMENT "Copying lib-files to location ${CMAKE_CURRENT_LIST_DIR}/../prebuilt_libs/"
        COMMAND_EXPAND_LISTS
        )