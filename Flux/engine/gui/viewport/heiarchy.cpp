#include "heiarchy.h"
#include "viewport.h"
#include "imgui.h"

namespace Flux
{
    void Heiarchy::renderHeiarchy(Viewport& viewport)
    {
        ImGui::Begin("Scene Heiarchy");

        for (int i = 0; i < (int)viewport.sceneObjects.size(); i++)
        {
            if (viewport.sceneObjects[i].parentIndex == -1) {
                DrawSceneNode(i, viewport);
            }
        }

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::BeginMenu("Add Model"))
            {
                if (ImGui::MenuItem("Cube"))
                {

                }
                ImGui::EndMenu();
            }

            ImGui::Separator(); // Draw a thin line

            if (ImGui::MenuItem("Create Folder"))
            {
                GameObject newFolder;
                newFolder.name = "New Folder";
                newFolder.modelBlueprint = nullptr; // no model = folder/empty object
                viewport.sceneObjects.push_back(newFolder);
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void Heiarchy::DrawSceneNode(int index, Viewport& viewport) {
        auto& obj = viewport.sceneObjects[index];
        ImGui::PushID(index);

        bool isSelected = (viewport.selectedObjectIndex == index);

        std::string displayName = (obj.modelBlueprint == nullptr) ? "[DIR] " + obj.name : obj.name;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
        if (obj.children.empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (isSelected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool node_open = ImGui::TreeNodeEx(displayName.c_str(), flags);

        // Click to select
        if (ImGui::IsItemClicked()) {
            viewport.selectedObjectIndex = index;
        }

        if (ImGui::BeginDragDropSource()) {
            // We pack the INDEX number into the truck, not the path!
            ImGui::SetDragDropPayload("SCENE_OBJ_INDEX", &index, sizeof(int));
            ImGui::Text("Moving %s", obj.name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_OBJ_INDEX")) {
                int draggedIndex = *(const int*)payload->Data;

                if (draggedIndex != index) {
                    int oldParent = viewport.sceneObjects[draggedIndex].parentIndex;
                    if (oldParent != -1) {
                        auto& oldKids = viewport.sceneObjects[oldParent].children;
                        oldKids.erase(std::remove(oldKids.begin(), oldKids.end(), draggedIndex), oldKids.end());
                    }

                    viewport.sceneObjects[draggedIndex].parentIndex = index;
                    obj.children.push_back(draggedIndex);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (node_open) {
            for (int childIndex : obj.children) {
                DrawSceneNode(childIndex, viewport);
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}
