#include "runtime.h"
#include <glad/glad.h>

// "I just finished my homework lol" - Idkthisguy May 14, 2026

namespace Flux
{

    void Runtime::Start(const std::string &projectName, const std::filesystem::path &projectPath,
                    std::vector<SceneNode> &copiedNodes)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(projectName.c_str(), 1270, 720, SDL_WINDOW_OPENGL);
    m_glContext = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_glContext);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        return;
    }

    SDL_GL_SetSwapInterval(1);

    m_renderer.Init();
    m_gameNodes = copiedNodes;

    m_luaEngine.init();
    m_luaEngine.runAllScriptsInFolder(projectPath.string());
    m_luaEngine.isRunning = true;

    isRunning = true;

    SDL_GL_MakeCurrent(NULL, NULL);
}

void Runtime::Update()
{
    if (!isRunning) return;

    SDL_GL_MakeCurrent(m_window, m_glContext);

    SDL_Event e;
    while (SDL_PollEvent(&e))
        if (e.type == SDL_EVENT_QUIT) { Runtime::Stop(); return; }

    m_luaEngine.step();

    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    glViewport(0, 0, w, h);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);

    for (auto& node : m_gameNodes)
    {
        if (node.model)
        {
            m_renderer.DrawScene(*node.model, node.textureID,
                                 node.GetTransformMatrix(), view, proj,
                                 glm::vec3(0), m_gameNodes);
        }
    }

    SDL_GL_SwapWindow(m_window);
    SDL_GL_MakeCurrent(NULL, NULL);
}

void Runtime::Stop()
{
    m_luaEngine.stop();
    if (m_glContext) SDL_GL_DestroyContext(m_glContext);
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
    isRunning = false;
}

} // namespace Flux