#include "properties.h"
#include "heiarchy.h"
#include "logic/Textureloader.h"
#include "render/3D/OpenGL/Model.h"
#include <cstring>
#include <filesystem>
#include "output.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Flux {
static bool BeginTable2Col(const char* id = "##t") {
    if (ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings)) {
        ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 100.f);
        ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);
        return true;
    }
    return false;
}

static bool DragVec3Row(const char* label, glm::vec3& v, float speed = 0.1f, Heiarchy* h = nullptr) {
    float a[3] = { v.x, v.y, v.z };
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-1);
    bool c = ImGui::DragFloat3("##v", a, speed, -FLT_MAX, FLT_MAX, "%.3f");
    if (c) v = { a[0], a[1], a[2] };
    if (ImGui::IsItemDeactivatedAfterEdit() && h) h->PushUndoState();
    ImGui::PopID();
    return c;
}

static bool ColorRow(const char* label, glm::vec3& c, Heiarchy* h = nullptr) {
    float col[3] = { c.r, c.g, c.b };
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-1);
    bool ch = ImGui::ColorEdit3("##c", col, ImGuiColorEditFlags_Float);
    if (ch) c = { col[0], col[1], col[2] };
    if (ImGui::IsItemDeactivatedAfterEdit() && h) h->PushUndoState();
    ImGui::PopID();
    return ch;
}

static bool FloatRow(const char* label, float& f,
                     float spd = 0.01f, float mn = 0.f, float mx = FLT_MAX,
                     const char* fmt = "%.3f",
                     ImGuiSliderFlags flags = 0, Heiarchy* h = nullptr) {
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-1);
    bool c = ImGui::DragFloat("##f", &f, spd, mn, mx, fmt);
    if (ImGui::IsItemDeactivatedAfterEdit() && h) h->PushUndoState();
    ImGui::PopID();
    return c;
}

static bool SliderRow(const char* label, float& f,
                      float mn = 0.f, float mx = 1.0f,
                      const char* fmt = "%.2f", Heiarchy* h = nullptr) {
    ImGui::PushID(label);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-1);
    bool c = ImGui::SliderFloat("##s", &f, mn, mx, fmt);
    if (ImGui::IsItemDeactivatedAfterEdit() && h) h->PushUndoState();
    ImGui::PopID();
    return c;
}

static void TextureSlot(const char* label, SceneNode& node, Heiarchy* h = nullptr) {
    float avail = ImGui::GetContentRegionAvail().x;
    float swatchH = 56.f;

    ImGui::Spacing();
    ImGui::TextUnformatted(label);

    ImGui::PushID(label);
    if (node.textureID != 0) {
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(node.textureID)),
                     ImVec2(avail - 60.f, swatchH), ImVec2(0,1), ImVec2(1,0));
    } else {
        float col[3] = { node.baseColor.r, node.baseColor.g, node.baseColor.b };
        ImGui::SetNextItemWidth(avail - 60.f);
        if (ImGui::ColorButton("##bc", ImVec4(col[0], col[1], col[2], 1.f),
                ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
                ImVec2(avail - 60.f, swatchH)))
        {
            ImGui::OpenPopup("##bcPicker");
        }
        if (ImGui::BeginPopup("##bcPicker")) {
            bool ch = ImGui::ColorPicker3("##bcp", col, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoSidePreview);
            if (ch) {
                node.baseColor = { col[0], col[1], col[2] };
            }
            if (ImGui::IsItemDeactivatedAfterEdit() && h) h->PushUndoState();
            ImGui::EndPopup();
        }
    }

    ImVec2 dropMin = ImGui::GetItemRectMin();
    ImVec2 dropMax = ImGui::GetItemRectMax();

    ImGui::SameLine();
    if (!node.texturePath.empty()) {
        if (ImGui::SmallButton("X##clr")) {
            if (h) h->PushUndoState();
            node.texturePath = "";
            node.textureID   = 0;
            if (node.model) node.model->SetTexture(0);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clear texture");
    } else {
        ImGui::TextDisabled(" drop\n here");
    }

    ImGui::SetCursorScreenPos(dropMin);
    ImGui::InvisibleButton("##texDrop", ImVec2(dropMax.x - dropMin.x, dropMax.y - dropMin.y));
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("EXPLORER_FILE")) {
            std::string droppedPath(static_cast<const char*>(p->Data));
            std::string ext = std::filesystem::path(droppedPath).extension().string();
            if (ext==".png"||ext==".jpg"||ext==".jpeg"||ext==".bmp"||ext==".tga") {
                if (h) h->PushUndoState();
                node.texturePath = droppedPath;
                node.textureID = TextureLoader::Load(droppedPath);
                if (node.model) node.model->SetTexture(node.textureID);
            }
        }
        ImGui::EndDragDropTarget();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        ImGui::GetWindowDrawList()->AddRect(dropMin, dropMax,
            IM_COL32(80, 160, 255, 200), 3.f, 0, 2.f);

    if (!node.texturePath.empty())
        ImGui::TextDisabled("%s", std::filesystem::path(node.texturePath).filename().string().c_str());

    ImGui::PopID();
}

