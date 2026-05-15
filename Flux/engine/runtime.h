#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include "3DRenderer.h"
#include "mechanics/Scenenode.h"
#include "luaEngine.h"

namespace Flux {
    class Runtime {
        public:
            void Start(const std::string& projectName, const std::filesystem::path& projectPath, std::vector<SceneNode>& copiedNodes);
            void Update();
            void Stop();

            bool isRunning = false;
        private:
            SDL_Window* m_window = nullptr;
            SDL_GLContext m_glContext;

            Renderer3D m_renderer;
            LuaEngine m_luaEngine;
            std::vector<SceneNode> m_gameNodes;
    };
}