#include <iostream>
#include <string>
#include <vector>
#include <map>
#define _USE_MATH_DEFINES //get M_PI
#include <cmath>
#include <cstdlib> //has int abs(int X)

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

Texture::Texture(SDL_Renderer* r, SDL_Surface* s, bool free_s) {
    t = SDL_CreateTextureFromSurface(r, s);
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    if (free_s) SDL_FreeSurface(s);
}

Texture::Texture(SDL_Renderer* rend, Uint32 format, int access, int width, int height) {
	t = SDL_CreateTexture(rend, format, access, width, height), w=width, h=height;
}

Texture::Texture(SDL_Texture* t) {
	this->t = t; SDL_QueryTexture(t, NULL, NULL, &w, &h);
}

Texture::~Texture() {
	SDL_DestroyTexture(t);
}
