#ifndef _TRANSFORM4X4F_H_
#define _TRANSFORM4X4F_H_

#include <math/Matrix4x4f.h>
#include <math/Vector3f.h>

class Transform4x4f : public Matrix4x4f
{
public:

	Transform4x4f()                                                                                                            { }
	Transform4x4f(const Matrix4x4f& m)                                                            : Matrix4x4f(m)              { }
	Transform4x4f(const Vector4f& r0, const Vector4f& r1, const Vector4f& r2, const Vector4f& r3) : Matrix4x4f(r0, r1, r2, r3) { }

	const Transform4x4f operator*(const Matrix4x4f& other) const
	{
		const float* tm = (float*)this;
		const float* om = (float*)&other;

		return
		{
			{
				tm[ 0] * om[ 0] + tm[ 1] * om[ 4] + tm[ 2] * om[ 8],
				tm[ 0] * om[ 1] + tm[ 1] * om[ 5] + tm[ 2] * om[ 9],
				tm[ 0] * om[ 2] + tm[ 1] * om[ 6] + tm[ 2] * om[10],
				0
			},
			{
				tm[ 4] * om[ 0] + tm[ 5] * om[ 4] + tm[ 6] * om[ 8],
				tm[ 4] * om[ 1] + tm[ 5] * om[ 5] + tm[ 6] * om[ 9],
				tm[ 4] * om[ 2] + tm[ 5] * om[ 6] + tm[ 6] * om[10],
				0
			},
			{
				tm[ 8] * om[ 0] + tm[ 9] * om[ 4] + tm[10] * om[ 8],
				tm[ 8] * om[ 1] + tm[ 9] * om[ 5] + tm[10] * om[ 9],
				tm[ 8] * om[ 2] + tm[ 9] * om[ 6] + tm[10] * om[10],
				0
			},
			{
				tm[ 0] * om[12] + tm[ 4] * om[13] + tm[ 8] * om[14] + tm[12],
				tm[ 1] * om[12] + tm[ 5] * om[13] + tm[ 9] * om[14] + tm[13],
				tm[ 2] * om[12] + tm[ 6] * om[13] + tm[10] * om[14] + tm[14],
				1
			}
		};
	}

	const Transform4x4f operator*(const Matrix3x3f& other) const
	{
		const float* tm = (float*)this;
		const float* om = (float*)&other;

		return
		{
			{
				tm[ 0] * om[0] + tm[ 1] * om[3] + tm[ 2] * om[6],
				tm[ 0] * om[1] + tm[ 1] * om[4] + tm[ 2] * om[7],
				tm[ 0] * om[2] + tm[ 1] * om[5] + tm[ 2] * om[8],
				0
			},
			{
				tm[ 4] * om[0] + tm[ 5] * om[3] + tm[ 6] * om[6],
				tm[ 4] * om[1] + tm[ 5] * om[4] + tm[ 6] * om[7],
				tm[ 4] * om[2] + tm[ 5] * om[5] + tm[ 6] * om[8],
				0
			},
			{
				tm[ 8] * om[0] + tm[ 9] * om[3] + tm[10] * om[6],
				tm[ 8] * om[1] + tm[ 9] * om[4] + tm[10] * om[7],
				tm[ 8] * om[2] + tm[ 9] * om[5] + tm[10] * om[8],
				0
			},
			{
				tm[12],
				tm[13],
				tm[14],
				1
			}
		};
	}

	const Vector3f operator*(const Vector3f& other) const
	{
		const float* tm = (float*)this;
		const float* ov = (float*)&other;

		return
		{
			tm[ 0] * ov[0] + tm[ 4] * ov[1] + tm[ 8] * ov[2] + tm[12],
			tm[ 1] * ov[0] + tm[ 5] * ov[1] + tm[ 9] * ov[2] + tm[13],
			tm[ 2] * ov[0] + tm[ 6] * ov[1] + tm[10] * ov[2] + tm[14]
		};
	}

	Transform4x4f& operator*=(const Matrix4x4f& other) { *this = *this * other; return *this; }
	Transform4x4f& operator*=(const Matrix3x3f& other) { *this = *this * other; return *this; }

	inline       Vector3f& translation()       { return mR3.v3(); }
	inline const Vector3f& translation() const { return mR3.v3(); }

	inline Transform4x4f& translate(const Vector3f& translation)
	{
		float*       tm = (float*)this;
		const float* tv = (float*)&translation;

		tm[12] += tm[ 0] * tv[0] + tm[ 4] * tv[1] + tm[ 8] * tv[2];
		tm[13] += tm[ 1] * tv[0] + tm[ 5] * tv[1] + tm[ 9] * tv[2];
		tm[14] += tm[ 2] * tv[0] + tm[ 6] * tv[1] + tm[10] * tv[2];

		return *this;
	}

	inline Transform4x4f& round()
	{
		float* tm = (float*)this;

		tm[12] = (int)(tm[12] + 0.5f);
		tm[13] = (int)(tm[13] + 0.5f);

		return *this;
	}

};

#endif
