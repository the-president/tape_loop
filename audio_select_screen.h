#ifndef AUDIO_SELECT_SCREEN_DOT_H
#define AUDIO_SELECT_SCREEN_DOT_H

#include <SDL2/SDL.h>
#include "screen.h"

struct audio_select_screen : public screen
{
	screen* next_screen;
	screen_result loop(sdl_system& sys, audio_system& audio,graphical_interface& gui, frame_rate_manager& frame_buddy);
};

#endif