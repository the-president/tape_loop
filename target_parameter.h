#ifndef TARGET_PARAMETER_DOT_H
#define TARGET_PARAMETER_DOT_H

//ok so this expects numbers more or less but it's hard to tell you
//if your number is good
template <class T> 
struct target_parameter
{	
	T val;
	T target;
	T increment;

	target_parameter(T v, T tar, T inc):
	val(v),
	target(tar),
	increment(inc)
	{

	}

	void update()
	{
		T diff = target - val;

		if(target > val)
		{
			val += std::min<T>(diff,increment);
		}
		else
		{
			val += std::max<T>(diff,-increment);
		}
	}
};

#endif