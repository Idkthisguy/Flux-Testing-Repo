#include "heiarchy.h"
#include "core/pathHelper.h"
#include "logic/Textureloader.h"
#include "render/3D/OpenGL/Model.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Flux
{

static const char *NodeTypeLabel(NodeType t)
{
    switch (t)
    {
    case NodeType::DirectionalLight:
        return "[Dir]  ";
    case NodeType::PointLight:
        return "[Pt]   ";
    case NodeType::SpotLight:
        return "[Spot] ";
    case NodeType::SurfaceLight:
        return "[Surf] ";
    default:
        return "[Mesh] ";
    }
}

std::shared_ptr<Model> Heiarchy::GetOrLoadModel(const std::string &path)
{
    auto it = modelRegistry.find(path);
    if (it != modelRegistry.end())
        return it->second;
    auto m = std::make_shared<Model>(path);
    modelRegistry[path] = m;
    return m;
}

SceneNode *Heiarchy::GetLightingNode()
{
    for (auto &n : nodes)
        if (n.isLightingNode)
            return &n;
    return nullptr;
}

bool Heiarchy::isSelected(int index) const
{
    return std::find(selectedIndices.begin(), selectedIndices.end(), index) != selectedIndices.end();
}

void Heiarchy::selectNode(int index, bool ctrlDown, bool shiftDown)
{
    if (ctrlDown || shiftDown)
    {
        auto it = std::find(selectedIndices.begin(), selectedIndices.end(), index);

        if (it != selectedIndices.end())
            selectedIndices.erase(it);
        else
            selectedIndices.push_back(index);

        lastClickedIndex = index;
    }
    else if (shiftDown && lastClickedIndex != -1)
    {
        selectedIndices.clear();
        int start = std::min(lastClickedIndex, index);
        int end = std::max(lastClickedIndex, index);
        for (int i = start; i <= end; ++i)
            selectedIndices.push_back(i);
    }
    else
    {
        selectedIndices.clear();
        selectedIndices.push_back(index);
        lastClickedIndex = index;
    }
}

void Heiarchy::setup()
{
    SceneNode lighting;
    lighting.type = NodeType::DirectionalLight;
    lighting.name = "Lighting";
    lighting.isLightingNode = true;
    lighting.position = glm::vec3(0.0f, 10.0f, 0.0f);

    float pitch = -45.0f, yaw = 20.0f;
    lighting.rotation = glm::vec3(pitch, yaw, 0.0f);
    glm::quat q =
        glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0)) * glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
    lighting.light.direction = glm::normalize(q * glm::vec3(0.f, -1.f, 0.f));
    lighting.light.color = glm::vec3(1.0f, 0.97f, 0.88f);
    lighting.light.intensity = 3.5f;
    lighting.light.moonColor = glm::vec3(0.5f, 0.6f, 0.9f);
    lighting.light.moonIntensity = 1.0f;
    lighting.light.timeOfDay = 14.0f;
    lighting.light.brightness = 2.0f;

    std::string iconPath = PathHelper::GetAssetPath("assets/icons/l_dir.png");
    if (std::filesystem::exists(iconPath))
        lighting.textureID = TextureLoader::Load(iconPath);

    nodes.push_back(lighting);
    selectedIndices.clear();
    lastClickedIndex = -1;

    AddModel(PathHelper::GetAssetPath(std::string("assets/models/cube.obj")));

    undoStack.clear();
    redoStack.clear();
}

std::string Heiarchy::GetUniqueName(const std::string &baseName)
{
    std::string name = baseName;

    size_t openParen = name.find_last_of('(');
    size_t closeParen = name.find_last_of(')');
    if (openParen != std::string::npos && closeParen != std::string::npos && closeParen > openParen)
    {
        name = name.substr(0, openParen - 1);
    }

    std::string strippedBase = name;
    int counter = 1;

    for (;;)
    {
        bool clash = false;
        for (const auto &n : nodes)
        {
            if (n.name == name)
            {
                clash = true;
                break;
            }
        }
        if (!clash)
            break;
        name = strippedBase + " (" + std::to_string(counter++) + ")";
    }

    return name;
}

void Heiarchy::AddModel(const std::string &path, const std::string &name)
{
    PushUndoState();
    SceneNode n;
    n.type = NodeType::Mesh;
    n.model = GetOrLoadModel(path);
    std::string desired = name.empty() ? std::filesystem::path(path).stem().string() : name;
    n.name = GetUniqueName(desired);
    nodes.push_back(n);
    selectedIndices.clear();
    selectedIndices.push_back((int)nodes.size() - 1);
    lastClickedIndex = selectedIndices.back();
}

