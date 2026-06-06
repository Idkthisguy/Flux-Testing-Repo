#include "Window.h"
#include <iostream>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "imgui_internal.h"
#include <SDL3/SDL.h>

#include <ShObjIdl.h>
#include <propkey.h>

namespace Flux
{
void SetStalkerTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Call StyleColorsDark first as Aurora's parent theme
    ImGui::StyleColorsDark(&style);

    // Apply Aurora Sizing Variables (from Style.hpp)
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(20.0f, 8.0f);
    style.ItemSpacing = ImVec2(20.0f, 8.0f);
    style.ScrollbarSize = 17.0f;
    style.ScrollbarRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;

    // Set other roundings to follow the 8.0f standard
    style.WindowRounding = 8.0f;
    style.ChildRounding = 0.0f;
    style.PopupRounding = 8.0f;

    // Apply Border Sizes
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;

    // Aurora Theme Colors
    ImVec4 s_MainBgLight0 = ImVec4(0.404f, 0.404f, 0.404f, 1.0f);
    ImVec4 s_MainBg      = ImVec4(0.21f, 0.21f, 0.21f, 1.0f);
    ImVec4 s_MainBgDark0  = ImVec4(0.190f, 0.190f, 0.190f, 1.0f);
    ImVec4 s_MainBgDark1  = ImVec4(0.145f, 0.145f, 0.145f, 1.0f);
    ImVec4 s_MainBgDark2  = ImVec4(0.098f, 0.098f, 0.098f, 1.0f);

    ImVec4 s_Accent      = ImVec4(0.149f, 0.149f, 0.149f, 1.0f);
    ImVec4 s_AccentDark0 = ImVec4(0.102f, 0.102f, 0.102f, 1.0f);
    ImVec4 s_AccentDark1 = ImVec4(0.063f, 0.063f, 0.063f, 1.0f);

    ImVec4 s_Button      = ImVec4(0.882f, 0.882f, 0.882f, 1.0f);
    ImVec4 s_ButtonHovered = ImVec4(0.782f, 0.782f, 0.782f, 1.0f);

    ImVec4 s_Header      = ImVec4(0.338f, 0.338f, 0.338f, 1.0f);
    ImVec4 s_HeaderHovered = ImVec4(0.276f, 0.276f, 0.276f, 1.0f);
    ImVec4 s_HeaderActive  = ImVec4(0.379f, 0.379f, 0.379f, 1.0f);

    ImVec4 s_Font        = ImVec4(0.902f, 0.902f, 0.902f, 1.0f);
    ImVec4 s_FontDisabled = ImVec4(0.36f, 0.36f, 0.36f, 1.0f);
    ImVec4 s_HighlightColor = ImVec4(0.145f, 0.553f, 0.384f, 1.0f);

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]             = s_MainBg;
    colors[ImGuiCol_ChildBg]              = s_MainBg;

    colors[ImGuiCol_Text]                 = s_Font;
    colors[ImGuiCol_TextDisabled]         = s_FontDisabled;
    colors[ImGuiCol_TextSelectedBg]       = s_HighlightColor;

    colors[ImGuiCol_FrameBg]              = s_MainBgDark1;
    colors[ImGuiCol_FrameBgHovered]       = s_MainBgDark0;
    colors[ImGuiCol_FrameBgActive]        = s_MainBgDark2;

    colors[ImGuiCol_TitleBg]              = s_MainBgDark0;
    colors[ImGuiCol_TitleBgCollapsed]     = s_MainBgDark0;
    colors[ImGuiCol_TitleBgActive]        = s_MainBgDark0;
    colors[ImGuiCol_MenuBarBg]            = s_AccentDark0;

    colors[ImGuiCol_Tab]                  = s_MainBgDark0;
    colors[ImGuiCol_TabUnfocused]         = s_MainBgDark0;
    colors[ImGuiCol_TabHovered]           = s_MainBgDark1;
    colors[ImGuiCol_TabActive]            = s_MainBgDark1;
    colors[ImGuiCol_TabUnfocusedActive]   = s_MainBgDark1;

    colors[ImGuiCol_ScrollbarBg]          = s_MainBgDark1;
    colors[ImGuiCol_ScrollbarGrab]        = s_Font;
    colors[ImGuiCol_ScrollbarGrabActive]  = s_FontDisabled;
    colors[ImGuiCol_ScrollbarGrabHovered] = s_FontDisabled;
    colors[ImGuiCol_CheckMark]            = s_Font;
    colors[ImGuiCol_SliderGrab]           = s_Font;
    colors[ImGuiCol_SliderGrabActive]     = s_FontDisabled;

    colors[ImGuiCol_Header]               = s_Header;
    colors[ImGuiCol_HeaderHovered]        = s_HeaderHovered;
    colors[ImGuiCol_HeaderActive]         = s_HeaderActive;

    colors[ImGuiCol_Separator]            = s_MainBgLight0;
    colors[ImGuiCol_SeparatorHovered]     = s_MainBgLight0;
    colors[ImGuiCol_SeparatorActive]      = s_MainBgLight0;
    colors[ImGuiCol_Border]               = s_MainBgLight0;

    colors[ImGuiCol_ResizeGrip]           = s_MainBg;
    colors[ImGuiCol_ResizeGripHovered]    = s_MainBg;
    colors[ImGuiCol_ResizeGripActive]     = s_MainBg;

    colors[ImGuiCol_DockingPreview]       = s_AccentDark0;
    colors[ImGuiCol_NavHighlight]         = s_AccentDark0;
}

