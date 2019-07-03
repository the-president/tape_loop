#include <sndfile.hh>
#include "audio.h"
#include "file_dialog/tinyfiledialogs.h"
#include "tape_lop.h"

static const int panel_width = 1278;
static const int panel_height = 400;
static const int collapsed_panel_height = 210;
static const int panel_margin = 2;
static const int rh_panel_w = 335;
static const int mixer_ph = 192;
static const int mixer_pw = 634;
static const int ph = 160;
static const int gap= 30;
static const int in_out_page_size = 8;

tape_lop::action_call tape_lop::collapsed_panel(graphical_interface& gui, int x, int y)
{
	tape_lop::action_call retval = tape_lop::action_call::NONE;

	gui.panel(x,y,panel_width,collapsed_panel_height);
	{
		retval = top_controls(gui,panel_margin,panel_margin);
		
		int bottom_line = gui.bottom() + panel_margin;

		transport_panel(gui,panel_margin,bottom_line);

		for(int i = 0; i< read_heads.size(); ++i)
		{
			read_head& rh = read_heads.at(i);
			read_head_view(gui,gui.right_end()+panel_margin,bottom_line,rh,i);
		}
	}
	gui.panel_end();

	return retval;
}

tape_lop::action_call tape_lop::mixer_panel(graphical_interface& gui, int x, int y)
{
	tape_lop::action_call retval = tape_lop::action_call::NONE;

	gui.panel(x,y,panel_width,panel_height);
	{
		retval = top_controls(gui,panel_margin,panel_margin);
		int start_line = gui.left_end();

		int mid_line = gui.bottom() + panel_margin;

		input_mixer(gui,panel_margin,mid_line);
		output_mixer(gui,gui.right_end() + (2 * panel_margin),mid_line);

		int bottom_line = gui.bottom() + panel_margin;

		transport_panel(gui,start_line,bottom_line);

		int rh_start = gui.right_end() + 2;

		for(int i = 0; i< read_heads.size(); ++i)
		{
			read_head& rh = read_heads.at(i);
			read_head_view(gui,rh_start,bottom_line,rh,i);
			rh_start = gui.right_end() + 2;
		}
	}
	gui.panel_end();

	return retval;
}

tape_lop::action_call tape_lop::waveform_panel(graphical_interface& gui, int x, int y)
{
	tape_lop::action_call retval = tape_lop::action_call::NONE;

	gui.panel(x,y,panel_width,panel_height);
	{
		retval = top_controls(gui,panel_margin,panel_margin);
		int start_line = gui.left_end();

		gui.tape_view(start_line,gui.bottom() + panel_margin,*this);

		int bottom_line = gui.bottom() + panel_margin;

		transport_panel(gui,start_line,bottom_line);

		int rh_start = gui.right_end() + 2;

		for(int i = 0; i< read_heads.size(); ++i)
		{
			read_head& rh = read_heads.at(i);
			read_head_view(gui,rh_start,bottom_line,rh,i);
			rh_start = gui.right_end() + 2;
		}
	}
	gui.panel_end();

	return retval;
}

tape_lop::action_call tape_lop::main_panel(graphical_interface& gui, int x, int y)
{
	auto retval = tape_lop::action_call::NONE;

	switch(graphics_state)
	{
		case GONE:
			retval = tape_lop::action_call::NONE;
		break;
		case COLLAPSED:
			retval = collapsed_panel(gui, x, y);
		break;
		case MIXER:
			retval = mixer_panel(gui, x, y);
		break;
		case WAVE_FORM:
			retval = waveform_panel(gui, x, y);
		break;
		default:
			retval = tape_lop::action_call::NONE;
		break;
	}

	return retval;
}

