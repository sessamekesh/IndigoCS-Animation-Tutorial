#pragma once

#include "Vec3.h"
#include "Matrix.h"

struct Quaternion
{
public:
	float x, y, z, w;

public:
	Quaternion();
	Quaternion(Vec3 axis, float angle);
	Quaternion(float w, float x, float y, float z);
	Quaternion(const Quaternion&) = default;
	~Quaternion() = default;

	Quaternion Inverse() const;

	Quaternion operator*(const Quaternion&) const;
	Quaternion& operator*=(const Quaternion&);

	static Quaternion FromMatrix(const Matrix& m);

protected:
	void Normalize();
};