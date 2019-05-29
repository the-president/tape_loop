#include <string>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <RtAudio.h>

#include "audio.h"
#include "audio_select_screen.h"
#include "graphical_interface.h"

// static void layout_helpers(graphical_interface& gui,int& panel_margin,int& line_height, int& panel_padding, int& panel_width, int& panel_gap)
// {
// 	gui.flat_label(20,480,"panel margin:");
// 	gui.int_bar(20,500,panel_margin,0,25);

// 	gui.flat_label(20,540,"line_height:");
// 	gui.int_bar(20,560,line_height,16,64);

// 	gui.flat_label(20,600,"panel padding:");
// 	gui.int_bar(20,620,panel_padding,0,30);

// 	gui.flat_label(20,660,"panel width:");
// 	gui.int_bar(20,680,panel_width,100,800);

// 	gui.flat_label(20,720,"panel gap:");
// 	gui.int_bar(20,740,panel_gap,10,100);
// }

screen_result audio_select_screen::loop(sdl_system& sys, audio_system& audio,graphical_interface& gui, frame_rate_manager& frame_buddy)
{
	SDL_Event e;

	int selected_input = -1;
	int selected_output= -1;

	int panel_margin=10;
	int line_height = 25;
	int panel_padding = 10;
	int panel_width = 600;
	int panel_gap = 20;
	int buffer_size = 64;
	char tmp_buff[64];

	for(;;)
	{
		frame_buddy.start();

		while( SDL_PollEvent( &e ) != 0 )
		{
			gui.process_event(e);

			if( e.type == SDL_QUIT )
			{
				return screen_result{QUIT,NULL};
			}
		}

		int device_count = audio.rt.getDeviceCount();
		int panel_height = (2*panel_margin) + (line_height * device_count);

		gui.start_frame();
		
		gui.flat_label(panel_padding,panel_padding,"INPUT DEVICE");
		gui.panel(panel_padding,panel_padding + line_height,panel_width,panel_height);
		{
			for(int i=0;i<device_count;++i)
			{
				RtAudio::DeviceInfo inf = audio.rt.getDeviceInfo(i);

				snprintf(tmp_buff,64,"%d %s - %d channels",i, inf.name.c_str(),inf.inputChannels);

				if(gui.select_button(panel_margin, panel_margin + (i*line_height), tmp_buff, selected_input == i ) && inf.inputChannels > 0)
				{
					selected_input = i;
				}
			}
			gui.panel_end();
		}
		gui.panel_end();

		gui.flat_label(panel_padding+ panel_width + panel_gap,panel_padding,"OUTPUT DEVICE");
		gui.panel(panel_padding+ panel_width + panel_gap,panel_padding + line_height,panel_width,panel_height);
		{
			for(int i=0;i<device_count;++i)
			{
				RtAudio::DeviceInfo inf = audio.rt.getDeviceInfo(i);

				snprintf(tmp_buff,64,"%d %s - %d channels",i, inf.name.c_str(),inf.outputChannels);

				if(gui.select_button(panel_margin, panel_margin + (i*line_height), tmp_buff, selected_output == i ) && inf.outputChannels > 0)
				{
					selected_output = i;
				}
			}
			gui.panel_end();
		}
		gui.panel_end();

		gui.flat_label(panel_padding,gui.bottom()+5,"buffer size");

		int button_line = gui.bottom()+5;

		if(gui.select_button(panel_padding,button_line,"64",buffer_size == 64))
		{
			buffer_size = 64;
		}

		if(gui.select_button(gui.right_end() + 5,button_line,"128",buffer_size == 128))
		{
			buffer_size = 128;
		}

		if(gui.select_button(gui.right_end() + 5,button_line,"256",buffer_size == 256))
		{
			buffer_size = 256;
		}

		if(gui.select_button(gui.right_end() + 5,button_line,"512",buffer_size == 512))
		{
			buffer_size = 512;
		}

		if(gui.select_button(gui.right_end() + 5,button_line,"1024",buffer_size == 1024))
		{
			buffer_size = 1024;
		}


		if(gui.button(panel_padding, gui.bottom() + 5,"done" ))
		{
			//set up the audio
			audio.buffer_size = buffer_size;
			audio.input_device_id = selected_input;
			audio.output_device_id = selected_output;

			audio.in_channels = audio.rt.getDeviceInfo(selected_input).inputChannels;
			audio.out_channels = audio.rt.getDeviceInfo(selected_output).outputChannels;

			return screen_result{NEW_SCREEN,next_screen};
		}

		gui.end_frame();
		frame_buddy.delay();
	}
}