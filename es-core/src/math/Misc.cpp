#include "math/Misc.h"

#include <math.h>

namespace Math
{
	// added here to avoid including math.h whenever these are used
	float cosf(const float _num)
	{
		return ::cosf(_num);

	} // Math::cos

	float sinf(const float _num)
	{
		return ::sinf(_num);

	} // Math::sin

	float floorf(const float _num)
	{
		return ::floorf(_num);

	} // Math::floor

	float ceilf(const float _num)
	{
		return ::ceilf(_num);

	} // Math::ceil

	int min(const int _num1, const int _num2)
	{
		return (_num1 < _num2) ? _num1 : _num2;

	} // Math::min

	int max(const int _num1, const int _num2)
	{
		return (_num1 > _num2) ? _num1 : _num2;

	} // Math::max

	float min(const float _num1, const float _num2)
	{
		return (_num1 < _num2) ? _num1 : _num2;

	} // Math::min

	float max(const float _num1, const float _num2)
	{
		return (_num1 > _num2) ? _num1 : _num2;

	} // Math::max

	float clamp(const float _min, const float _max, const float _num)
	{
		return max(min(_num, _max), _min);

	} // Math::clamp

	float round(const float _num)
	{
		return (float)(int)(_num + 0.5);

	} // Math::round

	float lerp(const float _start, const float _end, const float _fraction)
	{
		return (_start + ((_end - _start) * clamp(0, 1, _fraction)));

	} // Math::lerp

	float smoothStep(const float _left, const float _right, const float _x)
	{
		const float x = clamp(0, 1, (_x - _left)/(_right - _left));
		return x * x * (3 - (2 * x));

	} // Math::smoothStep

	float smootherStep(const float _left, const float _right, const float _x)
	{
		const float x = clamp(0, 1, (_x - _left)/(_right - _left));
		return x * x * x * (x * ((x * 6) - 15) + 10);

	} // Math::smootherStep

	namespace Scroll
	{
		float bounce(const float _delayTime, const float _scrollTime, const float _currentTime, const float _scrollLength)
		{
			if(_currentTime < _delayTime)
			{
				// wait
				return 0;
			}
			else if(_currentTime < (_delayTime + _scrollTime))
			{
				// lerp from 0 to scrollLength
				const float fraction = (_currentTime - _delayTime) / _scrollTime;
				return lerp(0.0f, _scrollLength, smootherStep(0, 1, fraction));
			}
			else if(_currentTime < (_delayTime + _scrollTime + _delayTime))
			{
				// wait some more
				return _scrollLength;
			}
			else if(_currentTime < (_delayTime + _scrollTime + _delayTime + _scrollTime))
			{
				// lerp back from scrollLength to 0
				const float fraction = (_currentTime - _delayTime - _scrollTime - _delayTime) / _scrollTime;
				return lerp(_scrollLength, 0.0f, smootherStep(0, 1, fraction));
			}

			// and back to waiting
			return 0;

		} // Math::Scroll::bounce

		float loop(const float _delayTime, const float _scrollTime, const float _currentTime, const float _scrollLength)
		{
			if(_currentTime < _delayTime)
			{
				// wait
				return 0;
			}
			else if(_currentTime < (_delayTime + _scrollTime))
			{
				// lerp from 0 to scrollLength
				const float fraction = (_currentTime - _delayTime) / _scrollTime;
				return lerp(0.0f, _scrollLength, fraction);
			}

			// and back to waiting
			return 0;

		} // Math::Scroll::loop

	} // Math::Scroll::

} // Math::
