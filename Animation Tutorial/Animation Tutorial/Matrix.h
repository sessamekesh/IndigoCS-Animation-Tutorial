#pragma once

#include <memory>

class Matrix
{
public:
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};

		float m[4][4];
	};

public:
	Matrix();
	Matrix(const Matrix&) = default;
	Matrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
	~Matrix() = default;

	Matrix Transpose() const;
	float Determinant() const;

	Matrix operator*(const Matrix& m2) const;

public:
	static void* operator new(std::size_t size)
	{
		return _aligned_malloc(sizeof(Matrix) * size, __alignof(Matrix));
	}

	static void operator delete(void* p)
	{
		_aligned_free(p);
	}

public:
	static const Matrix Identity;
};