#include <iostream>
#include <map>
#include <string>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#include "application.h"
#include "texture.h"

/*
 * The Application class handles the creation of windows and keeps a list of Textures, handling
 */

void Application::render_begin() {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
}

void Application::render_end() {
		ImVec4 clear_color = ImVec4(0, 25, 25, 255); //used by sdl. expressed differently from the colors used by imgui
        ImGui::Render();
        SDL_SetRenderDrawColor(this->rend, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(this->rend);
        //Render to the background screen with SDL here.
        //---
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(this->rend);
}

void Application::event_poll() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                this->done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(this->win))
                this->done = true;
        }
}

Application::Application() {
    sdl_init();
    imgui_init();
}

Application::~Application() {
    imgui_destroy();
    sdl_destroy();
}

void Application::sdl_init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cout << "Error: " << SDL_GetError() << "\n";
        std::terminate();
    }
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    win = SDL_CreateWindow("RAB Robots", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED |SDL_RENDERER_TARGETTEXTURE /*support for render targets*/);
    if (rend == NULL) {
        SDL_Log("Error creating SDL_Renderer!");
        std::terminate();
    }
}

void Application::imgui_init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(win, rend);
    ImGui_ImplSDLRenderer2_Init(rend);
}

void Application::sdl_destroy() {
	//delete all textures
    for (auto &kv: tex_map) { 
        delete kv.second;
    }
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
}

void Application::imgui_destroy() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
