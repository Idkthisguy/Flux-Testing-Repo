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

namespace Flux
{
	void Explorer::renderExplorer(Viewport& viewport) {
		ImGui::Begin("Explorer");

		if (ImGui::IsWindowHovered()) {
			ImGui::SetWindowFocus();
		}

		if (ImGui::TreeNodeEx("Project Files", ImGuiTreeNodeFlags_DefaultOpen)) {

			DrawVirtualNodes(projectRoot);

			ImGui::TreePop();
		}

		if (ImGui::IsWindowFocused() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
			if (ImGui::GetIO().MouseDragMaxDistanceSqr[ImGuiMouseButton_Right] < 10.0f) {
				ImGui::OpenPopup("ContextMenuExplorer");
			}
		}

		if (ImGui::BeginPopup("ContextMenuExplorer")) {
			if (ImGui::MenuItem("Create a new Project"))
			{
				char* usrProfile = std::getenv("USERPROFILE");
				if (usrProfile)
				{
					std::filesystem::path docsPath = std::filesystem::path(usrProfile) / "Documents" / "FluxProjects" / "NewGame";

					std::filesystem::path searchPath = std::filesystem::current_path();
					std::filesystem::path absoluteTemplate;
					bool found = false;

					for (int i = 0; i < 5; ++i) {
						if (std::filesystem::exists(searchPath / "templates" / "base_game_folder")) {
							absoluteTemplate = searchPath / "templates" / "base_game_folder";
							found = true;
							break;
						}
						if (searchPath.has_parent_path()) {
							searchPath = searchPath.parent_path();
						} else {
							break;
						}
					}

					if (found) {
						try {
							if (std::filesystem::exists(docsPath)) {
								std::filesystem::remove_all(docsPath);
							}

							std::filesystem::create_directories(docsPath);

							std::filesystem::copy(absoluteTemplate, docsPath, std::filesystem::copy_options::recursive);

							this->activeFolderPath = docsPath;

							projectRoot.name = docsPath.filename().string();
							syncFiles(docsPath, projectRoot);

							std::cout << "SUCCESS! found templates at: " << absoluteTemplate << std::endl;
							std::cout << "Project created at: " << docsPath << std::endl;

						} catch (const std::filesystem::filesystem_error& e) {
							std::cerr << "COPY FAILED: " << e.what() << std::endl;
						}
					} else {
						std::cerr << "ERROR: Could not find 'templates/base_game_folder' anywhere above " << std::filesystem::current_path() << std::endl;
					}
				}
			}

			if (ImGui::BeginMenu("Add.."))
			{
				if (ImGui::MenuItem("Add Folder")) {
					std::cout << "gmmm" << std::endl;
					std::filesystem::path newPath = activeFolderPath;
					std::filesystem::create_directory(newPath);
					syncFiles(activeFolderPath, projectRoot);
					std::cout << "HEHEHEHEHAWUODHWADIOJAWDIAWBUIDAHWODOJAWHOWAFOAEWOBUFAUOFBUOAEWJOFAEBJOFAEKBJFAEBKJGBAKJEGBJASEGF" << std::endl;
				}
				if (ImGui::MenuItem("Add Script")) {
					createNewFile("NewScript", ".lua");
				}
				if (ImGui::MenuItem("Add Text")) {
					createNewFile("NewTextFile", ".txt");
				}
				if (ImGui::MenuItem("Add Model"))
				{
					projectRoot.children.push_back({ "New Model", fileType::Model });
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}
		ImGui::End();

		if (!pathToDelete.empty()) {
			try {
				if (std::filesystem::exists(pathToDelete)) {
					std::filesystem::remove_all(pathToDelete);
					// Now we refresh the whole list safely
					syncFiles(activeFolderPath, projectRoot);
					std::cout << "Chef successfully cleared the station." << std::endl;
				}
			} catch (const std::filesystem::filesystem_error& e) {
				std::cerr << "Executioner failed: " << e.what() << std::endl;
			}
			pathToDelete = ""; // Reset the hit list
		}
	}

	void Explorer::DrawVirtualNodes(virtualFile& file) {
		std::string uniqueID = file.name + "##" + std::to_string((uintptr_t)&file);

		if (file.type == fileType::Folder) {
			bool node_open = ImGui::TreeNodeEx(uniqueID.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete Folder"))
				{
					this->pathToDelete = file.path;
				}
				ImGui::EndPopup();
			}

			if (node_open) {
				for (auto& child : file.children) {
					DrawVirtualNodes(child); // The recursive loop
				}
				ImGui::TreePop();
			}
		} else {
			ImGui::Selectable(uniqueID.c_str());

			if (ImGui::BeginPopupContextItem())
			{
				this->pathToDelete = file.path;
				ImGui::EndPopup();
			}

		}
	}

	void Explorer::syncFiles(const std::filesystem::path& path, virtualFile& node)
	{
		node.children.clear();
		node.path = path;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			virtualFile child;
			child.name = entry.path().filename().string();
			child.path = entry.path();

			if (entry.is_directory())
			{
				child.type = fileType::Folder;
				syncFiles(entry.path(), child);
			} else
			{
				std::string ext = entry.path().extension().string();
				if (ext == ".lua") child.type = fileType::Script;
				else if (ext == ".txt") child.type = fileType::Text;
				else if (ext == ".obj" || ext == ".fbx") child.type = fileType::Model;
				else child.type = fileType::Text;
			}

			node.children.push_back(child);
		}
	}

	void Explorer::createNewFile(const std::string& name, const std::string& ext)
	{
		std::filesystem::path fullPath = activeFolderPath / (name + ext);

		int counter = 1;
		while (std::filesystem::exists(fullPath))
		{
			fullPath = activeFolderPath / (name + " (" + std::to_string(counter) + ")" + ext);
		}

		std::ofstream file(fullPath);
		file.close();

		syncFiles(activeFolderPath, projectRoot);
	}
}