void Heiarchy::AddLight(NodeType type, const std::string &name)
{
    PushUndoState();
    SceneNode n;
    n.type = type;

    auto tryLoadIcon = [&](const char *rel) -> unsigned int {
        std::string p = PathHelper::GetAssetPath(rel);
        if (std::filesystem::exists(p))
            return TextureLoader::Load(p);
        return 0;
    };

    switch (type)
    {
    case NodeType::DirectionalLight:
        n.name = GetUniqueName(name.empty() ? "Directional Light" : name);
        n.textureID = tryLoadIcon("assets/icons/l_dir.png");
        break;
    case NodeType::PointLight:
        n.name = GetUniqueName(name.empty() ? "Point Light" : name);
        n.textureID = tryLoadIcon("assets/icons/l_point.png");
        break;
    case NodeType::SpotLight:
        n.name = GetUniqueName(name.empty() ? "Spot Light" : name);
        n.textureID = tryLoadIcon("assets/icons/l_spot.png");
        break;
    default:
        n.name = GetUniqueName(name.empty() ? "Surface Light" : name);
    }
    nodes.push_back(n);
    selectedIndices.clear();
    selectedIndices.push_back((int)nodes.size() - 1);
    lastClickedIndex = selectedIndices.back();
}

void Heiarchy::AddCamera(const std::string &name)
{
    PushUndoState();
    SceneNode n;
    n.type = NodeType::Camera;
    n.name = GetUniqueName(name.empty() ? "Camera" : name);

    bool hasMain = false;
    for (auto &node : nodes)
    {
        if (node.isMainCamera)
        {
            hasMain = true;
            break;
        }
    }
    if (!hasMain)
        n.isMainCamera = true;
    std::string iconPath = PathHelper::GetAssetPath("assets/icons/camera.png");
    if (std::filesystem::exists(iconPath))
        n.textureID = TextureLoader::Load(iconPath);

    nodes.push_back(n);
    selectedIndices.clear();
    selectedIndices.push_back((int)nodes.size() - 1);
    lastClickedIndex = selectedIndices.back();
}

