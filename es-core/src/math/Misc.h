#pragma once
#ifndef ES_CORE_MATH_MISC_H
#define ES_CORE_MATH_MISC_H

#include <math.h>

#define	ES_PI (3.1415926535897932384626433832795028841971693993751058209749445923)
#define	ES_RAD_TO_DEG(x) ((x) * (180.0 / ES_PI))
#define	ES_DEG_TO_RAD(x) ((x) * (ES_PI / 180.0))

namespace Math
{
	inline float scroll_bounce(const float delayTime, const float scrollTime, const float currentTime, const int scrollLength)
	{
		if(currentTime < delayTime)
		{
			// wait
			return 0;
		}
		else if(currentTime < (delayTime + scrollTime))
		{
			// lerp from 0 to scrollLength
			const float fraction = (currentTime - delayTime) / scrollTime;
			return (float)(((1.0 - cos(ES_PI * fraction)) * 0.5) * scrollLength);
		}
		else if(currentTime < (delayTime + scrollTime + delayTime))
		{
			// wait some more
			return scrollLength;
		}
		else if(currentTime < (delayTime + scrollTime + delayTime + scrollTime))
		{
			// lerp back from scrollLength to 0
			const float fraction = 1.0 - (currentTime - delayTime - scrollTime - delayTime) / scrollTime;
			return (float)(((1.0 - cos(ES_PI * fraction)) * 0.5) * scrollLength);
		}

		// and back to waiting
		return 0;
	}

	inline float scroll_loop(const float delayTime, const float scrollTime, const float currentTime, const int scrollLength)
	{
		if(currentTime < delayTime)
		{
			// wait
			return 0;
		}
		else if(currentTime < (delayTime + scrollTime))
		{
			// lerp from 0 to scrollLength
			const float fraction = (currentTime - delayTime) / scrollTime;
			return fraction * scrollLength;
		}

		// and back to waiting
		return 0;
	}
}

#endif // ES_CORE_MATH_MISC_H
