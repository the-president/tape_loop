#ifndef AUDIO_DOT_H
#define AUDIO_DOT_H

#include <atomic>
#include <RtAudio.h>
#include "audio_guy.h"

struct audio_system
{
	bool stream_open = false;
	bool stream_running = false;

	RtAudio rt;

	int input_device_id;
	int output_device_id;

	unsigned int sample_rate;
	unsigned int in_channels;
	unsigned int out_channels;
	unsigned int buffer_size;

	std::atomic<audio_guy*> aguy;

	audio_system(unsigned int samp_rate,unsigned int buf_sz);
	~audio_system();
	
	bool open_stream();
	void close_stream();

	void start();
	void stop();
};

int rtaudio_callback(void* outbuf,void* inbuf,unsigned int nFrames,double streamtime,RtAudioStreamStatus	status,void* userdata);

//=============================================================================
//a half-assed global implementation
//=============================================================================


void set_global_audio(audio_system* sys);
const audio_system& get_global_audio();

#endif