/*
* Flux is a free, versatile game engine built for developers of all skill levels.
* Copyright (C) 2026  Zero Point Studio (Idkthisguy)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "explorer.h"
#include "imgui.h"

namespace Flux {
	void Explorer::renderExplorer(Viewport& viewport) {
		ImGui::Begin("Explorer");
        for (int i = 0; i < viewport.sceneObjects.size(); i++) {
            std::string label = viewport.sceneObjects[i].name;

            bool isSelected = (viewport.selectedObjectIndex == i);
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                viewport.selectedObjectIndex = i;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
		ImGui::End();
	}
}