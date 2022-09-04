CMAKE_MINIMUM_REQUIRED(VERSION 3.23)
project(kronos LANGUAGES CXX )

option(KRONOS_BUILD_EXEC "Build exec to test kronos_engine" ON) 

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
find_package(glfw3 3.3)


#add_definitions(-DUNICODE -D_UNICODE)
#add_compile_definitions(NOMINMAX)
target_compile_definitions(engine 
    PRIVATE 
        $<$<CXX_COMPILER_ID:MSVC>:UNICODE _UNICODE NOMINMAX>
)
add_compile_options("/source-charset:utf-8;/execution-charset:utf-8")
target_compile_options(engine PRIVATE $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/DEBUG /W4 /Zi /source-charset:utf-8 /execution-charset:utf-8>>)
target_include_directories(engine PUBLIC ${Vulkan_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/Linking/Include)
target_link_libraries(engine PUBLIC ${Vulkan_LIBRARIES} ${CMAKE_CURRENT_LIST_DIR}/Linking/Lib/GLFW/glfw3.lib )
# add_custom_command(TARGET engine POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:engine> $<TARGET_FILE_DIR:engine>
#         COMMENT "Copying dll-files to location of executable..."
#         COMMAND_EXPAND_LISTS
#         )

# If we will build the executable for devs to test...
if(${KRONOS_BUILD_EXEC})
    
    add_executable(engine_demo)
    target_compile_features(engine_demo PUBLIC cxx_std_20)    
    target_sources(engine_demo PRIVATE Main.cpp ${ENGINE_SOURCES} ${ENGINE_HEADERS})

    target_link_options(engine_demo
        PRIVATE 
            $<$<CXX_COMPILER_ID:MSVC>:/subsystem:console /NODEFAULTLIB:library>
    )
    target_compile_options(engine_demo 
        PRIVATE 
            $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:MSVC>:/DEBUG /W4 /Zi /source-charset:utf-8 /execution-charset:utf-8>>
    )
    target_include_directories(engine_demo 
        PUBLIC 
            ${Vulkan_INCLUDE_DIR} 
            ${CMAKE_SOURCE_DIR}/Linking/Include
    )
    target_link_libraries(engine_demo 
        PUBLIC 
            ${Vulkan_LIBRARIES} 
            ${CMAKE_CURRENT_LIST_DIR}/Linking/Lib/GLFW/glfw3.lib 
    )
    set_property(TARGET engine_demo 
        PROPERTY 
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    
    target_compile_definitions(engine_demo 
        PRIVATE 
            $<$<CXX_COMPILER_ID:MSVC>:UNICODE _UNICODE NOMINMAX>
    )
    
endif()


add_custom_command(TARGET engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:engine> ${CMAKE_CURRENT_LIST_DIR}/../prebuilt_libs/
        COMMENT "Copying lib-files to location ${CMAKE_CURRENT_LIST_DIR}/../prebuilt_libs/"
        COMMAND_EXPAND_LISTS
        )