static glm::vec3 DirToEuler(glm::vec3 dir) {
    dir = glm::normalize(dir);
    float pitch = glm::degrees(std::asin(-dir.y));
    float yaw   = glm::degrees(std::atan2(dir.x, -dir.z));
    return glm::vec3(pitch, yaw, 0.f);
}

static void DrawProfileAndStats(Heiarchy* h) {
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Profile & Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        float fps = ImGui::GetIO().Framerate;
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("RAM: 3.71 GB");
        
        int totalDrawCalls = 0;
        int totalVertices = 0;
        if (h) {
            for (const auto& n : h->nodes) {
                if (n.model) {
                    totalDrawCalls += (int)n.model->meshes.size();
                    for (const auto& mesh : n.model->meshes) {
                        totalVertices += (int)mesh.verticies.size();
                    }
                }
            }
        }
        ImGui::Text("Draw Calls: %d", totalDrawCalls);
        ImGui::Text("Vertices: %d", totalVertices);
    }
}

void Properties::renderProperties(Heiarchy* h) {
    ImGui::Begin("Properties");
    if (ImGui::IsWindowHovered()) ImGui::SetWindowFocus();

    if (!h || h->selectedIndex < 0 || h->selectedIndex >= (int)h->nodes.size()) {
        ImGui::TextDisabled("No object selected.");
    } else {
        SceneNode& node = h->nodes[h->selectedIndex];

        if (node.isLightingNode) {
            ImGui::TextColored(ImVec4(1.f, 0.85f, 0.35f, 1.f), "Lighting  [locked]");
        } else {
            char nameBuf[128];
            std::strncpy(nameBuf, node.name.c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
                node.name = nameBuf;
        }
        ImGui::Separator();

        if (node.isLightingNode) {
            ImGui::Text("Lighting Properties");
            ImGui::Spacing();

            if (BeginTable2Col("##t_lighting"))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Time of Day");
                ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-1);
                int   h24   = (int)node.light.timeOfDay;
                int   m60   = (int)((node.light.timeOfDay - h24) * 60.f);
                char  disp[16]; snprintf(disp,  sizeof(disp), "%02d:%02d", h24, m60);
                ImGui::PushID("tod");
                if (ImGui::DragFloat("##tod", &node.light.timeOfDay, 0.01f, 0.f, 24.f, disp)) {
                    float angle = (node.light.timeOfDay - 12.0f) * 15.0f; 
                    node.rotation.x = angle;
                    
                    glm::quat q = glm::angleAxis(glm::radians(node.rotation.y), glm::vec3(0,1,0))
                                * glm::angleAxis(glm::radians(node.rotation.x), glm::vec3(1,0,0));
                    node.light.direction = glm::normalize(q * glm::vec3(0.f, -1.f, 0.f));
                }
                ImGui::PopID();
            }

            FloatRow("Brightness",      node.light.brightness,     0.01f, 0.f, 10.f, "%.3f", 0, h);
            ColorRow("Color",           node.light.color, h);
            ColorRow("Moon Color",      node.light.moonColor, h);
            FloatRow("Moon Intensity",  node.light.moonIntensity, 0.01f, 0.f, 100.f, "%.3f", 0, h);
            ColorRow("ColorShift",      node.light.colorShift, h);
            FloatRow("Ambient Day",     node.light.ambientDaytime, 0.005f, 0.f, 1.f, "%.3f", 0, h);
            FloatRow("Ambient Night",   node.light.ambientNight,   0.005f, 0.f, 1.f, "%.3f", 0, h);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Direction");
            ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-1);
            {
                float arr[3] = { node.light.direction.x, node.light.direction.y, node.light.direction.z };
                ImGui::PushID("ldir");
                if (ImGui::DragFloat3("##ld", arr, 0.005f, -1.f, 1.f, "%.3f")) {
                    node.light.direction = glm::normalize(glm::vec3(arr[0], arr[1], arr[2]));
                    node.rotation = DirToEuler(node.light.direction);
                }
                if (ImGui::IsItemDeactivatedAfterEdit() && h) h->PushUndoState();
                ImGui::PopID();
            }

            ImGui::Separator();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Spacing();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Fog Color");
            ImGui::TableSetColumnIndex(1); /* handled below */
            ImGui::EndTable();

            if (BeginTable2Col("##t_lighting_fog")) {
                ColorRow("Fog Color",  node.light.fogColor, h);
                FloatRow("Fog Start",  node.light.fogStart, 1.f, 0.f, 5000.f, "%.3f", 0, h);
                FloatRow("Fog End",    node.light.fogEnd,   1.f, 0.f, 5000.f, "%.3f", 0, h);
                ImGui::EndTable();
            }
        }
        else {
            ImGui::Text("Transform");
            ImGui::Spacing();
            if (BeginTable2Col("##t_transform")) {
                DragVec3Row("Position", node.position, 0.1f, h);
                DragVec3Row("Rotation", node.rotation, 0.5f, h);
                DragVec3Row("Scale",    node.scale,    0.01f, h);
                ImGui::EndTable();
            }

            if (node.type == NodeType::Camera) {
                ImGui::Separator();
                ImGui::Text("Camera Settings");
                ImGui::Spacing();
                
                if (ImGui::Checkbox("Main Camera", &node.isMainCamera)) {
                    h->PushUndoState();
                    if (node.isMainCamera) {
                        for (auto& otherNode : h->nodes) {
                            if (&otherNode != &node) otherNode.isMainCamera = false;
                        }
                    }
                }

                if (BeginTable2Col("##t_FOV")) {
                    SliderRow("FOV", node.fov, 10.0f, 170.0f, "%.1f°", h);
                    ImGui::EndTable();
                }
            }

            if (node.type == NodeType::Mesh) {
                ImGui::Separator();
                ImGui::Text("Surface");
                ImGui::Spacing();

                if (node.model)
                    ImGui::TextDisabled("  %s", node.model->path.c_str());
                ImGui::Separator();
                TextureSlot("Albedo", node, h);

                ImGui::Separator();
                ImGui::Text("Physics");
                ImGui::Spacing();

                if (BeginTable2Col("##t_physics")) {

                    DragVec3Row("Velocity", node.velocity, 0.1f, h);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::AlignTextToFramePadding(); 
                    ImGui::TextUnformatted("Anchored");

                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::Checkbox("###anchored", &node.isAnchored)) {
                        h->PushUndoState();
                    }

                    ImGui::EndTable();
                }
                ImGui::Separator();
                ImGui::Text("Material");
                ImGui::Spacing();
                if (BeginTable2Col("##t_material")) {
                    SliderRow("Roughness", node.roughness, 0.0f, 1.0f, "%.2f", h);
                    SliderRow("Metallic",  node.metallic,  0.0f, 1.0f, "%.2f", h);
                    ImGui::EndTable();
                }
                if (!node.texturePath.empty()) {
                    ImGui::Separator();
                    ImGui::Text("Texture Settings");
                    ImGui::Spacing();
                    if (BeginTable2Col("##t_texstretch")) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Tiling U/V");
                        ImGui::TableSetColumnIndex(1); ImGui::SetNextItemWidth(-1);
                        if (ImGui::DragFloat2("##ts", glm::value_ptr(node.textureScale), 0.01f, 0.01f, 100.0f, "%.2f")) {
                        }
                        if (ImGui::IsItemDeactivatedAfterEdit() && h) h->PushUndoState();

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Pixelated");
                        ImGui::TableSetColumnIndex(1);
                        if (ImGui::Checkbox("##pixelated", &node.pixelated)) {
                            if (h) h->PushUndoState();
                        }
                        ImGui::EndTable();
                    }
                }
            }
            else {
                ImGui::Separator();
                const char* lbl =
                    node.type == NodeType::DirectionalLight ? "Directional Light" :
                    node.type == NodeType::PointLight       ? "Point Light"       :
                    node.type == NodeType::SpotLight        ? "Spot Light"        : "Surface Light";
                ImGui::Text("%s", lbl);
                ImGui::Spacing();

                BeginTable2Col("##t_texture");
                ColorRow("Color",     node.light.color, h);
                FloatRow("Intensity", node.light.intensity, 0.01f, 0.f, 100.f, "%.3f", 0, h);


                if (node.type == NodeType::DirectionalLight) {
                    if (DragVec3Row("Direction", node.light.direction, 0.01f, h)) {
                        node.light.direction = glm::normalize(node.light.direction);
                        node.rotation = DirToEuler(node.light.direction);
                    }
                }
                if (node.type == NodeType::PointLight)
                    FloatRow("Range", node.light.range, 0.1f, 0.f, 1000.f, "%.3f", 0, h);

                if (node.type == NodeType::SpotLight) {
                    if (DragVec3Row("Direction", node.light.direction, 0.01f, h)) {
                        node.light.direction = glm::normalize(node.light.direction);
                        glm::vec3 e = DirToEuler(node.light.direction);
                        node.rotation.x = e.x; node.rotation.y = e.y;
                    }
                    FloatRow("Range",        node.light.range,       0.1f, 0.f, 1000.f, "%.3f", 0, h);
                    FloatRow("Inner Cutoff", node.light.innerCutoff, 0.1f, 0.f, 90.f, "%.3f", 0, h);
                    FloatRow("Outer Cutoff", node.light.outerCutoff, 0.1f, 0.f, 90.f, "%.3f", 0, h);
                }
                if (node.type == NodeType::SurfaceLight) {
                    FloatRow("Area Width",  node.light.areaWidth,  0.01f, 0.f, 100.f, "%.3f", 0, h);
                    FloatRow("Area Height", node.light.areaHeight, 0.01f, 0.f, 100.f, "%.3f", 0, h);
                }
                ImGui::EndTable();
            }
        }
        DrawProfileAndStats(h);
    }

    ImGui::End();
}
}