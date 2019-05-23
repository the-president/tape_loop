#include <SDL2/SDL.h>
#include "frame_rate_manager.h"

frame_rate_manager::frame_rate_manager(Uint32 fps)
{
	frame_ticks = 1000/fps;
	ticks= SDL_GetTicks();
}

void frame_rate_manager::start()
{
	ticks= SDL_GetTicks();
}

void frame_rate_manager::delay()
{
	int delta = SDL_GetTicks() - ticks;

	if(delta < frame_ticks)
	{
		SDL_Delay(frame_ticks - delta);
	}
}
