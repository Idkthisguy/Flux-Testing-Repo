#include "runtime.h"
#include <glad/glad.h>
#include <iostream>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/Quat.h>

namespace Flux
{

void Runtime::Start(const std::string &projectName, const std::filesystem::path &projectPath,
                    std::vector<SceneNode> &copiedNodes, int windowWidth, int windowHeight)
{
    if (isRunning)
        Stop();

    isRunning = true;

    lastTimeFrame = SDL_GetPerformanceCounter();

    if (!std::filesystem::exists(projectPath) || !std::filesystem::is_directory(projectPath))
    {
        Output::addLog("RUNTIME ERROR: Project folder missing: " + projectPath.string());
        isRunning = false;
        return;
    }

    Output::addLog("Runtime: Creating game window...");

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    m_window =
        SDL_CreateWindow(projectName.c_str(), windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!m_window)
    {
        Output::addLog("RUNTIME ERROR: SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        isRunning = false;
        return;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        Output::addLog("RUNTIME ERROR: SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        isRunning = false;
        return;
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);

    SplashConfig splash;
    splash.title = "Flux Game";
    splash.subtitle = "Loading...";
    Flux::RunSplashScreen(m_window, splash);

    if (!SDL_GL_MakeCurrent(m_window, m_glContext))
    {
        Output::addLog("RUNTIME ERROR: Context re-binding failed after splash screen: " + std::string(SDL_GetError()));
        isRunning = false;
        return;
    }

    SDL_GL_SetSwapInterval(1);

    for (auto &[path, id] : m_runtimeTextureCache)
        if (id)
            glDeleteTextures(1, &id);
    m_runtimeTextureCache.clear();

    m_renderer.Init();
    m_renderer.InitSkybox();
    m_renderer.InitShadowMap(4096);

    m_gameNodes.clear();

    for (const auto &node : copiedNodes)
    {
        SceneNode newNode = node;

        if (node.model)
            newNode.model = std::make_shared<Model>(node.model->path);

        if (!node.texturePath.empty())
        {
            auto it = m_runtimeTextureCache.find(node.texturePath);
            if (it != m_runtimeTextureCache.end())
            {
                newNode.textureID = it->second;
            }
            else
            {
                unsigned int id = TextureLoader::Load(node.texturePath);
                m_runtimeTextureCache[node.texturePath] = id;
                newNode.textureID = id;
            }
        }

        newNode.baseColor = node.baseColor;
        newNode.roughness = node.roughness;
        newNode.metallic = node.metallic;

        m_gameNodes.push_back(std::move(newNode));
    }

    m_luaEngine.activeNodes = &m_gameNodes;
    m_luaEngine.init();
    m_luaEngine.bindEngineAPI();
    m_luaEngine.isRunning = true;
    m_luaEngine.runAllScriptsInFolder(projectPath.string());

    static bool joltInitialized = false;

    if (!joltInitialized) {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        joltInitialized = true;
    }
    m_tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    m_jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() -1);

    m_PhysicsSystem = new JPH::PhysicsSystem();

    JPH::PhysicsSettings settings;

    m_PhysicsSystem->SetGravity(JPH::Vec3(0.0f, -4.0f,  0.0f));

    m_PhysicsSystem->Init(1024, 
        0, 
        1024, 
        1024, 
        m_broadPhaseLayerInterface, 
        m_objectVsBroadPhaseLayerFilter, 
        m_objectLayerPairFilter
    );

    JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();
    for (auto &node : m_gameNodes) {
        if (node.type == NodeType::Mesh) {
            JPH::BoxShapeSettings shapeSettings(JPH::Vec3(node.scale.x, node.scale.y, node.scale.z));
            JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();

            if (shapeResult.HasError()) {
                Output::addLog("Jolt shape Creation Error: " + std::string(shapeResult.GetError().c_str()));
                std::cerr << "JOLT SHAPE CREATION ERROR!!: " + std::string(shapeResult.GetError().c_str());
                continue;
            }

            JPH::ShapeRefC shape = shapeResult.Get();
            
            JPH::EMotionType motionType = node.isAnchored ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
            JPH::ObjectLayer    layer = node.isAnchored ? Flux::Layers::NON_MOVING : Flux::Layers::MOVING;

            auto rot = glm::radians(node.rotation);
            JPH::Quat joltRotation = JPH::Quat::sEulerAngles(
                JPH::Vec3(rot.x, rot.y, rot.z)
            );

            JPH::BodyCreationSettings settings(
                shape,
                JPH::Vec3(node.position.x, node.position.y, node.position.z),
                joltRotation,
                motionType,
                layer
            );

            JPH::Body* body = bodyInterface.CreateBody(settings);
            bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);

            node.physicsBodyID = body->GetID();
        }
    }

    Output::addLog("Runtime started successfully.");

    if (m_window && m_glContext)
        SDL_GL_MakeCurrent(m_window, nullptr);
}

void Runtime::Update()
{
    if (!isRunning || !m_window || !m_glContext)
        return;

    if (!SDL_GL_MakeCurrent(m_window, m_glContext))
    {

        Output::addLog("RUNTIME ERROR: Context re-binding failed: " + std::string(SDL_GetError()));
        isRunning = false;
        return;
    }

    if (!isRunning || !m_window || !m_glContext)
        return;

    m_luaEngine.step();

    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);

