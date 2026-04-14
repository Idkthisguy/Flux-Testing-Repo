#include "output.h"

namespace Flux {
	void Output::renderOutput() {
		ImGui::Begin("Output");
		ImGui::Text("This is the output window. Here you will see logs, errors, and other messages from the engine.");
		ImGui::End();
	}
}