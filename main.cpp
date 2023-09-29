#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

#include "robot_vm/teenyat.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "misc/cpp/imgui_stdlib.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#include "application.h"
#include "texture.h"
#include "robot.h"
#include "robot_controller.h"
#include "robot_arena.h"
#include "timer.h"
#include "helpers.h"
#include "file_dialog.h"


struct env_settings {
	int size[2] = {500, 500};
	int tex_size[2] = {500, 500};
	struct timer_state ts_config;
};

struct robot_loadout_data {
	uint16_t id {0};
	std::string bin_file {""};
};

enum log_level {
	log_level_info,
	log_level_error,
};

struct log_entry {
	enum log_level lvl {log_level_info};
	std::string content;
};

//TODO: Implement something more clever so robots don't just clip into each other.
//The starting pos, angle, and color of robots is implemented as big lists of items for now.
void layout_robot(RobotArena *a, Robot *r, int player_num) {
	std::vector<std::vector<float>> pos_list = {
		{.1, .1},
		{.9, .9},
		{.9, .1},
		{.1, .9},
	};
	//The angle of the nth robot as a list of ints
	std::vector<uint16_t> ang_list = {
		45,
		225,
		135,
		315
	};
	//The color of the nth robot as a list of color_t's
	std::vector<color_t> col_list = {
		{0x00, 0xFF, 0x00, 0xFF},
		{0xFF, 0x00, 0x00, 0xFF},
		{0x00, 0x00, 0xFF, 0xFF},
		{0xFF, 0x00, 0xFF, 0xFF},
	};
	//use the golden ratio to make evenly distributed hues
	double PHI = (1.0 + std::sqrt(5)) / 2.0;
	double hue = player_num * (1.0/PHI);
	hue = std::fmod(hue, 1.0);

	//saturation between 0.5 and 1.0
	double sat = (player_num * 3) * (1.0/PHI);
	sat = std::fmod(sat, 1.0);
	sat = sat / 2 + 0.5;
	//value between 0.5 and 1.0
	double val = (player_num * 7) * (1.0/PHI);
	val = std::fmod(val, 1.0);
	val = val / 2 + 0.5;

	r->SetCol(hsv_to_color(hue, sat, val));
	/* r->SetCol(new_col); */
	//test positions (square grid)
	/* float v[2] = {0.2f + (player_num/10)*0.05f, 0.2f + 0.05f * (player_num % 10)}; */
	/* r->SetCol(col_list[player_num % col_list.size()]); */
	auto v = pos_list[player_num % pos_list.size()];
	float real_pos[2] = {v[0] * a->size[0], v[1] * a->size[1]};
	r->InitPosition(real_pos);
	r->InitAngle(ang_list[player_num % ang_list.size()]);
}


//Handles the creation and deletion of arenas, robots, controllers, and the gui
class Wrangler {
	public:
	Application *app;
	struct env_settings cur_settings;
	std::string user_file {""};
	std::list<struct robot_loadout_data *> robot_loadouts {};
	bool arena_active {false};
	RobotArena *arena {NULL};
	Texture *tex {NULL};
	Timer *tim {NULL};
	FileDialog fd;
	std::list<struct log_entry> log;
	Wrangler(Application *a);
	~Wrangler();
	void CreateArena();
	void DeleteArena();
	void Tick();
	void GuiRender();
	void GuiArena();
};

Wrangler::Wrangler(Application *a) {
	app = a;
}

Wrangler::~Wrangler() {
	DeleteArena();
	for (auto r : robot_loadouts) {
		delete r;
	}
}

void Wrangler::Tick() {
	if (!arena_active)
		return;
	tim->Update();
	while (tim->DoLogic())
		arena->LogicStep();
	while (tim->DoRender()) {
		arena->RenderToTex(app->rend, tex);
	}
}


//cpy'd from imgui_demo.cpp
//TODO: Make this constrain the _content area_ to a square size, not the whole window
static void SquareConstraint(ImGuiSizeCallbackData* data) { data->DesiredSize.x = data->DesiredSize.y = std::max(data->DesiredSize.x, data->DesiredSize.y); }

void Wrangler::GuiArena() {
	//TODO:
	//Give the window a decent default size.
	//Base the window's size constraints on the texture's size constraints, ideally keeping the content area's aspect ratio
	//to the aspect ratio of the arena's texture.
	ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), SquareConstraint);
	bool keep_open {true};
	if (ImGui::Begin("Arena", &keep_open)) {
		ImGui::Image((void*) tex->t, ImGui::GetContentRegionAvail());
	}
	ImGui::End();

	if( ImGui::Begin("Timer") ) {
		tim->GuiRender();
	}
	ImGui::End();

	if (ImGui::Begin("[DEBUG] gui control") ) {
		int i = 0;
		for(auto p : arena->robs) {
			auto r = std::get<1>(p);
			ImGui::PushID(i);
			r->GuiDebugMenu();
			ImGui::PopID();
			i++;
		}
	}
	ImGui::End();
	if (!keep_open) {
		DeleteArena();
	}
}

