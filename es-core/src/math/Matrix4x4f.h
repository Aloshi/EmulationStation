#ifndef _MATRIX4X4F_H_
#define _MATRIX4X4F_H_

#include <assert.h>
#include <math/Matrix3x3f.h>
#include <math/Vector4f.h>

class Matrix4x4f
{
public:

	friend class Transform4x4f;

	Matrix4x4f()                                                                                                                    { }
	Matrix4x4f(const Vector4f& r0, const Vector4f& r1, const Vector4f& r2, const Vector4f& r3) : mR0(r0), mR1(r1), mR2(r2), mR3(r3) { }

	const bool operator==(const Matrix4x4f& other) const { return ((mR0 == other.mR0) && (mR1 == other.mR1) && (mR2 == other.mR2) && (mR3 == other.mR3)); }
	const bool operator!=(const Matrix4x4f& other) const { return ((mR0 != other.mR0) || (mR1 != other.mR1) || (mR2 != other.mR2) || (mR3 != other.mR3)); }

	const Matrix4x4f operator*(const Matrix4x4f& other) const
	{
		const float* tm = (float*)this;
		const float* om = (float*)&other;

		return
		{
			{
				tm[ 0] * om[ 0] + tm[ 1] * om[ 4] + tm[ 2] * om[ 8] + tm[ 3] * om[12],
				tm[ 0] * om[ 1] + tm[ 1] * om[ 5] + tm[ 2] * om[ 9] + tm[ 3] * om[13],
				tm[ 0] * om[ 2] + tm[ 1] * om[ 6] + tm[ 2] * om[10] + tm[ 3] * om[14],
				tm[ 0] * om[ 3] + tm[ 1] * om[ 7] + tm[ 2] * om[11] + tm[ 3] * om[15]
			},
			{
				tm[ 4] * om[ 0] + tm[ 5] * om[ 4] + tm[ 6] * om[ 8] + tm[ 7] * om[12],
				tm[ 4] * om[ 1] + tm[ 5] * om[ 5] + tm[ 6] * om[ 9] + tm[ 7] * om[13],
				tm[ 4] * om[ 2] + tm[ 5] * om[ 6] + tm[ 6] * om[10] + tm[ 7] * om[14],
				tm[ 4] * om[ 3] + tm[ 5] * om[ 7] + tm[ 6] * om[11] + tm[ 7] * om[15]
			},
			{
				tm[ 8] * om[ 0] + tm[ 9] * om[ 4] + tm[10] * om[ 8] + tm[11] * om[12],
				tm[ 8] * om[ 1] + tm[ 9] * om[ 5] + tm[10] * om[ 9] + tm[11] * om[13],
				tm[ 8] * om[ 2] + tm[ 9] * om[ 6] + tm[10] * om[10] + tm[11] * om[14],
				tm[ 8] * om[ 3] + tm[ 9] * om[ 7] + tm[10] * om[11] + tm[11] * om[15]
			},
			{
				tm[12] * om[ 0] + tm[13] * om[ 4] + tm[14] * om[ 8] + tm[15] * om[12],
				tm[12] * om[ 1] + tm[13] * om[ 5] + tm[14] * om[ 9] + tm[15] * om[13],
				tm[12] * om[ 2] + tm[13] * om[ 6] + tm[14] * om[10] + tm[15] * om[14],
				tm[12] * om[ 3] + tm[13] * om[ 7] + tm[14] * om[11] + tm[15] * om[15]
			}
		};
	}

	const Matrix4x4f operator*(const Matrix3x3f& other) const
	{
		const float* tm = (float*)this;
		const float* om = (float*)&other;

		return
		{
			{
				tm[ 0] * om[0] + tm[ 1] * om[3] + tm[ 2] * om[6],
				tm[ 0] * om[1] + tm[ 1] * om[4] + tm[ 2] * om[7],
				tm[ 0] * om[2] + tm[ 1] * om[5] + tm[ 2] * om[8],
				tm[ 3]
			},
			{
				tm[ 4] * om[0] + tm[ 5] * om[3] + tm[ 6] * om[6],
				tm[ 4] * om[1] + tm[ 5] * om[4] + tm[ 6] * om[7],
				tm[ 4] * om[2] + tm[ 5] * om[5] + tm[ 6] * om[8],
				tm[ 7]
			},
			{
				tm[ 8] * om[0] + tm[ 9] * om[3] + tm[10] * om[6],
				tm[ 8] * om[1] + tm[ 9] * om[4] + tm[10] * om[7],
				tm[ 8] * om[2] + tm[ 9] * om[5] + tm[10] * om[8],
				tm[11]
			},
			{
				tm[12],
				tm[13],
				tm[14],
				tm[15]
			}
		};
	}

	Matrix4x4f& operator*=(const Matrix4x4f& other) { *this = *this * other; return *this; }
	Matrix4x4f& operator*=(const Matrix3x3f& other) { *this = *this * other; return *this; }

