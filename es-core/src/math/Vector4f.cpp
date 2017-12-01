#include "math/Vector4f.h"

Vector4f& Vector4f::round()
{
	mX = (float)(int)(mX + 0.5f);
	mY = (float)(int)(mY + 0.5f);
	mZ = (float)(int)(mZ + 0.5f);
	mW = (float)(int)(mW + 0.5f);

	return *this;

} // round

Vector4f& Vector4f::lerp(const Vector4f& _start, const Vector4f& _end, const float _fraction)
{
	mX = Math::lerp(_start.x(), _end.x(), _fraction);
	mY = Math::lerp(_start.y(), _end.y(), _fraction);
	mZ = Math::lerp(_start.z(), _end.z(), _fraction);
	mW = Math::lerp(_start.w(), _end.w(), _fraction);

	return *this;

} // lerp
