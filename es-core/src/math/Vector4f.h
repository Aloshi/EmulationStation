#pragma once
#ifndef ES_CORE_MATH_VECTOR4F_H
#define ES_CORE_MATH_VECTOR4F_H

#include <assert.h>

class Vector2f;
class Vector3f;

class Vector4f
{
public:

	         Vector4f()                                                                                                                                        { }
	         Vector4f(const float f)                                              : mX(f),                 mY(f),                 mZ(f),                 mW(f) { }
	         Vector4f(const float x, const float y, const float z, const float w) : mX(x),                 mY(y),                 mZ(z),                 mW(w) { }
	explicit Vector4f(const Vector2f& v)                                          : mX(((Vector4f&)v).mX), mY(((Vector4f&)v).mY), mZ(0),                 mW(0) { }
	explicit Vector4f(const Vector2f& v, const float z)                           : mX(((Vector4f&)v).mX), mY(((Vector4f&)v).mY), mZ(z),                 mW(0) { }
	explicit Vector4f(const Vector2f& v, const float z, const float w)            : mX(((Vector4f&)v).mX), mY(((Vector4f&)v).mY), mZ(z),                 mW(w) { }
	explicit Vector4f(const Vector3f& v)                                          : mX(((Vector4f&)v).mX), mY(((Vector4f&)v).mY), mZ(((Vector4f&)v).mZ), mW(0) { }
	explicit Vector4f(const Vector3f& v, const float w)                           : mX(((Vector4f&)v).mX), mY(((Vector4f&)v).mY), mZ(((Vector4f&)v).mZ), mW(w) { }

	const bool operator==(const Vector4f& other) const { return ((mX == other.mX) && (mY == other.mY) && (mZ == other.mZ) && (mW == other.mW)); }
	const bool operator!=(const Vector4f& other) const { return ((mX != other.mX) || (mY != other.mY) || (mZ != other.mZ) || (mW != other.mW)); }

	const Vector4f operator+(const Vector4f& other) const { return { mX + other.mX, mY + other.mY, mZ + other.mZ, mW + other.mW }; }
	const Vector4f operator-(const Vector4f& other) const { return { mX - other.mX, mY - other.mY, mZ - other.mZ, mW - other.mW }; }
	const Vector4f operator*(const Vector4f& other) const { return { mX * other.mX, mY * other.mY, mZ * other.mZ, mW * other.mW }; }
	const Vector4f operator/(const Vector4f& other) const { return { mX / other.mX, mY / other.mY, mZ / other.mZ, mW / other.mW }; }

	const Vector4f operator+(const float& other) const { return { mX + other, mY + other, mZ + other, mW + other }; }
	const Vector4f operator-(const float& other) const { return { mX - other, mY - other, mZ - other, mW - other }; }
	const Vector4f operator*(const float& other) const { return { mX * other, mY * other, mZ * other, mW * other }; }
	const Vector4f operator/(const float& other) const { return { mX / other, mY / other, mZ / other, mW / other }; }

	const Vector4f operator-() const { return {-mX , -mY, -mZ, -mW }; }

	Vector4f& operator+=(const Vector4f& other) { *this = *this + other; return *this; }
	Vector4f& operator-=(const Vector4f& other) { *this = *this - other; return *this; }
	Vector4f& operator*=(const Vector4f& other) { *this = *this * other; return *this; }
	Vector4f& operator/=(const Vector4f& other) { *this = *this / other; return *this; }

	Vector4f& operator+=(const float& other) { *this = *this + other; return *this; }
	Vector4f& operator-=(const float& other) { *this = *this - other; return *this; }
	Vector4f& operator*=(const float& other) { *this = *this * other; return *this; }
	Vector4f& operator/=(const float& other) { *this = *this / other; return *this; }

	      float& operator[](const int index)       { assert(index < 4 && "index out of range"); return (&mX)[index]; }
	const float& operator[](const int index) const { assert(index < 4 && "index out of range"); return (&mX)[index]; }

	      float& x()       { return mX; }
	      float& y()       { return mY; }
	      float& z()       { return mZ; }
	      float& w()       { return mW; }
	const float& x() const { return mX; }
	const float& y() const { return mY; }
	const float& z() const { return mZ; }
	const float& w() const { return mW; }

	inline       Vector2f& v2()       { return *(Vector2f*)this; }
	inline const Vector2f& v2() const { return *(Vector2f*)this; }

	inline       Vector3f& v3()       { return *(Vector3f*)this; }
	inline const Vector3f& v3() const { return *(Vector3f*)this; }

	inline Vector4f& round() { mX = (int)(mX + 0.5f); mY = (int)(mY + 0.5f); mZ = (int)(mZ + 0.5f); mW = (int)(mW + 0.5f); return *this; }

	static const Vector4f Zero()  { return { 0, 0, 0, 0 }; }
	static const Vector4f UnitX() { return { 1, 0, 0, 0 }; }
	static const Vector4f UnitY() { return { 0, 1, 0, 0 }; }
	static const Vector4f UnitZ() { return { 0, 0, 1, 0 }; }
	static const Vector4f UnitW() { return { 0, 0, 0, 1 }; }

private:

	float mX;
	float mY;
	float mZ;
	float mW;

};

#endif // ES_CORE_MATH_VECTOR4F_H
