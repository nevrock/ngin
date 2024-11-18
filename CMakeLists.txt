cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(nev_engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(ROOT_DIR_PARENT "${CMAKE_SOURCE_DIR}" DIRECTORY)

# Set CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Set build type if not defined
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Find required packages
find_package(GLM REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(IRRKLANG REQUIRED)
message(STATUS "GLM: ${GLM_INCLUDE_DIR}")
message(STATUS "GLFW3: ${GLFW3_INCLUDE_DIR}")
message(STATUS "ASSIMP: ${ASSIMP_INCLUDE_DIR}")
message(STATUS "IRRKLANG: ${IRRKLANG_INCLUDE_DIR}")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/includes/)

# Set libraries and platform-specific settings
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    set(LIBS glfw3 opengl32 assimp freetype irrKlang)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY} ${IRRKLANG_LIBRARY})
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
    include_directories(/System/Library/Frameworks)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKit_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
    set(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY} ${FREETYPE_LIBRARIES})
    set(LIBS ${LIBS} ${APPLE_LIBS})
endif()

# Add source files
set(SOURCES
    src/glad.c
    src/stb_image.cpp
    src/game.cpp
    src/main.cpp
    src/animator.cpp
    src/animation.cpp
    src/surface_sdf.cpp
    src/model/model_primitive.cpp
    src/model/model_fbx.cpp
    src/model/model_ui.cpp
    src/model/model_drawer.cpp
    src/model/model_text.cpp
    src/model/model_sdf.cpp
    src/physics/body.cpp
    src/physics/body_box.cpp
    src/physics/body_mesh.cpp
    src/physics/physics_manager.cpp
    src/light/light.cpp
    src/camera/camera.cpp
)

# Configure and include root directory header
configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

# Add executable
add_executable(nev_engine ${SOURCES})

# Link libraries
target_link_libraries(nev_engine ${LIBS})

# MSVC specific flags
if(MSVC)
    target_compile_options(nev_engine PRIVATE /std:c++17 /MP)
    target_link_options(nev_engine PUBLIC /ignore:4099)
endif()
