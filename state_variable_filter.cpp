#include <cmath>
#include "audio.h"
#include "state_variable_filter.h"

svf::svf():
mode(LP),
cutoff(.99),
reso(0.0),

state_1(0.0),
state_2(0.0)
{
	sample_rate = get_global_audio().sample_rate;

}

svf::svf(double c,double r,svf_mode m):
mode(m),
cutoff(c),
reso(r),

state_1(0.0),
state_2(0.0)
{
	sample_rate = get_global_audio().sample_rate;
}

void svf::calculate_filter_coefficients()
{
	//first cutoff warping
	double cutoff_freq = (sample_rate/2) * (exp2(cutoff) - 1);
	double q = 1.0/(2 * (1-reso));
	double freq = cutoff_freq * 2 * M_PI;
	double time_scale = 1.0/sample_rate;
	double warped_frequency = (2.0/time_scale) * tan(freq*time_scale/2.0);

	gain = warped_frequency * time_scale/2;
	damping_factor = 1.0/ (2 * q);
}

double svf::tick(double input)
{

	if(mode == OFF)
	{
		return input;
	}

	double highpass_out = (input - (2 * damping_factor + gain) * state_1 - state_2) / (1 + (2 * damping_factor * gain) + gain * gain);
	double bandpass_out = highpass_out * gain + state_1;
	double lowpass_out = bandpass_out * gain + state_2;

	state_1 = gain * highpass_out + bandpass_out;
	state_2 = gain * bandpass_out + lowpass_out;

	switch(mode)
	{
		case LP:
			return lowpass_out;
		break;
		case HP:
			return highpass_out;	
		break;
		case BP:
			return bandpass_out;
		break;
		default:
			return input;
		break;
	}
}