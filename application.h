#ifndef ROBOTS_APPLICATION_H
#define ROBOTS_APPLICATION_H
#include "texture.h"

class Application {
    public:
    Application();
    ~Application();
    void sdl_init();
    void imgui_init();
    void sdl_destroy();
    void imgui_destroy();
    ImGuiIO* io;
    SDL_Window* win;
    SDL_Renderer* rend;
    std::map<std::string, Texture*> tex_map;
	void event_poll();
	bool done { false };
	void render_begin();
	void render_end();
};
#endif
