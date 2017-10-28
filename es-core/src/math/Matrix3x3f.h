#ifndef _MATRIX3X3F_H_
#define _MATRIX3X3F_H_

#include <assert.h>
#include <math/Vector3f.h>

class Matrix3x3f
{
public:

	Matrix3x3f()                                                                                       { }
	Matrix3x3f(const Vector3f& r0, const Vector3f& r1, const Vector3f& r2) : mR0(r0), mR1(r1), mR2(r2) { }

	const bool operator==(const Matrix3x3f& other) const { return ((mR0 == other.mR0) && (mR1 == other.mR1) && (mR2 == other.mR2)); }
	const bool operator!=(const Matrix3x3f& other) const { return ((mR0 != other.mR0) || (mR1 != other.mR1) || (mR2 != other.mR2)); }

	const Matrix3x3f operator*(const Matrix3x3f& other) const
	{
		const float* tm = (float*)this;
		const float* om = (float*)&other;

		return
		{
			{
				tm[0] * om[0] + tm[1] * om[3] + tm[2] * om[6],
				tm[0] * om[1] + tm[1] * om[4] + tm[2] * om[7],
				tm[0] * om[2] + tm[1] * om[5] + tm[2] * om[8]
			},
			{
				tm[3] * om[0] + tm[4] * om[3] + tm[5] * om[6],
				tm[3] * om[1] + tm[4] * om[4] + tm[5] * om[7],
				tm[3] * om[2] + tm[4] * om[5] + tm[5] * om[8]
			},
			{
				tm[6] * om[0] + tm[7] * om[3] + tm[8] * om[6],
				tm[6] * om[1] + tm[7] * om[4] + tm[8] * om[7],
				tm[6] * om[2] + tm[7] * om[5] + tm[8] * om[8]
			}
		};
	}

	const Vector3f operator*(const Vector3f& other) const
	{
		const float* tm = (float*)this;
		const float* ov = (float*)&other;

		return
		{
			tm[0] * ov[0] + tm[3] * ov[1] + tm[6] * ov[2],
			tm[1] * ov[0] + tm[4] * ov[1] + tm[7] * ov[2],
			tm[2] * ov[0] + tm[5] * ov[1] + tm[8] * ov[2]
		};
	}

	Matrix3x3f& operator*=(const Matrix3x3f& other) { *this = *this * other; return *this; }

	      float& operator[](const int index)       { assert(index < 9 && "index out of range"); return ((float*)&mR0)[index]; }
	const float& operator[](const int index) const { assert(index < 9 && "index out of range"); return ((float*)&mR0)[index]; }

	      Vector3f& r0()       { return mR0; }
	      Vector3f& r1()       { return mR1; }
	      Vector3f& r2()       { return mR2; }
	const Vector3f& r0() const { return mR0; }
	const Vector3f& r1() const { return mR1; }
	const Vector3f& r2() const { return mR2; }

	static const Matrix3x3f Identity() { return { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }; }

protected:

	Vector3f mR0;
	Vector3f mR1;
	Vector3f mR2;

};

#endif