tape_lop::action_call tape_lop::top_controls(graphical_interface& gui, int x, int y)
{
	tape_lop::action_call retval = tape_lop::action_call::NONE;

	gui.panel(x,y,panel_width-7,40);
	{
		int other_side = panel_width-7;
		int center_line = 12;

		if( gui.select_button(gui.right_end()+panel_margin,center_line,"collapsed",graphics_state == COLLAPSED) )
		{
			graphics_state = COLLAPSED;
		}

		if( gui.select_button(gui.right_end()+panel_margin,center_line,"mixer",graphics_state == MIXER) )
		{
			graphics_state = MIXER;
		}

		if( gui.select_button(gui.right_end()+panel_margin,center_line,"wave form",graphics_state == WAVE_FORM) )
		{
			graphics_state = WAVE_FORM;
		}

		gui.format_label(gui.right_end()+10, center_line + 2,"loop %d",loop_number);

		if (gui.button(other_side - 200, center_line,"X"))
		{
			retval = tape_lop::action_call::DELETE;
		}

		if (gui.button(gui.right_end()+panel_margin, center_line,"dupe"))
		{
			retval = tape_lop::action_call::DUPE;
		}

		if (gui.button(gui.right_end()+panel_margin, center_line,"save"))
		{
			const char* path = tinyfd_saveFileDialog("open file","",0,NULL,NULL);

			if(path != NULL)
			{
				SF_INFO inf;
				
				inf.samplerate = get_global_audio().sample_rate;;
				inf.frames = tape.size();
				inf.channels	= 1;
				inf
				.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24;

				SNDFILE* file = sf_open(path, SFM_WRITE, &inf);

				if(file != NULL)
				{
					double* copybuff = new double[tape.size()];

					for(int i=0;i<tape.size();++i)
					{
						copybuff[i] = tape.at(i);
					}

					sf_write_double(file,copybuff,tape.size());
					delete[] copybuff;
					sf_close(file);
				}
				else
				{
					printf("error saving file\n");
				}
			}
		}

		if (gui.button(gui.right_end()+panel_margin, center_line,"normalize"))
		{
			normalize();
		}
	}
	gui.panel_end();

	return retval;
}

void tape_lop::input_mixer(graphical_interface& gui, int x, int y)
{
	

	gui.panel(panel_margin,y,mixer_pw, mixer_ph);
	{
		int panel_start = 5;
		gui.flat_label(panel_start,gui.top()+5,"input mixer");

		if(inputs.size() > in_out_page_size)
		{
			if(gui.fs_button(gui.right_end() + 12, gui.top(), 20,20,"<") && input_page > 0)
			{
				input_page--;
			}

			if(gui.fs_button(gui.right_end() + 2, gui.top(), 20,20,">") && input_page < inputs.size() - in_out_page_size)
			{
				input_page++;
			}
		}

		//the -10 is a hack
		int input_panel_top = gui.bottom() + 5;
		gui.set_layout_rect(panel_start-10,input_panel_top,0,0);

		for(int i=input_page; i < input_page + in_out_page_size && i < inputs.size(); ++i)
		{
			int col_start = gui.right_end()+10;
			double& in = inputs.at(i);

			gui.format_label(col_start,input_panel_top,"in %d",i + 1);
			gui.double_knob(col_start, gui.bottom()+5,in,0,2);

			int button_row = gui.bottom() + 5;

			if(gui.fs_button(col_start,button_row,45,20,"ON"))
			{
				in = 1;
			}

			if(gui.fs_button(col_start,button_row + gap,45,20,"OFF"))
			{
				in = 0;
			}
		}

		int filter_x = panel_width - 160;

		gui.flat_label(filter_x,input_panel_top,"input filter");
		gui.double_knob(filter_x,gui.bottom() + 5,input_filter.cutoff,0.001,0.9999);

		if(gui.fs_button(panel_start,150,100,30,"mute all"))
		{
			for(double& in : inputs)
			{
				in = 0;
			}
		}
	}
	gui.panel_end();
}

void tape_lop::output_mixer(graphical_interface& gui, int x, int y)
{
	gui.panel(gui.right_end() + panel_margin,y,mixer_pw, mixer_ph);
	{
		int panel_start = 5;
		gui.flat_label(panel_start,gui.top()+5,"output mixer");

		if(outputs.size() > in_out_page_size)
		{
			if(gui.fs_button(gui.right_end() + 12, gui.top(), 20,20,"<") && output_page > 0)
			{
				output_page--;
			}

			if(gui.fs_button(gui.right_end() + 2, gui.top(), 20,20,">") && output_page < outputs.size() - in_out_page_size)
			{
				output_page++;
			}
		}
		
		//the -10 is a hack
		int input_panel_top = gui.bottom() + 5;
		gui.set_layout_rect(panel_start-15,input_panel_top,0,0);


		for(int i=output_page; i < output_page + in_out_page_size && i < outputs.size(); ++i)
		{
			int col_start = gui.right_end()+15;
			double& out = outputs.at(i);

			gui.format_label(col_start,input_panel_top,"out %d",i + 1);
			gui.double_knob(col_start, gui.bottom()+5,out,0,2);

			int button_row = gui.bottom() + 5;

			if(gui.fs_button(col_start,button_row,45,20,"ON"))
			{
				out = 1;
			}

			if(gui.fs_button(col_start,button_row + gap,45,20,"OFF"))
			{
				out = 0;
			}
		}

		if(gui.fs_button(panel_start,150,100,30,"mute all"))
		{
			for(double& out : outputs)
			{
				out = 0;
			}
		}

	}
	gui.panel_end();
}