Window::Window(int width, int height, const std::string &title) : m_width(width), m_height(height), m_title(title)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "ERROR: FAILED TO INITIALIZE SDL3: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    

    m_window = SDL_CreateWindow(m_title.c_str(), m_width, m_height,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!m_window)
    {
        std::cerr << "FATAL: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    if (m_window != nullptr)
    {
        SDL_StartTextInput(m_window);
        std::cout << "SUCCESSFULLY STARTED TEXT INPUT" << std::endl;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        std::cerr << "FATAL: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw std::runtime_error("SDL_GL_CreateContext failed");
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "ERROR: FAILED TO INITIALIZE GLAD" << std::endl;
    }

    {
        int iconW, iconH, iconCh;
        unsigned char *pixels = stbi_load("assets/icon.png", &iconW, &iconH, &iconCh, 4);
        if (pixels)
        {
            SDL_Surface *icon = SDL_CreateSurfaceFrom(iconW, iconH, SDL_PIXELFORMAT_RGBA32, pixels, iconW * 4);
            if (icon)
            {
                SDL_SetWindowIcon(m_window, icon);
                SDL_DestroySurface(icon);
            }
            stbi_image_free(pixels);
        }
    }

#if defined(_WIN32)
    {
        SetCurrentProcessExplicitAppUserModelID(L"IdkthisguysStuff.FluxEngine.Core");

        SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
        HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        if (hwnd)
        {
            BOOL useDarkMode = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
            
            IPropertyStore* pPropStore = nullptr;
            if (SUCCEEDED(SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(&pPropStore)))) {
                PROPVARIANT propvar;
                PropVariantInit(&propvar);
                propvar.vt = VT_LPWSTR;
                propvar.pwszVal = const_cast<wchar_t*>(L"ZeroPointStudio.FluxEngine.Core");
                
                pPropStore->SetValue(PKEY_AppUserModel_ID, propvar);
                pPropStore->Commit();
                pPropStore->Release();
            }
        }

        HWND consoleHwnd = GetConsoleWindow();
        if (consoleHwnd) {
            IPropertyStore* pConsoleStore = nullptr;
            if (SUCCEEDED(SHGetPropertyStoreForWindow(consoleHwnd, IID_PPV_ARGS(&pConsoleStore)))) {
                PROPVARIANT propvar;
                PropVariantInit(&propvar);
                propvar.vt = VT_LPWSTR;
                propvar.pwszVal = const_cast<wchar_t*>(L"IdkthisguysStuff.FluxEngine.Core");
                
                pConsoleStore->SetValue(PKEY_AppUserModel_ID, propvar);
                pConsoleStore->Commit();
                pConsoleStore->Release();
            }
        }
    }
