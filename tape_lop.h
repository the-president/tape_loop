#ifndef TAPE_LOP_DOT_H
#define TAPE_LOP_DOT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <array>

#include "graphical_interface.h"
#include "target_parameter.h"
#include "sdl_system.h"
#include "state_variable_filter.h"

class graphical_interface; 

struct read_head
{
	target_parameter<int> offset;
	target_parameter<double> amp;
	target_parameter<double> feedback;
	svf output_filter;
	
	bool active = false;

	read_head();
	double position(double const write_head, int const tape_size);
};

struct tape_lop
{
	std::vector<double> tape;

	int write_head;
	double write_head_fraction;
	double last_val;

	double speed = 1;

	bool play;
	bool record;
	bool erase;
	bool feedback;

	svf input_filter;

	void record_button();
	void feedback_button();

	target_parameter<double> global_record;
	target_parameter<double> global_fb;

	int last_sample =0;

	std::vector<double> inputs;
	std::vector<double> outputs;
	std::array<read_head,3> read_heads;

	void update_and_record(double input_val);

	double tick(double input_val);
	
	tape_lop(int inputs,int outputs,unsigned int length);
	void init_buckets();

	int find_bucket(int sample);

	//---------------------------------------------------
	//GUI STUFF in tape_lop_gui.cpp

	enum gui_state
	{
		COLLAPSED=0,
		MIXER,
		WAVE_FORM
	};

	gui_state graphics_state = COLLAPSED;
	int loop_number = 0;
	int input_page = 0;
	int output_page = 0;

	void collapsed_panel(graphical_interface& gui, int x, int y);
	void mixer_panel(graphical_interface& gui, int x, int y);
	void waveform_panel(graphical_interface& gui, int x, int y);
	void top_controls(graphical_interface& gui, int x, int y);
	void main_panel(graphical_interface& gui, int x, int y);
	void input_mixer(graphical_interface& gui, int x, int y);
	void output_mixer(graphical_interface& gui, int x, int y);
	void read_head_view(graphical_interface& gui, int x, int y, read_head& rh,  int index);
	void transport_panel(graphical_interface& gui, int x, int y);
};

#endif