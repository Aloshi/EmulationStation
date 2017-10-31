#pragma once
#ifndef ES_CORE_MATH_VECTOR3F_H
#define ES_CORE_MATH_VECTOR3F_H

#include <assert.h>

class Vector2f;
class Vector4f;

class Vector3f
{
public:

	         Vector3f()                                                                                                                  { }
	         Vector3f(const float f)                               : mX(f),            mY(f),                      mZ(f)                 { }
	         Vector3f(const float x, const float y, const float z) : mX(x),            mY(y),                      mZ(z)                 { }
	explicit Vector3f(const Vector2f& v)                           : mX(((Vector3f&)v).mX), mY(((Vector3f&)v).mY), mZ(0)                 { }
	explicit Vector3f(const Vector2f& v, const float z)            : mX(((Vector3f&)v).mX), mY(((Vector3f&)v).mY), mZ(z)                 { }
	explicit Vector3f(const Vector4f& v)                           : mX(((Vector3f&)v).mX), mY(((Vector3f&)v).mY), mZ(((Vector3f&)v).mZ) { }

	const bool operator==(const Vector3f& other) const { return ((mX == other.mX) && (mY == other.mY) && (mZ == other.mZ)); }
	const bool operator!=(const Vector3f& other) const { return ((mX != other.mX) || (mY != other.mY) || (mZ != other.mZ)); }

	const Vector3f operator+(const Vector3f& other) const { return { mX + other.mX, mY + other.mY, mZ + other.mZ }; }
	const Vector3f operator-(const Vector3f& other) const { return { mX - other.mX, mY - other.mY, mZ - other.mZ }; }
	const Vector3f operator*(const Vector3f& other) const { return { mX * other.mX, mY * other.mY, mZ * other.mZ }; }
	const Vector3f operator/(const Vector3f& other) const { return { mX / other.mX, mY / other.mY, mZ / other.mZ }; }

	const Vector3f operator+(const float& other) const { return { mX + other, mY + other, mZ + other }; }
	const Vector3f operator-(const float& other) const { return { mX - other, mY - other, mZ - other }; }
	const Vector3f operator*(const float& other) const { return { mX * other, mY * other, mZ * other }; }
	const Vector3f operator/(const float& other) const { return { mX / other, mY / other, mZ / other }; }

	const Vector3f operator-() const { return { -mX , -mY, -mZ }; }

	Vector3f& operator+=(const Vector3f& other) { *this = *this + other; return *this; }
	Vector3f& operator-=(const Vector3f& other) { *this = *this - other; return *this; }
	Vector3f& operator*=(const Vector3f& other) { *this = *this * other; return *this; }
	Vector3f& operator/=(const Vector3f& other) { *this = *this / other; return *this; }

	Vector3f& operator+=(const float& other) { *this = *this + other; return *this; }
	Vector3f& operator-=(const float& other) { *this = *this - other; return *this; }
	Vector3f& operator*=(const float& other) { *this = *this * other; return *this; }
	Vector3f& operator/=(const float& other) { *this = *this / other; return *this; }

	      float& operator[](const int index)       { assert(index < 3 && "index out of range"); return (&mX)[index]; }
	const float& operator[](const int index) const { assert(index < 3 && "index out of range"); return (&mX)[index]; }

	      float& x()       { return mX; }
	      float& y()       { return mY; }
	      float& z()       { return mZ; }
	const float& x() const { return mX; }
	const float& y() const { return mY; }
	const float& z() const { return mZ; }

	inline       Vector2f& v2()       { return *(Vector2f*)this; }
	inline const Vector2f& v2() const { return *(Vector2f*)this; }

	inline Vector3f& round() { mX = (int)(mX + 0.5f); mY = (int)(mY + 0.5f); mZ = (int)(mZ + 0.5f); return *this; }

	static const Vector3f Zero()  { return { 0, 0, 0 }; }
	static const Vector3f UnitX() { return { 1, 0, 0 }; }
	static const Vector3f UnitY() { return { 0, 1, 0 }; }
	static const Vector3f UnitZ() { return { 0, 0, 1 }; }

private:

	float mX;
	float mY;
	float mZ;

};

#endif // ES_CORE_MATH_VECTOR3F_H
