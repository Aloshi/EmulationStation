#ifndef _ROTATION3X3F_H_
#define _ROTATION3X3F_H_

class Rotation3x3f : public Matrix3x3f
{
public:

	Rotation3x3f(const float angle, const Vector3f& axis)
	{
		float*       tm = (float*)this;
		const float* av = (float*)&axis;

		const float	s  = sin(-angle);
		const float	c  = cos(-angle);
		const float	t  = 1 - c;
		const float	x  = av[0];
		const float	y  = av[1];
		const float	z  = av[2];
		const float	tx = t * x;
		const float	ty = t * y;
		const float	tz = t * z;
		const float	sx = s * x;
		const float	sy = s * y;
		const float	sz = s * z;

		tm[0] = tx * x + c;
		tm[1] = tx * y - sz;
		tm[2] = tx * z + sy;
		tm[3] = ty * x + sz;
		tm[4] = ty * y + c;
		tm[5] = ty * z - sy;
		tm[6] = tz * x - sy;
		tm[7] = tz * y + sx;
		tm[8] = tz * z + c;
	};

};

#endif
