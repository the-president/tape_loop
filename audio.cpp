#include <stdlib.h>
#include <stdio.h>
#include <RtAudio.h>
#include "audio.h"

void error_callback(RtAudioError::Type type, const std::string &errorText)
{
	printf("STREAM ERROR: %s\n",errorText.c_str() );
}

int rtaudio_callback(void* outbuf,void* inbuf,unsigned int nFrames,double streamtime,RtAudioStreamStatus	status,void* userdata)
{	
	switch(status)
	{
		case RTAUDIO_INPUT_OVERFLOW:
			printf("INPUT OVERFLOW\n");
		break;
		case RTAUDIO_OUTPUT_UNDERFLOW:
			printf("OUTPUT UNDERFLOW\n");
		break;
	}

	audio_system* sys = (audio_system*)userdata;
	sys->aguy.load()->do_the_audio(outbuf,inbuf,nFrames,streamtime,status,sys->in_channels,sys->out_channels);

	return 0;
}

audio_system::audio_system(unsigned int samp_rate,unsigned int buf_sz):
rt(),
input_device_id(-1),
output_device_id(-1),
sample_rate(samp_rate),
in_channels(0),
out_channels(0),
buffer_size(buf_sz)
{
}

audio_system::~audio_system()
{
	if(stream_running)
	{
		stop();
	}
	
	if(stream_open)
	{
		close_stream();	
	}
}

bool audio_system::open_stream()
{
	RtAudio::StreamOptions stream_opts;
	stream_opts.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;

	if(stream_open == true)
	{
		return false;
	}

	if(output_device_id < 0)
	{
		//this is a problem
		return false;
	}

	//we are just doing output
	if(input_device_id < 0)
	{
		RtAudio::StreamParameters stream_params;

		stream_params.deviceId = output_device_id;
		stream_params.nChannels = out_channels;

		rt.openStream(&stream_params, NULL, RTAUDIO_FLOAT64, sample_rate, &buffer_size, rtaudio_callback, this,&stream_opts);
		stream_open = true;
		return true;
	}

	//we are doing duplex
	RtAudio::StreamParameters in_params;
	RtAudio::StreamParameters out_params;

	in_params.deviceId = input_device_id;
	in_params.nChannels = in_channels;
	out_params.deviceId = output_device_id;
	out_params.nChannels = out_channels;

	rt.openStream(&out_params, &in_params, RTAUDIO_FLOAT64, sample_rate, &buffer_size, rtaudio_callback, this,&stream_opts,error_callback);
	stream_open = true;
	return true;
}

void audio_system::close_stream()
{
	if(stream_open)
	{
		rt.closeStream();
		stream_open = false;	
	}
}

void audio_system::start()
{
	if(stream_running == false)
	{
		rt.startStream();
		stream_running = true;
	}
}

void audio_system::stop()
{
	if(stream_running)
	{
		rt.stopStream();
		stream_running = false;	
	}	
}

static audio_system* global_audio_system = NULL;

void set_global_audio(audio_system* sys)
{
	global_audio_system = sys;
}

const audio_system& get_global_audio()
{
	if(global_audio_system != NULL)
	{
		return *global_audio_system;	
	}
	else
	{
		//gtfo, im doing this half-assed for now
		throw "global audio system invalid access";
	}
	
}