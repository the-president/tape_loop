#include "sdl_system.h"
#include "frame_rate_manager.h"
#include "audio.h"
#include "pdos_12.h"
#include "graphical_interface.h"
#include "screen.h"
#include "tape_lop_screen.h"
#include "audio_select_screen.h"

int main()
{
	sdl_system sys = sdl_system::init();

	if(sys.renderer == NULL || sys.window == NULL)
	{
		printf("sdl init failed\n");
		return -1;
	}

	frame_rate_manager frame_buddy(60);

	audio_system audio(48000,128);
	set_global_audio(&audio);

	SDL_Texture* font_text = pdos_12_info.load_font_texture(sys.renderer);

	if(font_text == NULL)
	{
		printf("font load failed\n");
		return 1;
	}

	graphical_interface::init_static_assets(sys.renderer);
	graphical_interface gui(sys.renderer,font_text,&pdos_12_info);

	tape_lop_screen lop_screen;
	audio_select_screen select_screen;

	select_screen.next_screen = &lop_screen;
	screen* current_screen = &select_screen;

	for(;;)
	{
		screen_result res = current_screen->loop(sys,audio,gui,frame_buddy);
		
		switch(res.type)
		{
			case QUIT:
				return 0;
			break;

			case NEW_SCREEN:
			{
				if(res.next == NULL)
				{
					printf("bad new screen");
					return 1;
				}

				current_screen = res.next;
			}
			break;
		}
	}

	sdl_system::quit(sys);
	return 0;
}
