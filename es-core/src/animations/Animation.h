#pragma once

#include <Eigen/Dense>

class Animation
{
public:
    virtual ~Animation() { }
	virtual int getDuration() const = 0;
	virtual void apply(float t) = 0;
};


// useful helper/interpolation functions
inline float clamp(float min, float max, float val)
{
	if(val < min)
		val = min;
	else if(val > max)
		val = max;

	return val;
}

//http://en.wikipedia.org/wiki/Smoothstep
inline float smoothStep(float edge0, float edge1, float x)
{
	// Scale, and clamp x to 0..1 range
	x = clamp(0, 1, (x - edge0)/(edge1 - edge0));
        
	// Evaluate polynomial
	return x*x*x*(x*(x*6 - 15) + 10);
}

template<typename T>
T lerp(const T& start, const T& end, float t)
{
	if(t <= 0.0f)
		return start;
	if(t >= 1.0f)
		return end;

	return (start * (1 - t) + end * t);
}
