cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(ngin)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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
find_package(Freetype REQUIRED)
find_package(ZLIB REQUIRED) # Added ZLIB
find_package(PNG REQUIRED)  # Added PNG
find_package(OpenAL REQUIRED)

message(STATUS "GLM: ${GLM_INCLUDE_DIR}")
message(STATUS "GLFW3: ${GLFW3_INCLUDE_DIR}")
message(STATUS "FreeType: ${FREETYPE_INCLUDE_DIR}")
message(STATUS "ZLIB: ${ZLIB_INCLUDE_DIRS}") # Added ZLIB message
message(STATUS "PNG: ${PNG_INCLUDE_DIRS}")   # Added PNG message
message(STATUS "OpenAL Libraries: ${OPENAL_LIBRARY}")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/includes/)
include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${OpenAL_INCLUDE_DIRS})

# Set libraries and platform-specific settings
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    set(LIBS glfw3 opengl32 freetype)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype)
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
    include_directories(/System/Library/Frameworks)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKit_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
    set(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY} ${GLFW3_LIBRARY} ${FREETYPE_LIBRARIES})
    set(LIBS ${LIBS} ${APPLE_LIBS})
endif()

# Add source files
set(SOURCES
    src/vendor/glad.c
    src/vendor/stb_image.cpp

    src/ngin/asset/types/manifest.cpp
    src/ngin/scene/module/kinds/manifest.cpp
    
    src/main.cpp
)

# Configure and include root directory header
configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

# Add executable
add_executable(ngin ${SOURCES})

# Link libraries
target_link_libraries(ngin ${LIBS} ZLIB::ZLIB PNG::PNG ${OPENAL_LIBRARY}) # Added OpenAL

# MSVC specific flags
if(MSVC)
    target_compile_options(ngin PRIVATE /MP) 
    target_link_options(ngin PUBLIC /ignore:4099)
endif()