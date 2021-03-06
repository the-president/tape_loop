#include <cmath>
#include "tape_lop.h"

read_head::read_head():
offset(100,100,1),
amp(.5,.5,.01),
feedback(.5,.5,.01),
output_filter(.99,0,LP)
{

}

//this works for array bounderies so the actual upper is "upper_bound - 1"
static int wrap(int x, int const lower_bound, int const upper_bound)
{
	int range = upper_bound - lower_bound;

	if (x < lower_bound)
	{
	  x += range * ((lower_bound - x) / range + 1);
	}

	return lower_bound + (x - lower_bound) % range;
}

static double range_flerp(double start,double end, double range, double index)
{
	double slope = (end-start)/range;
	return start + (index * slope);
}

double read_head::position(double const write_head, int const tape_size)
{
	int unwrapped = write_head - offset.val;
	return wrap(unwrapped,0,tape_size);
}

tape_lop::tape_lop(int num_inputs,int num_outputs,unsigned int length):
tape(length,0),
write_head(0),
last_val(0),
play(false),
record(false),
erase(false),
feedback(false),
input_filter(.99,0,OFF),
global_record(0,0,.01),
global_fb(0,0,.01),
inputs(num_inputs,1),
outputs(num_outputs,1)
{
}

void tape_lop::record_button()
{
	if(record)
	{
		global_record.target =0.0;
		record = false;
	}
	else
	{
		global_record.target =1.0;
		record = true;
	}
}

void tape_lop::feedback_button()
{
	if(feedback)
	{
		global_fb.target =0.0;
		feedback = false;
	}
	else
	{
		global_fb.target =1.0;
		feedback = true;
	}
}

void tape_lop::update_and_record(double input_val)
{
	double speed_integer =0;
	double speed_fraction = modf(speed,&speed_integer);

	double write_increment_int;
	double end_fraction = modf(write_head_fraction + speed_fraction,&write_increment_int);

	int total_samples = speed_integer + write_increment_int;
	double range = (double)total_samples + end_fraction - write_head_fraction;

	for(int i=1;i<=total_samples;++i)
	{
		int tape_pos = (write_head + i) % tape.size();
		double val = range_flerp(last_val,input_val,range,(((double)i)-write_head_fraction));

		last_sample = tape_pos;

		if(erase)
		{
			tape.at(tape_pos) = 0;	
		}
	
		tape.at(tape_pos) += val;	
	}

	last_val = input_val;

	write_head = (write_head + total_samples) % tape.size();
	write_head_fraction = end_fraction;
}

double tape_lop::tick(double input_val)
{
	//update the target params
	for(read_head& rh : read_heads)
	{
		rh.offset.update();
		rh.amp.update();
		rh.feedback.update();
		rh.output_filter.calculate_filter_coefficients();
	}

	global_record.update();
	global_fb.update();

	if(!play)
	{
		return 0;
	}

	double out_val = 0;
	double feedback_val =0;

	for(read_head& rh : read_heads)
	{
		if(rh.active)
		{
			int pos = rh.position(write_head,tape.size());
			double fraction = write_head_fraction;

			int bottom_sample = pos;

			double val =0;

			if(fraction == 0)
			{
				val = tape.at(bottom_sample);
			}
			else
			{
				int top_sample = (bottom_sample+1) % tape.size();
				val = range_flerp(tape.at(bottom_sample),tape.at(top_sample),1,fraction);
			}

			double filter_val = rh.output_filter.tick(val);
			out_val += (exp2(rh.amp.val)-1) * filter_val;
			feedback_val += (exp2(rh.feedback.val)-1) * filter_val;	
		}
	}

	double final_input = 0;

	if(record || global_record.val !=0)
	{
		final_input += input_val * global_record.val;
	}

	if(feedback || global_fb.val !=0)
	{
		final_input += feedback_val * global_fb.val;
	}

	double filtered_input = input_filter.tick(final_input);

	//now update positions
	update_and_record(filtered_input);

	return out_val;
}

void tape_lop::normalize()
{
	double abs_max = 0;

	for(double& d : tape)
	{
		if(abs_max < abs(d))
		{
			abs_max=d;
		}
	}

	if(abs_max >= 1.0)
	{
		return;
	}
	else
	{
		double normval = 1.0/abs_max;
		for(double& d : tape)
		{
			d = d * normval;
		}
	}

}