void Wrangler::GuiRender() {
	//Arena Creator
	ImGui::Begin("Arena Creator");

	ImGui::InputText("Bin File", &user_file);
	ImGui::SameLine();

	fd.visible = fd.visible != ImGui::Button("File Dialog");
	fd.GuiRender();
	bool add_from_dialog {false};
	std::string dialog_file;
	if ( fd.ResetChoice() ) {
		add_from_dialog = true;
		dialog_file = fd.user_choice.string();
	}


	if ( add_from_dialog || ImGui::Button("Add Robot") ) {
		//find lowest unused id
		std::vector<bool> id_used(robot_loadouts.size(), false);
		for (auto rl : robot_loadouts) {
			auto id = rl->id;
			if (id >= 0 && id < id_used.size()) {
				id_used[id] = true;
			}
		}
		int unused_id = 0;
		while ( unused_id < (int)id_used.size() && id_used[unused_id] ) {
			unused_id++;
		}
		//add robot loadout
		auto rl = new struct robot_loadout_data;
		rl->id = unused_id;
		if (add_from_dialog)
			rl->bin_file = dialog_file;
		else 
			rl->bin_file = user_file;
		robot_loadouts.push_back(rl);
	}
	ImGui::SameLine();

	if ( ImGui::Button("Clear Robots") ) {
		for (auto r : robot_loadouts) {
			delete r;
		}
		robot_loadouts.clear();
	}

	if ( ImGui::BeginTable("Robots:", 3) ) {
		ImGui::TableNextColumn(); ImGui::TextUnformatted("Robots:");
		ImGui::TableNextColumn(); ImGui::TextUnformatted("ID#");
		ImGui::TableNextColumn(); ImGui::TextUnformatted("File");

		int gui_id = 0;
		auto loadout = robot_loadouts.begin();
		while(loadout != robot_loadouts.end()) {
			auto rl = *loadout;
			ImGui::PushID(gui_id);
			ImGui::TableNextColumn();
			bool d = ImGui::Button("Delete");
			ImGui::TableNextColumn();
			ImGui::InputScalar("##id", ImGuiDataType_S16, &rl->id);
			ImGui::TableNextColumn();
			ImGui::InputText("##file", &rl->bin_file);
			ImGui::PopID();
			if (d) {
				delete rl;
				loadout = robot_loadouts.erase(loadout);
			}
			else {
				loadout++;
			}
			gui_id++;
		}
		ImGui::EndTable();
	}

	ImGui::Text("Arena Settings:");
	ImGui::InputInt2("size", cur_settings.size);
	cur_settings.tex_size[0] = cur_settings.size[0];
	cur_settings.tex_size[1] = cur_settings.size[1];

	ImGui::TextUnformatted("Timer State:");
	timer_state_gui(&cur_settings.ts_config);

	ImGui::TextUnformatted("");
	if (ImGui::Button("Create Arena")) {
		DeleteArena();
		CreateArena();
		//initialize the timer
		tim->ts = cur_settings.ts_config;
		//initialize
		for (auto rl : robot_loadouts) {
			if (!arena->IdTaken(rl->id)) {
				//insert robot
				auto new_rob = new Robot(rl->id, "Robot#" + std::to_string(rl->id));
				layout_robot(arena, new_rob, rl->id);
				arena->InsertRobot(new_rob);
				//connect controller
				auto *c = new RobotControllerTeenyAT();
				arena->ConnectController(new_rob->id, c);
				c->LoadFile(rl->bin_file.c_str());
			}
			else {
				log.push_back({
					log_level_error,
					"id #" + std::to_string(rl->id) + " already taken! Skipping bot..."
				});
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Arena")) {
		DeleteArena();
	}

	if (arena_active) {
		timer_state_gui(&tim->ts);
	}

	if (ImGui::CollapsingHeader("Log")) {
		if (ImGui::Button("Clear") ) {
			log.clear();
		}
		for ( auto s : log ) {
			auto color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			if (s.lvl == log_level_error) {
				color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			}
			ImGui::TextColored(color, s.content.c_str());
		}
	}

	ImGui::End();
	if (arena_active)
		GuiArena();
}

void Wrangler::CreateArena() {
	log.push_back({
		log_level_info,
		"Creating Arena."
	});
	arena = new RobotArena(cur_settings.size);
	tex = new Texture(app->rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
			cur_settings.tex_size[0], cur_settings.tex_size[1]);
	tim = new Timer();
	arena_active = true;
}
void Wrangler::DeleteArena() {
	if (!arena_active)
		return;
	log.push_back({
		log_level_info,
		"Deleting Arena."
	});
	//Delete the arena's robots and controllers here! The arena doesn't clean that up by itself.
	for (auto p : arena->robs) {
		auto r = std::get<1>(p);
		delete r;
	}
	for (auto p : arena->controllers) {
		auto c = std::get<1>(p);
		delete c;
	}
	delete arena;
	delete tex;
	delete tim;
	arena_active = false;
}


int main(int argc, char * argv[]) {
	//The Application class handles many aspects of rendering, including storing a list of Textures
	Application *app = new Application();


	Wrangler *w = new Wrangler(app);
	while (!app->done) {
		app->event_poll();
		//---- Game Logic and Rendering (SDL) here. ---

		w->Tick();
		
		//---- ImGui here ----
		//- Texture Debug -
		app->render_begin();

		w->GuiRender();

		app->render_end();
	}

	delete w;
	delete app;
	return 0;
}
