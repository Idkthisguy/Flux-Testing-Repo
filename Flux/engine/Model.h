#pragma once
#include <glad/glad.h> // The Model needs GLAD to talk to the GPU memory
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp> // Need GLM for the vectors
#include <vector>
#include <string>

namespace Flux {
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
    };

    struct Mesh {
        unsigned int VAO, VBO, EBO;
        unsigned int indexCount;
    };

    class Model {
    public:
        std::string path;
        std::vector<Mesh> meshes;

        Model(const std::string& modelPath) : path(modelPath) { Load(); }
        void Load();
        void Draw();
    };
}