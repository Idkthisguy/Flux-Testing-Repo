#include "ribbon.h"
#include "imgui.h"

namespace Flux {
	void Ribbon::renderRibbon() {
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(main_viewport->Pos);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, 55));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_MenuBar;

		ImGui::Begin("###Ribbon", nullptr, window_flags);

		if (ImGui::BeginMenuBar()) {
			drawFileMenu();
			drawEditMenu();
			ImGui::EndMenuBar();
		}

		ImGui::SetCursorPosX(main_viewport->Size.x * 0.5f - 50.0f);
		drawProjectControls();

		ImGui::End();
	}

	void Ribbon::drawFileMenu() {}
	void Ribbon::drawEditMenu() {}
	void Ribbon::drawProjectControls() {
		if (ImGui::Button("Play")) { /* Start Engine */ }
		ImGui::SameLine();
		if (ImGui::Button("Pause")) { /* Pause Engine */ }
	}
}