#ifndef ROBOTS_TEXTURE_H
#define ROBOTS_TEXTURE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

class Texture {
    public:
    Texture(SDL_Renderer* rend, Uint32 format, int access, int width, int height);
    Texture(SDL_Texture* t);
    Texture(SDL_Renderer* r, SDL_Surface* s, bool free_s);
    ~Texture();
    SDL_Texture* t {NULL};
	int w {0};
	int h {0};
};

#endif