void tape_lop::read_head_view(graphical_interface& gui, int x, int y, read_head& rh, int index)
{
	gui.panel(x,y,rh_panel_w, ph);
	{
		static const int col_start = 5;
		static const int col2 = 160;

		gui.format_label(col_start,2,"read head %d",index + 1);

		if(gui.select_button(col_start,gui.bottom()+2,"on",rh.active))
		{
			rh.active = !rh.active;
		}

		int button_spot = gui.bottom()+5;
		gui.int_knob(col_start,button_spot,rh.offset.target,1,tape.size(),500);

		int button_spot2 = gui.bottom()+5;
		gui.int_label(col_start + 37,button_spot+8,"offset",rh.offset.val);

		gui.double_knob(col_start,button_spot2,rh.amp.target,0,4);
		int button_spot3 = gui.bottom() + 5;
		gui.double_label(col_start + 37,button_spot2+8,"amp",rh.amp.val);

		gui.double_knob(col_start,button_spot3,rh.feedback.target,0,1.5);
		gui.double_label(col_start + 37,button_spot3+8,"FB",rh.feedback.val);

		gui.double_knob(col2,button_spot,rh.output_filter.cutoff,0.001,0.9999);

		gui.double_label(col2 + 37, button_spot+8,"cutoff",rh.output_filter.cutoff);

		gui.double_knob(col2,button_spot2,rh.output_filter.reso,0.00,0.9999);
		gui.double_label(col2 + 37, button_spot2+8,"reso",rh.output_filter.reso);

		if(gui.select_button(col2,button_spot3+6,"L",rh.output_filter.mode == LP))
		{
			rh.output_filter.mode = LP;
		}

		if(gui.select_button(gui.right_end()+2,button_spot3+6,"B",rh.output_filter.mode == BP))
		{
			rh.output_filter.mode = BP;
		}

		if(gui.select_button(gui.right_end()+2,button_spot3+6,"H",rh.output_filter.mode == HP))
		{
			rh.output_filter.mode = HP;
		}
	}
	gui.panel_end();
}

void tape_lop::transport_panel(graphical_interface& gui, int x, int y)
{
	static int button_gap = 70;

	gui.panel(x,y,260, ph);
	{
		int panel_start = 5;
		gui.flat_label(panel_start,gui.top()+5,"transport");
		int col_start = gui.left_end();
		int row_zone = gui.bottom() + button_gap;
		
		if(gui.play_button(col_start,row_zone,play))
		{
			play = !play;
		}

		if(gui.record_button(gui.right_end()+2,row_zone,record))
		{
			record_button();
		}

		if(gui.erase_button(gui.right_end()+2,row_zone,erase))
		{
			erase = !erase;
		}

		if(gui.feedback_button(gui.right_end()+2,row_zone,feedback))
		{
			feedback_button();
		}

		gui.double_knob(gui.right_end()+45,row_zone,speed,.25,2);
		int speed_center = gui.center_x();

		int second_row = gui.bottom()+2;

		if(gui.fs_button(col_start,second_row,32,32,"1/4"))
		{
			speed = .25;
		}

		if(gui.fs_button(gui.right_end()+2,second_row,32,32,"1/2"))
		{
			speed = .5;
		}

		if(gui.fs_button(gui.right_end()+2,second_row,32,32,"1"))
		{
			speed = 1;
		}

		if(gui.fs_button(gui.right_end()+2,second_row,32,32,"2"))
		{
			speed = 2;
		}

		int c_line = gui.center_y()-8;
		//fuck it just magic number
		gui.double_label(speed_center - (6*9),c_line,"Speed",speed);
	}
	gui.panel_end();
}

int tape_lop::fill()
{
	switch(graphics_state)
	{
		case GONE:
			return 0;
		break;
		case COLLAPSED:
			return 1;
		break;
		default:
			return 2;
		break;
	}
}