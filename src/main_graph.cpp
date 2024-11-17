#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <iostream>
#include <thread>
#include <atomic>

#include <snorri_graphs/graph_generator.h>
#include <snorri/dict.h>
#include <snorri/fileutils.h>

int main()
{
    GraphGenerator graph(25, 0.5);
    graph.loadFromFile(FileUtils::getResourcePath("snorri/map_generator.snorri"));
    graph.saveToPNG(FileUtils::getResourcePath("out/grid.png").c_str());

    return 0;
}