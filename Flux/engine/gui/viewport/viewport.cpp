#include "viewport.h"
#include "OpenGLManager.h"
#include "3DRenderer.h"
#include "Model.h"

namespace Flux {
    void Viewport::Init() {
        glManager = std::make_unique<OpenGLManager>();
        renderer = std::make_unique<Renderer3D>();

        glManager->Init(1280, 720);
        renderer->Init();

        currentModel = std::make_unique<Model>(modelPath);
    }

    void Viewport::RenderViewport() {
        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImVec2 size = ImGui::GetContentRegionAvail();

        glManager->Resize((int)size.x, (int)size.y);

        glManager->Bind();

        glViewport(0, 0, (int)size.x, (int)size.y);

        float aspectRatio = size.x / size.y;
        glm::mat4 view = glm::lookAt(glm::vec3(0, 5, 15), glm::vec3(0), glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

        renderer->DrawScene(*currentModel, view, proj, (float)ImGui::GetTime());

        glManager->Unbind();

        ImGui::Image((void*)(intptr_t)glManager->GetTexture(), size, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();
    }
}