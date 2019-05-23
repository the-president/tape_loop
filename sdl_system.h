#ifndef SDL_SYSTEM_DOT_H
#define SDL_SYSTEM_DOT_H

#include <SDL2/SDL.h>

struct sdl_system
{
	const static int SCREEN_WIDTH;
	const static int SCREEN_HEIGHT;

	static sdl_system init();
	static void quit(sdl_system& sys);

	SDL_Window* window;
	SDL_Renderer* renderer;
};

#endif