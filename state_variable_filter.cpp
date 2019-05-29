#include <cmath>
#include "state_variable_filter.h"

svf::svf():
cutoff(.99),
reso(0.0),
mode(LP),
stage_1(0.0),
stage_2(0.0),
stage_3(0.0),
stage_4(0.0)
{}

svf::svf(float c,float r,svf_mode m):
cutoff(c),
reso(r),
mode(m),
stage_1(0.0),
stage_2(0.0),
stage_3(0.0),
stage_4(0.0)
{}

float svf::tick(float input)
{
	float cut = cutoff;
	calculate_feedback(cut);

	stage_1 += cut * (input - stage_1 + feedback * (stage_1 - stage_2));
	stage_2 += cut * (stage_1 - stage_2);
	stage_3 += cut * (stage_2 - stage_3);
	stage_4 += cut * (stage_3 - stage_4);

	switch(mode)
	{
		case LP:
			return stage_4;
		break;
		case HP:
			return input - stage_4;	
		break;
		case BP:
			return (stage_1 - stage_4);
		break;
		case OFF:
			return input;
		break;
	}
}

void svf::calculate_feedback(float cut)
{ 
	
	feedback = reso + reso/(1.0 - cut); 
}