    glViewport(0, 0, w, h);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float gameTime = 14.0f;
    glm::vec3 gameSunDir = glm::vec3(0, -1, 0);

    glm::mat4 proj = glm::perspective(glm::radians(70.0f), (float)w / (float)h, 0.1f, 2000.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0, 1, 0));

    uint64_t semiCurrentTIme = SDL_GetPerformanceCounter();
    float dt = (float)(semiCurrentTIme - lastTimeFrame) / (float)SDL_GetPerformanceFrequency();

    if (dt > 0.33f) dt = 0.33f;

    m_PhysicsSystem->Update(dt, 1, m_tempAllocator, m_jobSystem);

    JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();
    for (auto &node : m_gameNodes)
    {
        if (node.type == NodeType::Mesh && !node.isAnchored) {
            JPH::Vec3 pos = bodyInterface.GetPosition(node.physicsBodyID);
            JPH::Quat rot = bodyInterface.GetRotation(node.physicsBodyID);

            glm::quat glmRot(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
            glm::mat4 rotMat = glm::mat4_cast(glmRot);
            glm::mat4 transMat = glm::translate(glm::mat4(1.0f), glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
            glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), node.scale);

            node.physicsWorldMatrix = transMat * rotMat * scaleMat;
            node.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
        }

        if (node.type == NodeType::Camera && node.isMainCamera)
        {
            glm::mat4 transform = node.GetTransformMatrix();
            glm::vec3 camPos = node.position;
            glm::vec3 camFront = glm::normalize(glm::vec3(transform * glm::vec4(0, 0, -1, 0)));
            glm::vec3 camUp = glm::normalize(glm::vec3(transform * glm::vec4(0, 1, 0, 0)));

            view = glm::lookAt(camPos, camPos + camFront, camUp);
            proj = glm::perspective(glm::radians(node.fov > 0 ? node.fov : 70.0f), (float)w / (float)h, 0.1f, 2000.0f);
            break;
        }
        if (node.isLightingNode)
        {
            gameTime = node.light.timeOfDay;
            gameSunDir = node.light.direction;
        }
    }

    m_renderer.DrawSkybox(view, proj, gameSunDir, gameTime, true);

    for (auto &node : m_gameNodes)
    {
        if (node.model)
        {
            glm::mat4 modelMat = node.GetTransformMatrix();
            m_renderer.DrawScene(*node.model, node.textureID, modelMat, view, proj, cameraPos, m_gameNodes, 1.0f,
                                 node.roughness, node.metallic, gameTime, node.baseColor);
        }
    }

    SDL_GL_SwapWindow(m_window);

    if (m_window && m_glContext)
        SDL_GL_MakeCurrent(m_window, nullptr);

    lastTimeFrame = semiCurrentTIme;
}

void Runtime::Stop()
{
    if (!isRunning)
        return;

    Output::addLog("Stopping runtime...");

    if (m_tempAllocator != nullptr && m_PhysicsSystem != nullptr) {
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();
        for (auto &node : m_gameNodes) {
            if (node.type == NodeType::Mesh) {
                bodyInterface.RemoveBody(node.physicsBodyID);
                bodyInterface.DestroyBody(node.physicsBodyID);
            }
        }

        delete m_PhysicsSystem;
        m_PhysicsSystem = nullptr;

        delete m_jobSystem;
        m_jobSystem = nullptr;

        delete m_tempAllocator;
        m_tempAllocator = nullptr;
    }

    m_luaEngine.stop();
    m_luaEngine.isRunning = false;

    if (m_glContext && m_window)
    {
        SDL_GL_MakeCurrent(m_window, m_glContext);
        for (auto &[path, id] : m_runtimeTextureCache)
            if (id)
                glDeleteTextures(1, &id);
        m_runtimeTextureCache.clear();
    }

    if (m_glContext)
    {
        SDL_GL_DestroyContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    m_gameNodes.clear();
    isRunning = false;

    Output::addLog("Runtime stopped.");
}

void Runtime::SyncCamera(glm::vec3 editorPos, glm::vec3 editorTarget)
{
    this->cameraPos = editorPos;
    this->cameraTarget = editorTarget;
}

} // namespace Flux