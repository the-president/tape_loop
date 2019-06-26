#ifndef STATE_VARIABLE_FILTER_DOT_H
#define STATE_VARIABLE_FILTER_DOT_H

enum svf_mode
{
	OFF,
	LP,
	HP,
	BP
};

struct svf
{
	svf();
	svf(double c,double r,svf_mode m);

	double tick(double input);
	void calculate_filter_coefficients();
	
	svf_mode mode;

	double cutoff;
	double reso;
	double sample_rate = 48000;
	
	protected:
		double gain;
		double damping_factor;
		double state_1; // first integrator
		double state_2;
};

#endif