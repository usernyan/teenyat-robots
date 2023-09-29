#ifndef TIMER_H
#define TIMERH

#include <cstdint>
#include <SDL2/SDL.h>

struct timer_state {
    bool paused {false};
    uint64_t tick_ms {10};
    uint64_t logic_per_tick {1};
    bool single_tick {false};
};

void timer_state_gui(struct timer_state* ts);

class Timer {
	public:
	//control variables
    uint64_t prev_tick_time {0};
	/* struct timer_state ts; */
	struct timer_state ts;
	//control variables
	uint64_t logic_this_frame {0};
	bool render_this_frame {true};

	//just statistics
	uint64_t tick_times {0};
	uint64_t logic_times {0};

	//The Tick func should be called in a loop
	//after the Update func every frame
	void Update();
	bool DoLogic();
	bool DoRender();
	void GuiRender();
};
#endif
