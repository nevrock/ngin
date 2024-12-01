#ifndef MESH_H
#define MESH_H

class Mesh {
    
public:
    unsigned int VAO, VBO, EBO;

private:
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
}


#endif // MESH_H
