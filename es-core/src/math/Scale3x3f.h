#ifndef _SCALE3X3F_H_
#define _SCALE3X3F_H_

class Scale3x3f : public Matrix3x3f
{
public:

	Scale3x3f(const Vector3f& scale)
	{
		float*       tm = (float*)this;
		const float* sv = (float*)&scale;

		tm[0] = sv[0];
		tm[1] = 0;
		tm[2] = 0;
		tm[3] = 0;
		tm[4] = sv[1];
		tm[5] = 0;
		tm[6] = 0;
		tm[7] = 0;
		tm[8] = sv[2];
	};

};

#endif