#endif

    m_explorer.textEditor = &m_texteditor;
    m_explorer.ribbonPtr = &m_ribbon;

    m_ribbon.luaEnginePtr = &m_luaEngine;
    m_ribbon.textEditorPtr = &m_texteditor;
    m_ribbon.explorerPtr = &m_explorer;
    m_ribbon.heiarchyPtr = &m_heiarchy;
    m_ribbon.viewportPtr = &m_viewport;
    m_ribbon.LoadPreferences();
    m_texteditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());

    m_viewport.ribbonPtr = &m_ribbon;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 410");

    SetStalkerTheme();

    m_viewport.Init();
    m_heiarchy.setup();
    m_luaEngine.init();

    // Auto-load last project or fallback to default UntitledProject
    std::filesystem::path projectToLoad = m_ribbon.lastProjectPath;
    if (projectToLoad.empty() || !std::filesystem::exists(projectToLoad))
    {
        const char* home = std::getenv("HOME");
        if (!home) home = std::getenv("USERPROFILE");
        if (home)
        {
            std::filesystem::path defaultProj = std::filesystem::path(home) / "FluxProjects" / "UntitledProject";
            std::filesystem::create_directories(defaultProj);
            
            // Copy template if empty or doesn't exist
            std::filesystem::path templatePath = PathHelper::GetAssetPath("templates/Project_Templates/base_game_folder_lua");
            if (std::filesystem::exists(templatePath))
            {
                try {
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(templatePath)) {
                        const auto& path = entry.path();
                        auto relative = std::filesystem::relative(path, templatePath);
                        auto dest = defaultProj / relative;
                        if (entry.is_directory()) {
                            std::filesystem::create_directories(dest);
                        } else {
                            if (!std::filesystem::exists(dest)) {
                                std::filesystem::copy_file(path, dest);
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Failed to copy default templates: " << e.what() << std::endl;
                }
            }
            
            // Create UntitledProject.flux project file if not exists
            std::filesystem::path projFile = defaultProj / "UntitledProject.flux";
            if (!std::filesystem::exists(projFile))
            {
                nlohmann::json pj;
                pj["projectName"] = "UntitledProject";
                pj["startupScene"] = "scene.fscn";
                pj["currentScene"] = "scene.fscn";
                std::ofstream(projFile) << pj.dump(4);
            }
            
            // Save default scene to UntitledProject/scene.fscn if not exists
            std::filesystem::path scenePath = defaultProj / "scene.fscn";
            if (!std::filesystem::exists(scenePath))
            {
                SceneSerializer::Save(m_heiarchy, scenePath.string(), defaultProj);
            }
            
            projectToLoad = defaultProj;
        }
    }
    
    if (!projectToLoad.empty() && std::filesystem::exists(projectToLoad))
    {
        m_explorer.activeFolderPath = projectToLoad;
        m_explorer.projectRoot.path = projectToLoad;
        m_explorer.projectRoot.name = projectToLoad.filename().string();
        m_explorer.syncFiles(projectToLoad, m_explorer.projectRoot);
        m_explorer.scanForBackups();
        
        m_viewport.activeProjectPath = projectToLoad;
        m_ribbon.lastProjectPath = projectToLoad.string();
        m_ribbon.SavePreferences();
        
        // Load the scene saved in project settings or default to scene.fscn / main.fscn
        m_ribbon.LoadProjectSettings(projectToLoad);
        std::string sceneName = std::strlen(m_ribbon.projectSettings.currentScene) > 0 ? m_ribbon.projectSettings.currentScene : "scene.fscn";
        std::filesystem::path scenePath = sceneName;
        if (!scenePath.is_absolute())
        {
            scenePath = projectToLoad / scenePath;
        }
        if (std::filesystem::exists(scenePath))
        {
            SceneSerializer::Load(m_heiarchy, scenePath, projectToLoad);
        }
    }
}

Window::~Window()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (m_glContext)
        SDL_GL_DestroyContext(m_glContext);
    if (m_window)
        SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Window::shouldClose() const
{
    return m_window == nullptr || m_shouldClose;
}

void Window::update()
{
    ImGuiIO &io = ImGui::GetIO();

    if (io.WantTextInput)
    {

        SDL_StartTextInput(m_window);
    }
    if (m_window == nullptr)
        return;

    SDL_GL_MakeCurrent(m_window, m_glContext);

    if (m_pendingStart)
    {
        m_pendingStart = false;
        StartRuntimeEngine();

        if (!SDL_GL_MakeCurrent(m_window, m_glContext))
        {
            Output::addLog("RUNTIME ERROR: Main loop context re-binding failed: " + std::string(SDL_GetError()));
        }
    }
    if (m_pendingStop)
    {
        m_pendingStop = false;
        StopRuntimeEngine();

        SDL_GL_MakeCurrent(m_window, m_glContext);
    }

    Uint32 editorWindowID = SDL_GetWindowID(m_window);
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {

        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_KEY_DOWN)
        {

            if (event.key.key == SDLK_ESCAPE)
            {
                m_shouldClose = true;
            }

            if (io.WantCaptureKeyboard)
            {

                continue;
            }
        }

        bool isEditorWindowEvent = (event.type < SDL_EVENT_WINDOW_FIRST || event.type > SDL_EVENT_WINDOW_LAST) ||
                                   (event.window.windowID == editorWindowID);

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            m_shouldClose = true;
            break;

        /*case SDL_EVENT_TEXT_INPUT:

            ImGui::GetIO().AddInputCharactersUTF8(event.text.text);
            break;
        */
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (event.window.windowID == editorWindowID)
            {
                m_shouldClose = true;
            }
            else if (m_runtime.isRunning)
            {
                m_pendingStop = true;
            }
            break;
        case SDL_EVENT_DROP_FILE:
            if (!m_explorer.activeFolderPath.empty())
            {
                std::filesystem::path srcPath(event.drop.data);
                std::string ext = srcPath.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                std::filesystem::path destFolder = m_explorer.activeFolderPath;
                if (ext == ".obj" || ext == ".fbx")
                    destFolder /= "models";
                else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
                    destFolder /= "textures";

                std::filesystem::create_directories(destFolder);
                std::filesystem::path destPath = destFolder / srcPath.filename();
                try
                {
                    std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
                    if (ext == ".obj")
                    {
                        std::filesystem::path mtlSrc = srcPath;
                        mtlSrc.replace_extension(".mtl");
                        if (std::filesystem::exists(mtlSrc))
                        {
                            std::filesystem::path mtlDest = destFolder / mtlSrc.filename();
                            std::filesystem::copy_file(mtlSrc, mtlDest, std::filesystem::copy_options::overwrite_existing);
                        }
                    }
                    m_explorer.refreshRequested = true;
                    Output::addLog("Imported asset: " + srcPath.filename().string());
                }
                catch (const std::exception &e)
                {
                    Output::addLog("Import failed: " + std::string(e.what()));
                }
            }
            break;
        }
    }

    static bool lastState = false;
    if (io.WantCaptureKeyboard != lastState)
    {

        lastState = io.WantCaptureKeyboard;
        std::cout << "[TRAP 2] ImGui Keyboard Capture Changed! Focus State: " << lastState << std::endl;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImVec2 dockPos = viewport->Pos;
    dockPos.y += 75.0f;
    ImVec2 dockSize = viewport->Size;
    dockSize.y -= 100.0f; // 75.0f (ribbon) + 25.0f (status bar)

    static bool firstTime = true;
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

    ImGui::SetNextWindowPos(dockPos);
    ImGui::SetNextWindowSize(dockSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                  ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("MainDockHost", nullptr, host_flags);
    ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None);

    if (firstTime)
    {
        firstTime = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, dockSize);

        ImGuiID dock_id_left, dock_id_right, dock_id_bottom, dock_id_center = dockspace_id;

        dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Down, 0.30f, nullptr, &dock_id_center);
        dock_id_left = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Left, 0.20f, nullptr, &dock_id_center);
        dock_id_right = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Right, 0.25f, nullptr, &dock_id_center);

        ImGui::DockBuilderDockWindow("Viewport", dock_id_center);
        ImGui::DockBuilderDockWindow("###UniqueEditorID", dock_id_center);
        ImGui::DockBuilderDockWindow("Scene", dock_id_left);
        ImGui::DockBuilderDockWindow("Properties", dock_id_right);
        ImGui::DockBuilderDockWindow("Assets", dock_id_bottom);
        ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
        ImGui::DockBuilderFinish(dockspace_id);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);

    if (m_luaEngine.isRunning)
        m_luaEngine.step();

    m_ribbon.renderRibbon();
    m_explorer.renderExplorer(m_viewport);

    // Bottom Status Bar
    {
        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->Pos.x, main_viewport->Pos.y + main_viewport->Size.y - 25.0f));
        ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, 25.0f));
        ImGuiWindowFlags status_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoFocusOnAppearing;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        
        ImGui::Begin("###StatusBar", nullptr, status_flags);
        ImGui::Text("Flux Engine | Version 0.1.0");
        if (!m_explorer.activeFolderPath.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled(" | Project: %s", m_explorer.activeFolderPath.filename().string().c_str());
        }
        ImGui::SameLine(main_viewport->Size.x - 30.0f);
        if (ImGui::Button("?", ImVec2(20.0f, 17.0f))) {
            Output::addLog("Flux Engine - Help: Use WASD + Mouse Right-Click to fly. Drag assets from the Assets panel into the scene to load them.");
        }
        ImGui::End();
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    static std::filesystem::path lastLoadedProject;
    if (m_explorer.activeFolderPath != lastLoadedProject && !m_explorer.activeFolderPath.empty())
    {
        lastLoadedProject = m_explorer.activeFolderPath;
        m_ribbon.LoadProjectSettings(m_explorer.activeFolderPath);
    }

    m_viewport.activeProjectPath = m_explorer.activeFolderPath;

    if (m_ribbon.editorLocked)
        ImGui::BeginDisabled(true);

    m_viewport.RenderViewport(m_heiarchy);
    m_properties.renderProperties(&m_heiarchy);
    m_heiarchy.renderHeiarchy(m_viewport.activeProjectPath);

    if (m_ribbon.editorLocked)
        ImGui::EndDisabled();

    if (m_ribbon.playToggledFrame)
    {
        if (m_ribbon.editorLocked)
            m_pendingStart = true;
        else
            m_pendingStop = true;
        m_ribbon.playToggledFrame = false;
    }

    m_output.renderOutput();

    bool isTextEditorFocused = false;

    if (m_explorer.isEditorVisible)
    {
        std::string editorTitle = "Text Editor - " + m_explorer.activeScriptName;
        if (m_explorer.isEditorUnsaved)
            editorTitle += " *";
        editorTitle += "###UniqueEditorID";

        ImGui::Begin(editorTitle.c_str(), &m_explorer.isEditorVisible);

        isTextEditorFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        if (isTextEditorFocused)
        {
            ImGuiIO &io = ImGui::GetIO();

            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
            {
                if (!m_explorer.activeFilePath.empty())
                {
                    std::ofstream outFile(m_explorer.activeFilePath);
                    if (outFile.is_open())
                    {
                        outFile << m_texteditor.GetText();
                        outFile.close();
                        m_explorer.isEditorUnsaved = false;
                        Output::addLog("Script saved.");
                    }
                }
            }
        }
        m_texteditor.Render("CodeEditorWidget");
        ImGui::End();
    }
    else
    {
        SDL_StopTextInput(m_window);
    }

    if (!isTextEditorFocused && !m_ribbon.editorLocked)
    {
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
        {
            m_ribbon.TriggerSaveScene();
        }
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            m_heiarchy.Undo();
        }
        else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y))
        {
            m_heiarchy.Redo();
        }
    }

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SwapWindow(m_window);

    //m_runtime.SyncCamera(m_viewport.camera->Position, m_viewport.camera->Position + m_viewport.camera->Front);
    m_runtime.Update();

    if (m_window && m_glContext)
        SDL_GL_MakeCurrent(m_window, m_glContext);
}

