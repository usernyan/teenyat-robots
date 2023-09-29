#include "timer.h"
#include "imgui.h"
#include <cmath>


void timer_state_gui(struct timer_state* ts) {
	ImGui::Checkbox("Paused", &ts->paused);
	ImGui::Checkbox("Single Tick", &ts->single_tick);
	const ImU64 u64_1 = 1;
	const ImU64 u64_30 = 30;
	ImGui::InputScalar("tick_ms", ImGuiDataType_U64, &ts->tick_ms, &u64_1, &u64_30, "%u", 0);
	ImGui::InputScalar("logic_per_tick", ImGuiDataType_U64, &ts->logic_per_tick, &u64_1, &u64_30, "%u", 0);
}

void Timer::Update() {
	/* logic_this_frame = ts.logic_per_tick; */
	if (!ts.paused || ts.single_tick) {
		uint64_t now = SDL_GetTicks64();
		if (now - prev_tick_time > ts.tick_ms || ts.single_tick) {
			ts.single_tick = false;
			prev_tick_time = now;
			tick_times++;
			logic_this_frame = ts.logic_per_tick;
			render_this_frame = true;
		}
	}
}

//loop through logic code until this returns false
bool Timer::DoLogic() {
	bool do_tick = false;
	if ( logic_this_frame > 0 ) {
		logic_this_frame--;
		logic_times++;
		do_tick = true;
	}
	return do_tick;
}

//loop through rendering code until this returns false
bool Timer::DoRender() {
	bool to_return = render_this_frame;
	render_this_frame = false;
	return to_return;
}

void Timer::GuiRender() {
	timer_state_gui(&ts);
}
