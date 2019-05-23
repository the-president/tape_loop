#include <cmath>
#include <stdio.h>
#include "tape_lop_screen.h"

//some dumb layout consts
static int panel_margin = 2;
static int panel_padding = 10;
// static int panel_width = 1278;
// static int panel_height = 400;

//1280w / 960h
void num_bar(graphical_interface& gui,int x,int y,int& val,int min,int max,const char* label)
{
	const int text_center_y = y + 7; //15-8
	gui.flat_label(x,text_center_y,label);

	gui.int_bar(gui.right_end()+2,y,val,min,max);
	
	if(gui.fs_button(gui.right_end()+2,y,25,30,"+"))
	{
		val++;
	}
	
	if(gui.fs_button(gui.right_end()+2,y,25,30,"-"))
	{
		val--;
	}	
}

float tape_lop_screen::seconds()
{
	float seconds_per_beat = 60.0/bpm;
	return 4 * bars * seconds_per_beat;
}

int tape_lop_screen::samples(unsigned int sample_rate)
{
	return std::floor(sample_rate * seconds());
}

void tape_lop_screen::top_panel(graphical_interface& gui,audio_system& audio,int& scroll, int gap)
{
	gui.panel(panel_margin,panel_margin,1278,30+(2*panel_padding));
	{
		if(gui.fs_button(panel_padding,panel_padding,80,30,"add loop"))
		{
			add_loop(audio.sample_rate,audio.in_channels,audio.out_channels);
		}

		int bar_zone_x = gui.right_end()+gap;
		int bar_zone_y = gui.top();

		if(gui.fs_button(gui.right_end() + 5, gui.top(),30,30,"<") && scroll != 0)
		{
			scroll--;
		}
		if(gui.fs_button(gui.right_end() + 5, gui.top(),30,30,">"))
		{
			scroll++;	
		}

		int button_top = gui.top();

		gui.int_label(gui.right_end() + 5,gui.center_y() - 8, "scroll",scroll);

		if(gui.fs_button(gui.right_end() + 60, button_top,70,30,"SYNC >"))
		{
			for(tape_lop& tl : loops)
			{
				tl.write_head = 0;
				tl.write_head_fraction = 0;
				tl.play = true;
			}
		}

		if(gui.fs_button(gui.right_end() + 10, button_top,70,30,"SYNC []"))
		{
			for(tape_lop& tl : loops)
			{
				tl.play = false;
				tl.write_head = 0;
				tl.write_head_fraction =0;
			}
		}

		num_bar(gui,bar_zone_x,bar_zone_y,bpm,10,360,"B.P.M.");
		num_bar(gui,gui.right_end() + 20,gui.top(),bars,1,16,"Bars");

		float secs = seconds();
		int samps = samples(audio.sample_rate);
		int label_line = gui.right_end() + 20;
		gui.float_label(label_line,30,"seconds: ",secs);
		gui.int_label(label_line,7,"samples: ",samps);
	}
	gui.panel_end();
}

void tape_lop_screen::add_loop(unsigned int sample_rate,int inputs,int outputs)
{
	loops.emplace_back(inputs,outputs,samples(sample_rate));
}

void tape_lop_screen::drop_loop(int index)
{
	auto pos = loops.begin() + index;
	loops.erase(pos);
}

screen_result tape_lop_screen::loop(sdl_system& sys, audio_system& audio,graphical_interface& gui,frame_rate_manager& frame_buddy)
{
	SDL_Event e;

	audio.aguy.store(this);

	audio.open_stream();
	audio.start();

	gui.reset_state();

	//clean out the backbuff
	SDL_SetRenderDrawColor(sys.renderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(sys.renderer);

	SDL_RenderPresent(sys.renderer);

	int scroll = 0;

	for(;;)
	{
		frame_buddy.start();

		while( SDL_PollEvent( &e ) != 0 )
		{
			gui.process_event(e);

			//User requests quit
			if( e.type == SDL_QUIT )
			{
				audio.stop();
				audio.aguy = NULL;
				return screen_result{QUIT,NULL};
			}
		}

		gui.start_frame();
		top_panel(gui,audio,scroll);

		int fill = 0;
		
		for(int i=0+scroll ; fill < 4 && i<loops.size();++i)
		{
			int x = gui.left_end();
			int y = gui.bottom() + 10;
			tape_lop& loop = loops.at(i);
			loop.loop_number = i + 1;
			loop.main_panel(gui,x,y);

			if(loop.graphics_state == tape_lop::gui_state::COLLAPSED)
			{
				fill += 1;
			}
			else
			{
				fill += 2;
			}

		}
		
		gui.end_frame();
		frame_buddy.delay();
	}
}

void tape_lop_screen::do_the_audio(void* outbuf,void* inbuf,unsigned int nFrames,double streamtime,RtAudioStreamStatus	status,const int in_channels, const int out_channels)
{

	float	*out_buffer = static_cast<float*>(outbuf);
	float *in_buffer = static_cast<float*>(inbuf);

	for(int frame_offset=0;frame_offset<(nFrames); ++frame_offset)
	{
		for(int i=0; i<out_channels;++i)
		{
			float* out_channel = out_buffer + (i*nFrames);
			out_channel[frame_offset] =0;
		}

		for(tape_lop& loop : loops )
		{
			int in_chans = std::min<int>(loop.inputs.size(),in_channels);
			int out_chans = std::min<int>(loop.outputs.size(),out_channels);
			float input_val =0;

			for(int i=0; i<in_chans;++i)
			{
				float amp = loop.inputs.at(i);
				input_val += (in_buffer + (i*nFrames))[frame_offset] * amp;
			};

			float output_val = loop.tick(input_val);

			for(int i=0; i<out_chans;++i)
			{
				float amp = loop.outputs.at(i);
				float* out_channel = out_buffer + (i*nFrames);
				out_channel[frame_offset] += output_val * amp;
			};			
		}
	}
}