void Heiarchy::DrawNode(int index)
{
    SceneNode &node = nodes[index];
    std::string uid = "##node" + std::to_string(index);

    if (renamingIndex == index && !node.isLightingNode)
    {
        ImGui::SetNextItemWidth(-1);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText(("##ren" + uid).c_str(), renameBuffer, sizeof(renameBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            if (renameBuffer[0] != '\0' && node.name != renameBuffer)
            {
                PushUndoState();
                node.name = renameBuffer;
            }
            renamingIndex = -1;
        }
        if (ImGui::IsItemDeactivated())
            renamingIndex = -1;
        return;
    }

    bool selected = isSelected(index);
    std::string label = std::string(NodeTypeLabel(node.type)) + node.name + uid;

    if (node.isLightingNode)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.35f, 1.0f));

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
    if (selected)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.55f));
    }

    ImGui::TreeNodeEx(label.c_str(), flags);

    if (selected)
        ImGui::PopStyleColor(2);
    if (node.isLightingNode)
        ImGui::PopStyleColor();

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        selectNode(index, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
    }

    if (!node.isLightingNode && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        renamingIndex = index;
        std::strncpy(renameBuffer, node.name.c_str(), sizeof(renameBuffer) - 1);
        renameBuffer[sizeof(renameBuffer) - 1] = '\0';
    }

    if (!node.isLightingNode)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {

            if (!selected)
                selectNode(index, false, false);

            ImGui::SetDragDropPayload("HIER_NODE", nullptr, 0);
            ImGui::Text("Moving %d items", (int)selectedIndices.size());
            ImGui::EndDragDropSource();
        }
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *p = ImGui::AcceptDragDropPayload("HIER_NODE"))
        {
            PushUndoState();

            std::vector<int> sortedSel = selectedIndices;
            std::sort(sortedSel.rbegin(), sortedSel.rend());

            std::vector<SceneNode> extracted;
            int insertAt = index;

            for (int i : sortedSel)
            {
                if (nodes[i].isLightingNode)
                    continue;
                extracted.push_back(nodes[i]);
                nodes.erase(nodes.begin() + i);
                if (i < insertAt)
                    insertAt--;
            }

            std::reverse(extracted.begin(), extracted.end());
            nodes.insert(nodes.begin() + insertAt, extracted.begin(), extracted.end());

            selectedIndices.clear();
            for (size_t i = 0; i < extracted.size(); ++i)
            {
                selectedIndices.push_back(insertAt + i);
            }
            lastClickedIndex = selectedIndices.empty() ? -1 : selectedIndices.back();
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem(("##ctx" + uid).c_str()))
    {
        if (!selected)
            selectNode(index, false, false);

        if (node.isLightingNode)
        {
            ImGui::TextDisabled("Lighting (locked)");
        }
        else
        {
            if (selectedIndices.size() == 1 && ImGui::MenuItem("Rename"))
            {
                renamingIndex = index;
                std::strncpy(renameBuffer, node.name.c_str(), sizeof(renameBuffer) - 1);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
            {
                PushUndoState();
                std::vector<int> toDelete = selectedIndices;
                std::sort(toDelete.rbegin(), toDelete.rend());
                for (int idx : toDelete)
                {
                    if (idx >= 0 && idx < nodes.size())
                        nodes.erase(nodes.begin() + idx);
                }
                selectedIndices.clear();
                lastClickedIndex = -1;
                ImGui::EndPopup();
                return;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Duplicate"))
            {
                PushUndoState();
                std::vector<int> newSelection;
                for (int idx : selectedIndices)
                {
                    if (idx >= 0 && idx < nodes.size() && !nodes[idx].isLightingNode)
                    {
                        SceneNode copy = nodes[idx];
                        copy.name = GetUniqueName(copy.name);
                        nodes.push_back(copy);
                        newSelection.push_back((int)nodes.size() - 1);
                    }
                }
                selectedIndices = newSelection;
                lastClickedIndex = newSelection.empty() ? -1 : newSelection.back();
            }
        }
        ImGui::EndPopup();
    }
}

void Heiarchy::renderHeiarchy(const std::filesystem::path &activeProjectPath)
{
    ImGui::Begin("Scene");
    if (ImGui::IsWindowHovered())
        ImGui::SetWindowFocus();

    ImGui::Text("Scene (Play)");
    ImGui::SameLine();

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float size = ImGui::GetFontSize() * 0.5f;
    pos.y += ImGui::GetFontSize() * 0.25f + 1.0f;
    pos.x += size * 0.5f;
    ImGui::GetWindowDrawList()->AddCircleFilled(pos, size * 0.5f, IM_COL32(255, 30, 30, 255));
    ImGui::Dummy(ImVec2(size + 5.0f, size));
    ImGui::Separator();

    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##hierSearch", "Search...", searchBuffer, sizeof(searchBuffer));
    ImGui::Separator();

    std::string searchStr = searchBuffer;
    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

    for (int i = 0; i < (int)nodes.size(); i++)
    {
        if (!searchStr.empty())
        {
            std::string nameLower = nodes[i].name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(searchStr) == std::string::npos)
            {
                continue;
            }
        }
        DrawNode(i);
    }

    float emptyH = std::max(ImGui::GetContentRegionAvail().y, 8.0f);
    ImGui::InvisibleButton("##hierEmpty", ImVec2(ImGui::GetContentRegionAvail().x, emptyH));

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *p = ImGui::AcceptDragDropPayload("EXPLORER_FILE"))
        {
            std::string path(static_cast<const char *>(p->Data));
            std::string ext = std::filesystem::path(path).extension().string();
            if (ext == ".obj" || ext == ".fbx")
                AddModel(path);
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem("##hierEmpty"))
    {
        if (ImGui::BeginMenu("Add Object"))
        {
            if (ImGui::MenuItem("Camera"))
            {
                AddCamera();
            }
            if (ImGui::BeginMenu("Mesh"))
            {
                auto tryAdd = [&](const char *rel, const char *addName) {
                    std::string full = PathHelper::GetAssetPath(std::string("assets/models/") + rel);
                    if (!activeProjectPath.empty())
                    {
                        auto c = activeProjectPath / "models" / rel;
                        if (std::filesystem::exists(c))
                            full = c.string();
                    }
                    AddModel(full, addName);
                };
                if (ImGui::MenuItem("Cube"))
                    tryAdd("cube.obj", "Cube");
                if (ImGui::MenuItem("Sphere"))
                    tryAdd("sphere.obj", "Sphere");
                if (ImGui::MenuItem("Monkey"))
                    tryAdd("monkey.obj", "Monkey");
                if (ImGui::MenuItem("Plane"))
                    tryAdd("plane.obj", "Plane");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Light"))
            {
                if (ImGui::MenuItem("Directional Light"))
                    AddLight(NodeType::DirectionalLight);
                if (ImGui::MenuItem("Point Light"))
                    AddLight(NodeType::PointLight);
                if (ImGui::MenuItem("Spot Light"))
                    AddLight(NodeType::SpotLight);
                if (ImGui::MenuItem("Surface Light"))
                    AddLight(NodeType::SurfaceLight);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void Heiarchy::PushUndoState()
{
    undoStack.push_back(nodes);
    if (undoStack.size() > 50)
    {
        undoStack.erase(undoStack.begin());
    }
    redoStack.clear();
}

void Heiarchy::Undo()
{
    if (undoStack.empty())
        return;
    redoStack.push_back(nodes);
    nodes = undoStack.back();
    undoStack.pop_back();
    selectedIndices.clear();
    lastClickedIndex = -1;
}

void Heiarchy::Redo()
{
    if (redoStack.empty())
        return;
    undoStack.push_back(nodes);
    nodes = redoStack.back();
    redoStack.pop_back();
    selectedIndices.clear();
    lastClickedIndex = -1;
}

} // namespace Flux