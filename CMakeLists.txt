cmake_minimum_required(VERSION 3.15)
project(nev)

set(CMAKE_CXX_STANDARD 20)  # Use C++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

link_directories(${CMAKE_SOURCE_DIR}/lib/)

# Find OpenGL
find_package(OpenGL REQUIRED)
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

include_directories(${CMAKE_SOURCE_DIR}/includes/)

# Add source files
set(SOURCES
    src/glad.c
    src/stb_image.cpp
    src/game.cpp
    src/main.cpp
    src/animator.cpp
    src/animation.cpp
    src/surface_sdf.cpp
    src/worker_manager.cpp
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

# Add executable
add_executable(nev ${SOURCES})

# Link libraries
target_link_libraries(nev ${OPENGL_LIBRARIES} glfw3 freetype assimp irrKlang)
# target_link_options(nev PRIVATE "/VERBOSE")

add_custom_command(TARGET nev POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/dlls/glfw3.dll"
    $<TARGET_FILE_DIR:nev>)

add_custom_command(TARGET nev POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/dlls/assimp-vc143-mtd.dll"
    $<TARGET_FILE_DIR:nev>)

add_custom_command(TARGET nev POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/dlls/irrKlang.dll"
    $<TARGET_FILE_DIR:nev>)

add_custom_command(TARGET nev POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/dlls/ikpMP3.dll"
    $<TARGET_FILE_DIR:nev>)