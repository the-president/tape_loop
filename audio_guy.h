#ifndef AUDIO_GUY_DOT_H
#define AUDIO_GUY_DOT_H

struct audio_guy
{
	virtual void do_the_audio(void* outbuf,void* inbuf,unsigned int nFrames,double streamtime,RtAudioStreamStatus	status,const int in_channels, const int out_channels) = 0;
	virtual ~audio_guy(){}
};

#endif