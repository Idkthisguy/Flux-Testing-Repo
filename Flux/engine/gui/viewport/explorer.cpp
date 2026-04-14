#include "explorer.h"
#include "imgui.h"

namespace Flux {
	void Explorer::renderExplorer() {
		ImGui::Begin("Explorer");
		ImGui::Text("Scene Hierarchy Here");
		ImGui::End();
	}
}