	      float& operator[](const int index)       { assert(index < 16 && "index out of range"); return ((float*)&mR0)[index]; }
	const float& operator[](const int index) const { assert(index < 16 && "index out of range"); return ((float*)&mR0)[index]; }

	      Vector4f& r0()       { return mR0; }
	      Vector4f& r1()       { return mR1; }
	      Vector4f& r2()       { return mR2; }
	      Vector4f& r3()       { return mR3; }
	const Vector4f& r0() const { return mR0; }
	const Vector4f& r1() const { return mR1; }
	const Vector4f& r2() const { return mR2; }
	const Vector4f& r3() const { return mR3; }

	Matrix4x4f& invert(const Matrix4x4f& other)
	{
		auto Determinant2x2 = [=](const float x1, const float x2,
		                          const float y1, const float y2)
		{
			return x1 * y2 - x2 * y1;
		};

		auto Determinant3x3 = [=](const float x1, const float x2, const float x3,
		                          const float y1, const float y2, const float y3,
		                          const float z1, const float z2, const float z3)
		{
			return x1 * Determinant2x2(y2, y3, z2, z3) -
			       y1 * Determinant2x2(x2, x3, z2, z3) +
			       z1 * Determinant2x2(x2, x3, y2, y3);
		};

		float*       tm = (float*)this;
		const float* om = (float*)&other;

		tm[ 0] =  Determinant3x3(om[ 5], om[ 6], om[ 7], om[ 9], om[10], om[11], om[13], om[14], om[15]);
		tm[ 1] = -Determinant3x3(om[ 1], om[ 2], om[ 3], om[ 9], om[10], om[11], om[13], om[14], om[15]);
		tm[ 2] =  Determinant3x3(om[ 1], om[ 2], om[ 3], om[ 5], om[ 6], om[ 7], om[13], om[14], om[15]);
		tm[ 3] = -Determinant3x3(om[ 1], om[ 2], om[ 3], om[ 5], om[ 6], om[ 7], om[ 9], om[10], om[11]);
		tm[ 4] = -Determinant3x3(om[ 4], om[ 6], om[ 7], om[ 8], om[10], om[11], om[12], om[14], om[15]);
		tm[ 5] =  Determinant3x3(om[ 0], om[ 2], om[ 3], om[ 8], om[10], om[11], om[12], om[14], om[15]);
		tm[ 6] = -Determinant3x3(om[ 0], om[ 2], om[ 3], om[ 4], om[ 6], om[ 7], om[12], om[14], om[15]);
		tm[ 7] =  Determinant3x3(om[ 0], om[ 2], om[ 3], om[ 4], om[ 6], om[ 7], om[ 8], om[10], om[11]);
		tm[ 8] =  Determinant3x3(om[ 4], om[ 5], om[ 7], om[ 8], om[ 9], om[11], om[12], om[13], om[15]);
		tm[ 9] = -Determinant3x3(om[ 0], om[ 1], om[ 3], om[ 8], om[ 9], om[11], om[12], om[13], om[15]);
		tm[10] =  Determinant3x3(om[ 0], om[ 1], om[ 3], om[ 4], om[ 5], om[ 7], om[12], om[13], om[15]);
		tm[11] = -Determinant3x3(om[ 0], om[ 1], om[ 3], om[ 4], om[ 5], om[ 7], om[ 8], om[ 9], om[11]);
		tm[12] = -Determinant3x3(om[ 4], om[ 5], om[ 6], om[ 8], om[ 9], om[10], om[12], om[13], om[14]);
		tm[13] =  Determinant3x3(om[ 0], om[ 1], om[ 2], om[ 8], om[ 9], om[10], om[12], om[13], om[14]);
		tm[14] = -Determinant3x3(om[ 0], om[ 1], om[ 2], om[ 4], om[ 5], om[ 6], om[12], om[13], om[14]);
		tm[15] =  Determinant3x3(om[ 0], om[ 1], om[ 2], om[ 4], om[ 5], om[ 6], om[ 8], om[ 9], om[10]);

		float Determinant = om[ 0] * tm[ 0] +
		                    om[ 4] * tm[ 1] +
		                    om[ 8] * tm[ 2] +
		                    om[12] * tm[ 3];

		if(Determinant != 0)
			Determinant = 1 / Determinant;

		mR0 *= Determinant;
		mR1 *= Determinant;
		mR2 *= Determinant;
		mR3 *= Determinant;

		return *this;
	}

	Matrix4x4f& invert()
	{
		return invert(Matrix4x4f(*this));
	}

	Matrix4x4f inverse()
	{
		Matrix4x4f m;
		m.invert(*this);
		return m;
	}

	static const Matrix4x4f Identity() { return { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } }; }

protected:

	Vector4f mR0;
	Vector4f mR1;
	Vector4f mR2;
	Vector4f mR3;

};

#endif
