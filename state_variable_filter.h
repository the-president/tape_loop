#ifndef STATE_VARIABLE_FILTER_DOT_H
#define STATE_VARIABLE_FILTER_DOT_H

enum svf_mode
{
	LP,
	HP,
	BP
};

struct svf
{
	svf();
	svf(float c,float r,svf_mode m);

	float tick(float input);

	float cutoff;
	float reso;
	svf_mode mode;
	
	private:
	float feedback;
	
	void calculate_feedback(float cut);
	
	float stage_1;
	float stage_2;
	float stage_3;
	float stage_4;
};

#endif