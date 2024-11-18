cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(nev_engine)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Find the required packages (replace with your actual dependencies)
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")
find_package(IRRKLANG REQUIRED)
message(STATUS "Found IRRKLANG in ${IRRKLANG_INCLUDE_DIR}")

include_directories(${CMAKE_SOURCE_DIR}/includes/)

if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    set(LIBS glfw3 opengl32 assimp freetype irrKlang) # Add your Windows-specific libraries
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    find_package(OpenGL REQUIRED)
    add_definitions(${OPENGL_DEFINITIONS})
    find_package(X11 REQUIRED)
    set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY} ${IRRKLANG_LIBRARY}) # Add your Linux-specific libraries
    set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
    INCLUDE_DIRECTORIES(/System/Library/Frameworks)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
    FIND_LIBRARY(IOKit_LIBRARY IOKit)
    FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
    MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
    SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY} ${FREETYPE_LIBRARIES}) # Add your macOS-specific libraries
    set(LIBS ${LIBS} ${APPLE_LIBS})
else()
  set(LIBS )
endif(WIN32)


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


configure_file(configuration/root_directory.h.in configuration/root_directory.h)

include_directories(${CMAKE_BINARY_DIR}/configuration)

# Add your main.cpp file
add_executable(nev_engine ${SOURCES})

# Link the necessary libraries
target_link_libraries(nev_engine ${LIBS})

if(MSVC)
    target_compile_options(nev_engine PRIVATE /std:c++17 /MP)
    target_link_options(nev_engine PUBLIC /ignore:4099)
endif(MSVC)
