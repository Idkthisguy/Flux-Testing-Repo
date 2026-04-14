#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Model.h"

namespace Flux {
    class Renderer3D {
    public:
        void Init();
        void DrawScene(Model& model, glm::mat4 view, glm::mat4 proj, float time);

    private:
        unsigned int shaderProgram;
    };
}