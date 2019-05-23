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
	target_parameter<float> amp;
	target_parameter<float> feedback;
	svf output_filter;
	
	bool active = false;

	read_head();
	float position(float const write_head, int const tape_size);
};

struct tape_lop
{
	std::vector<float> tape;

	int write_head;
	float write_head_fraction;
	float last_val;

	float speed = 1;

	bool play;
	bool record;
	bool erase;
	bool feedback;

	svf input_filter;

	void record_button();
	void feedback_button();

	target_parameter<float> global_record;
	target_parameter<float> global_fb;

	int last_sample =0;

	std::vector<float> inputs;
	std::vector<float> outputs;
	std::array<read_head,3> read_heads;

	void update_and_record(float input_val);

	float tick(float input_val);
	
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