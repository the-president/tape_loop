#ifndef FRAME_RATE_MANAGER_DOT_H
#define FRAME_RATE_MANAGER_DOT_H

struct frame_rate_manager
{
	frame_rate_manager(unsigned int fps);
	void start();
	void delay();

	Uint32 ticks;
	Uint32 frame_ticks;

};


#endif