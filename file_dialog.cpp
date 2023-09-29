#include <filesystem>
#include <string>
#include <iostream>
#include "file_dialog.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "misc/cpp/imgui_stdlib.h"

namespace fs = std::filesystem;

FileDialog::FileDialog() {
	ChangeDir(fs::current_path());
	user_input_dir = cur_dir;
}

void FileDialog::ChangeDir(fs::path dir) {
	cur_dir = dir;
	user_input_dir = cur_dir.string();
}

bool FileDialog::ResetChoice() {
	bool r = user_choice_set;
	user_choice_set = false;
	return r;
}

void FileDialog::UpDir() {
	ChangeDir(cur_dir.parent_path());
}

void FileDialog::GuiRender() {
	if ( !visible )
		return;
	bool cont = ImGui::Begin("File Dialog", &visible);
	if (!cont) {
		ImGui::End();
		return;
	}
	bool enter_pressed = ImGui::InputText("dir", &user_input_dir,
		ImGuiInputTextFlags_EnterReturnsTrue|
		ImGuiInputTextFlags_AutoSelectAll
	);
	if (enter_pressed) {
		fs::path input_path = user_input_dir;
		if ( fs::is_directory(input_path) ) {
			cur_dir = input_path;
		}
	}

	if (ImGui::Button("^Up Dir^")) {
		UpDir();
	}

	if( ImGui::BeginTable("file table", 2)) {
		ImGui::TableNextColumn(); ImGui::Text("Filename");
		ImGui::TableNextColumn(); ImGui::Text("Actions");

		int id = 1;
		for (const auto &entry : fs::directory_iterator(cur_dir) ) {
			ImGui::PushID(id);
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( entry.path().filename().c_str() );
			ImGui::TableNextColumn();
			//TODO: implement stuff for symlinks and other entry types.
			if ( entry.is_directory()) {
				if (ImGui::Button("open")) {
					ChangeDir(entry.path());
				}
			}
			else if (entry.is_regular_file()) {
				if ( ImGui::Button("add robot") ) {
					user_choice = entry.path();
					user_choice_set = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("copy path")) {
					ImGui::SetClipboardText(entry.path().string().c_str());
				}
			}

			ImGui::PopID();
			id++;
		}
		ImGui::EndTable();
	}

	ImGui::End();
}
