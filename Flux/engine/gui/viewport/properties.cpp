#include "properties.h"

namespace Flux
{
	void Properties::renderProperties() {
		ImGui::Begin("Properties");
		ImGui::Text("This is the properties window. Here you will see details about the selected object and be able to edit them.");
		ImGui::End();
	}
}