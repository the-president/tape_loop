#ifndef SCREEN_DOT_H
#define SCREEN_DOT_H

#include <SDL2/SDL.h>
#include "audio.h"
#include "sdl_system.h"
#include "frame_rate_manager.h"
#include "graphical_interface.h"

//gotta do up top declarations here
struct screen;

enum res_type
{
	QUIT =0,
	NEW_SCREEN
};

struct screen_result
{
	res_type type;
	screen* next;
};

struct screen
{
	virtual screen_result loop(sdl_system& sys, audio_system& audio,graphical_interface& gui, frame_rate_manager& frame_buddy) = 0;
	virtual ~screen() {};
};

#endif