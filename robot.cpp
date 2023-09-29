#include <string>
#include "robot.h"
#include "helpers.h"
#include "imgui.h"

Robot::Robot(int16_t r_id, std::string r_name) {
	id = r_id;
	name = r_name;
}

Robot::Robot(int16_t r_id, std::string r_name, color_t col, float p[2], int16_t ang) {
	id = r_id;
	name = r_name;
	InitPosition(p);
	InitAngle(ang);
	SetCol(col);
}

void Robot::GuiDebugMenu() {
	bool t = ImGui::TreeNode(name.c_str());
	if (t) {
		const ImS16 zero {0};
		const ImS16 one {1};
		const ImS16 hundred {100};
		const ImS16 neg_hundred {-100};
		const ImS16 max_angle {359};
		const ImS16 min_tur {-180};
		const ImS16 max_tur {179};
		ImGui::Checkbox("alive", &alive);
		ImGui::SliderScalar("armor", ImGuiDataType_S16, &armor, &zero, &hundred);
		ImGui::SliderFloat("heat", &heat, 0.0f, 100.0f);
		ImGui::InputFloat2("pos", pos);
		ImGui::SliderScalar("throttle", ImGuiDataType_S16, &throttle, &neg_hundred, &hundred);
		ImGui::SliderScalar("t_throttle", ImGuiDataType_S16, &t_throttle, &neg_hundred, &hundred);
		ImGui::SliderScalar("angle", ImGuiDataType_S16, &ang, &zero, &max_angle);
		ImGui::SliderScalar("t_angle", ImGuiDataType_S16, &t_ang, &zero, &max_angle);
		ImGui::SliderScalar("tur_offset", ImGuiDataType_S16, &tur_offset, &min_tur, &max_tur);
		ImGui::Checkbox("fire_missile", &fire_missile); ImGui::SameLine();
		ImGui::Checkbox("drop_mine", &drop_mine);
		ImGui::InputScalar("num_mines", ImGuiDataType_S16, &num_mines);
		ImGui::Checkbox("scan", &scan);
		ImGui::SliderScalar("scan_arc", ImGuiDataType_S16, &scan_arc, &one, &scan_arc_max);
		ImGui::SliderScalar("scan_len", ImGuiDataType_S16, &scan_len, &zero, &hundred);
		ImGui::InputScalar("num_targets", ImGuiDataType_S16, &num_targets);
		if( ImGui::ColorEdit4("color", fcol) ) {
			col.r = (int8_t)(fcol[0] * 0xFF);
			col.g = (int8_t)(fcol[1] * 0xFF);
			col.b = (int8_t)(fcol[2] * 0xFF);
			col.a = (int8_t)(fcol[3] * 0xFF);
		}
		ImGui::TreePop();
	}
}

void Robot::SetCol(color_t c) {
	col = c;
	fcol[0] = (float)col.r/0xFF;
	fcol[1] = (float)col.g/0xFF;
	fcol[2] = (float)col.b/0xFF;
	fcol[3] = (float)col.a/0xFF;
}

void Robot::InitPosition(float new_pos[2]) {
	pos[0] = new_pos[0];
	pos[1] = new_pos[1];
	t_pos[0] = new_pos[0];
	t_pos[1] = new_pos[1];
}

void Robot::InitAngle(uint16_t new_ang) {
	ang = new_ang;
	t_ang = new_ang;
	tur_ang = mod(ang + tur_offset, 360);
}

void Robot::Damage(int16_t dmg) {
	armor -= dmg;
	heat += heat_per_dmg * dmg;
}
