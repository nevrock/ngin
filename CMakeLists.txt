cmake_minimum_required(VERSION 3.15)
project(ngin)

set(CMAKE_CXX_STANDARD 20)  # Use C++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

link_directories(${CMAKE_SOURCE_DIR}/lib/)

# Find OpenGL
find_package(OpenGL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/imgui/)

# Add source files
set(SOURCES
    include/imgui/imgui.cpp

    include/imgui/imgui_impl_glfw.cpp
    include/imgui/imgui_impl_opengl3.cpp

    include/imgui/imgui_tables.cpp
    include/imgui/imgui_widgets.cpp

    include/imgui/imgui_draw.cpp

    include/imgui/imgui_demo.cpp
    src/main.cpp
)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)

include_directories(${CMAKE_BINARY_DIR}/configuration)

# Add executable
add_executable(ngin ${SOURCES})


# Link libraries
target_link_libraries(ngin ${OPENGL_LIBRARIES} /usr/local/lib/libglfw3.a)
# target_link_options(ngin PRIVATE "/VERBOSE")

add_custom_command(TARGET ngin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/dlls/glfw3.dll"
    $<TARGET_FILE_DIR:ngin>)