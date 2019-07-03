#include <cmath>
#include <stdio.h>
#include <sndfile.hh>
#include <exception>
#include "tape_lop_screen.h"
#include "file_dialog/tinyfiledialogs.h"

tape_lop& tape_lop_screen::loop_at(int index)
{
	auto iter = loops.begin();

	for(int i=0; i < index; ++i)
	{
		++iter;

		if(iter == loops.end())
		{
			throw "bad loop access";
		}
	}

	return *iter;
}

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

double tape_lop_screen::seconds()
{
	double seconds_per_beat = 60.0/bpm;
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

		if(gui.fs_button(gui.right_end() + 5, panel_padding, 50,30,"load"))
		{
			const char* path = tinyfd_openFileDialog("open file","zomp",0,NULL,NULL,0);

			if(path != NULL)
			{
				SF_INFO inf;
				SNDFILE* file = sf_open(path, SFM_READ, &inf);
				
				loops.emplace_back(audio.in_channels,audio.out_channels,inf.frames);

				tape_lop& load_loop = loop_at(loops.size()-1);

				double read[inf.channels];

				for(int i=0;i<load_loop.tape.size();++i)
				{
					sf_readf_double(file,read,1);
					load_loop.tape[i] = read[0];
				}
				
				sf_close(file);	
			}
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

		double secs = seconds();
		int samps = samples(audio.sample_rate);
		int label_line = gui.right_end() + 20;
		gui.double_label(label_line,30,"seconds: ",secs);
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
	auto iter = loops.begin();

	for(int i=0; i < index; ++i)
	{
		++iter;

		if(iter == loops.end())
		{
			throw "bad loop access";
		}
	}

	loops.erase(iter);
}

void tape_lop_screen::dupe_loop (unsigned int sample_rate,int inputs,int outputs,int index)
{
	int sz = loop_at(index).tape.size();

	loops.emplace_back(inputs,outputs,sz);

	//you need to get the reference after you do any changes to the vector, as it might move things around

	tape_lop& loop = loop_at(index);
	tape_lop& to_dupe = loop_at(loops.size() - 1);

	for(int i=0;i<to_dupe.tape.size();++i)
	{
		to_dupe.tape[i] = loop.tape[i]; 
	}

	//I dont let them set anything else on the input filter
	to_dupe.input_filter.cutoff = loop.input_filter.cutoff;

	for(int i=0;i<to_dupe.inputs.size();++i)
	{
		to_dupe.inputs[i] = loop.inputs[i]; 
	}	

	for(int i=0;i<to_dupe.outputs.size();++i)
	{
		to_dupe.outputs[i] = loop.outputs[i]; 
	}

	for(int i=0;i<3;++i)
	{
		read_head& rh = loop.read_heads[i];
		read_head& dupe_rh = to_dupe.read_heads[i];

		dupe_rh.offset.val = rh.offset.val;
		dupe_rh.offset.target = rh.offset.target;
		dupe_rh.amp.val = rh.amp.val;

		dupe_rh.amp.val = rh.amp.val;
		dupe_rh.amp.target = rh.amp.target;
		dupe_rh.amp.increment = rh.amp.increment;

		dupe_rh.feedback.val = rh.feedback.val;
		dupe_rh.feedback.target = rh.feedback.target;
		dupe_rh.feedback.increment = rh.feedback.increment;
		
		dupe_rh.output_filter.cutoff = rh.output_filter.cutoff;
		dupe_rh.output_filter.reso = rh.output_filter.reso;
		dupe_rh.output_filter.mode = rh.output_filter.mode;
	
		dupe_rh.active = rh.active;
	}
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
			tape_lop& loop = loop_at(i);
			loop.loop_number = i + 1;
			
			tape_lop::action_call loop_action = loop.main_panel(gui,x,y);

			//ok this will cause some frame blips maybe
			fill += loop.fill();

			if(loop_action == tape_lop::action_call::DELETE)
			{
				drop_loop(i);
			}

			if(loop_action == tape_lop::action_call::DUPE)
			{
				dupe_loop(audio.sample_rate,audio.in_channels,audio.out_channels,i);
			}
		}
		
		frame_buddy.delay();
		gui.end_frame();
	}
}

void tape_lop_screen::do_the_audio(void* outbuf,void* inbuf,unsigned int nFrames,double streamtime,RtAudioStreamStatus	status,const int in_channels, const int out_channels)
{

	double	*out_buffer = static_cast<double*>(outbuf);
	double *in_buffer = static_cast<double*>(inbuf);

	for(int frame_offset=0;frame_offset<(nFrames); ++frame_offset)
	{
		for(int i=0; i<out_channels;++i)
		{
			double* out_channel = out_buffer + (i*nFrames);
			out_channel[frame_offset] =0;
		}

		for(tape_lop& loop : loops )
		{
			int in_chans = std::min<int>(loop.inputs.size(),in_channels);
			int out_chans = std::min<int>(loop.outputs.size(),out_channels);
			double input_val =0;

			for(int i=0; i<in_chans;++i)
			{
				double amp = loop.inputs.at(i);
				input_val += (in_buffer + (i*nFrames))[frame_offset] * amp;
			};

			double output_val = loop.tick(input_val);

			for(int i=0; i<out_chans;++i)
			{
				double amp = loop.outputs.at(i);
				double* out_channel = out_buffer + (i*nFrames);
				out_channel[frame_offset] += output_val * amp;
			};			
		}
	}
}