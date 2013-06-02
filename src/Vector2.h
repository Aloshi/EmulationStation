#ifndef _VECTOR2_H_
#define _VECTOR2_H_

//Taken from the SFML Vector2 class:
//https://github.com/LaurentGomila/SFML/blob/master/include/SFML/System/Vector2.hpp
//https://github.com/LaurentGomila/SFML/blob/master/include/SFML/System/Vector2.inl

template <typename T>
class Vector2
{
public:
	Vector2() : x(0), y(0)
	{
	}

	Vector2(T X, T Y) : x(X), y(Y)
	{
	}

	//convert between vector types
	template <typename U>
    explicit Vector2(const Vector2<U>& vector) : x(static_cast<T>(vector.x)), y(static_cast<T>(vector.y))
	{
	}
	
	T x;
	T y;
};


template <typename T>
Vector2<T> operator -(const Vector2<T>& right)
{
	return Vector2<T>(-right.x, -right.y);
}

template <typename T>
Vector2<T>& operator +=(Vector2<T>& left, const Vector2<T>& right)
{
	left.x += right.x;
	left.y += right.y;

	return left;
}

template <typename T>
Vector2<T>& operator -=(Vector2<T>& left, const Vector2<T>& right)
{
	left.x -= right.x;
	left.y -= right.y;

	return left;
}

template <typename T>
Vector2<T> operator +(const Vector2<T>& left, const Vector2<T>& right)
{
	return Vector2<T>(left.x + right.x, left.y + right.y);
}

template <typename T>
Vector2<T> operator -(const Vector2<T>& left, const Vector2<T>& right)
{
	return Vector2<T>(left.x - right.x, left.y - right.y);
}

template <typename T>
Vector2<T> operator *(const Vector2<T>& left, T right)
{
	return Vector2<T>(left.x * right, left.y * right);
}

template <typename T>
Vector2<T>& operator *=(Vector2<T>& left, T right)
{
	left.x *= right;
	left.y *= right;

	return left;
}

template <typename T>
bool operator ==(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x == right.x && left.y == right.y);
}

template <typename T>
bool operator !=(const Vector2<T>& left, const Vector2<T>& right)
{
	return (left.x != right.x) || (left.y != right.y);
}

typedef Vector2<int> Vector2i;
typedef Vector2<unsigned int> Vector2u;
typedef Vector2<float> Vector2f;

#endif
