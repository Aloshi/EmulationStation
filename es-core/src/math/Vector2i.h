#pragma once
#ifndef ES_CORE_MATH_VECTOR2I_H
#define ES_CORE_MATH_VECTOR2I_H

#include <assert.h>

class Vector2i
{
public:

	         Vector2i()                                        { }
	         Vector2i(const int i)              : mX(i), mY(i) { }
	         Vector2i(const int x, const int y) : mX(x), mY(y) { }

	const bool operator==(const Vector2i& other) const { return ((mX == other.mX) && (mY == other.mY)); }
	const bool operator!=(const Vector2i& other) const { return ((mX != other.mX) || (mY != other.mY)); }

	const Vector2i operator+(const Vector2i& other) const { return { mX + other.mX, mY + other.mY }; }
	const Vector2i operator-(const Vector2i& other) const { return { mX - other.mX, mY - other.mY }; }
	const Vector2i operator*(const Vector2i& other) const { return { mX * other.mX, mY * other.mY }; }
	const Vector2i operator/(const Vector2i& other) const { return { mX / other.mX, mY / other.mY }; }

	const Vector2i operator+(const int& other) const { return { mX + other, mY + other }; }
	const Vector2i operator-(const int& other) const { return { mX - other, mY - other }; }
	const Vector2i operator*(const int& other) const { return { mX * other, mY * other }; }
	const Vector2i operator/(const int& other) const { return { mX / other, mY / other }; }

	const Vector2i operator-() const { return { -mX , -mY }; }

	Vector2i& operator+=(const Vector2i& other) { *this = *this + other; return *this; }
	Vector2i& operator-=(const Vector2i& other) { *this = *this - other; return *this; }
	Vector2i& operator*=(const Vector2i& other) { *this = *this * other; return *this; }
	Vector2i& operator/=(const Vector2i& other) { *this = *this / other; return *this; }

	Vector2i& operator+=(const int& other) { *this = *this + other; return *this; }
	Vector2i& operator-=(const int& other) { *this = *this - other; return *this; }
	Vector2i& operator*=(const int& other) { *this = *this * other; return *this; }
	Vector2i& operator/=(const int& other) { *this = *this / other; return *this; }

	      int& operator[](const int index)       { assert(index < 2 && "index out of range"); return (&mX)[index]; }
	const int& operator[](const int index) const { assert(index < 2 && "index out of range"); return (&mX)[index]; }

	      int& x()       { return mX; }
	      int& y()       { return mY; }
	const int& x() const { return mX; }
	const int& y() const { return mY; }

	static const Vector2i Zero()  { return { 0, 0 }; }
	static const Vector2i UnitX() { return { 1, 0 }; }
	static const Vector2i UnitY() { return { 0, 1 }; }

private:

	int mX;
	int mY;

};

#endif // ES_CORE_MATH_VECTOR2I_H