void Window::clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::StartRuntimeEngine()
{
    Output::addLog("Starting runtime engine...");

    std::string projName = m_explorer.projectRoot.name;
    if (projName == "Project" || projName.empty())
        projName = "Flux Game";

    auto &ps = m_ribbon.projectSettings;

    if (ps.useStartupScene)
    {
        std::filesystem::path scenePath = m_explorer.activeFolderPath / ps.startupScene;
        if (std::filesystem::exists(scenePath))
        {
            m_sceneSerializer.Load(m_heiarchy, scenePath, m_explorer.activeFolderPath);
            Output::addLog("Runtime loading startup scene: " + std::string(ps.startupScene));
        }
        else
        {
            Output::addLog("WARNING: Startup scene not found: " + scenePath.string() + ", using current hierarchy.");
        }
    }

    m_runtimeNodes = m_heiarchy.nodes;
    m_runtime.Start(projName, m_explorer.activeFolderPath, m_runtimeNodes, ps.runtimeWidth, ps.runtimeHeight);
}

void Window::StopRuntimeEngine()
{
    m_runtime.Stop();

    if (m_ribbon.luaEnginePtr)
        m_ribbon.luaEnginePtr->isRunning = false;

    m_ribbon.editorLocked = false;
    m_runtimeNodes.clear();

    Output::addLog("Runtime stopped.");
}
} // namespace Flux