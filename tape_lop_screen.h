#ifndef TAPE_LOP_SCREEN_DOT_H
#define TAPE_LOP_SCREEN_DOT_H

#include <vector>
#include "tape_lop.h"
#include "screen.h"
#include "audio_guy.h"

struct tape_lop_screen : public screen, audio_guy
{
	std::vector<tape_lop> loops;

	int bpm = 120;
	int bars = 1;

	void add_loop(unsigned int sample_rate,int inputs,int outputs);
	void drop_loop(int index);

	void top_panel(graphical_interface& gui, audio_system& audio, int& scroll, int gap=586);

	float seconds();
	int samples(unsigned int sample_rate);

	screen_result loop(sdl_system& sys, audio_system& audio,graphical_interface& gui,frame_rate_manager& frame_buddy);
	void do_the_audio(void* outbuf,void* inbuf,unsigned int nFrames,double streamtime,RtAudioStreamStatus	status,const int in_channels, const int out_channels);
};

#endif