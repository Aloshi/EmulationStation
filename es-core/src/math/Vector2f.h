#pragma once
#ifndef ES_CORE_MATH_VECTOR2F_H
#define ES_CORE_MATH_VECTOR2F_H

#include <assert.h>

class Vector3f;
class Vector4f;

class Vector2f
{
public:

	         Vector2f()                                                                            { }
	         Vector2f(const float f)                : mX(f),                 mY(f)                 { }
	         Vector2f(const float x, const float y) : mX(x),                 mY(y)                 { }
	explicit Vector2f(const Vector3f& v)            : mX(((Vector2f&)v).mX), mY(((Vector2f&)v).mY) { }
	explicit Vector2f(const Vector4f& v)            : mX(((Vector2f&)v).mX), mY(((Vector2f&)v).mY) { }

	const bool operator==(const Vector2f& other) const { return ((mX == other.mX) && (mY == other.mY)); }
	const bool operator!=(const Vector2f& other) const { return ((mX != other.mX) || (mY != other.mY)); }

	const Vector2f operator+(const Vector2f& other) const { return { mX + other.mX, mY + other.mY }; }
	const Vector2f operator-(const Vector2f& other) const { return { mX - other.mX, mY - other.mY }; }
	const Vector2f operator*(const Vector2f& other) const { return { mX * other.mX, mY * other.mY }; }
	const Vector2f operator/(const Vector2f& other) const { return { mX / other.mX, mY / other.mY }; }

	const Vector2f operator+(const float& other) const { return { mX + other, mY + other }; }
	const Vector2f operator-(const float& other) const { return { mX - other, mY - other }; }
	const Vector2f operator*(const float& other) const { return { mX * other, mY * other }; }
	const Vector2f operator/(const float& other) const { return { mX / other, mY / other }; }

	const Vector2f operator-() const { return { -mX , -mY }; }

	Vector2f& operator+=(const Vector2f& other) { *this = *this + other; return *this; }
	Vector2f& operator-=(const Vector2f& other) { *this = *this - other; return *this; }
	Vector2f& operator*=(const Vector2f& other) { *this = *this * other; return *this; }
	Vector2f& operator/=(const Vector2f& other) { *this = *this / other; return *this; }

	Vector2f& operator+=(const float& other) { *this = *this + other; return *this; }
	Vector2f& operator-=(const float& other) { *this = *this - other; return *this; }
	Vector2f& operator*=(const float& other) { *this = *this * other; return *this; }
	Vector2f& operator/=(const float& other) { *this = *this / other; return *this; }

	      float& operator[](const int index)       { assert(index < 2 && "index out of range"); return (&mX)[index]; }
	const float& operator[](const int index) const { assert(index < 2 && "index out of range"); return (&mX)[index]; }

	      float& x()       { return mX; }
	      float& y()       { return mY; }
	const float& x() const { return mX; }
	const float& y() const { return mY; }

	inline Vector2f& round() { mX = (int)(mX + 0.5f); mY = (int)(mY + 0.5f); return *this; }

	static const Vector2f Zero()  { return { 0, 0 }; }
	static const Vector2f UnitX() { return { 1, 0 }; }
	static const Vector2f UnitY() { return { 0, 1 }; }

private:

	float mX;
	float mY;

};

#endif // ES_CORE_MATH_VECTOR2F_H
