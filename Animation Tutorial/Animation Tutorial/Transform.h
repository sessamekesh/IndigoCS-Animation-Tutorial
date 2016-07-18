#pragma once

#include <memory>
#include "maffs.h" // TODO KAM: Remove circular dependency

struct Transform
{
public:
	Vec3 Pos;
	Quaternion Rotation;
	Vec3 Scale;

public:
	Transform();
	Transform(Vec3 pos, Quaternion rotation, Vec3 scale);
	Transform(const Transform&) = default;
	~Transform() = default;

	Transform operator*(const Transform& rhs) const;

	static Transform Lerp(const Transform& t1, const Transform& t2, float ratio);

public:
	Matrix GetTransformMatrix() const;
	Transform Inverse() const;
	static Transform FromTransformMatrix(const Matrix& m);
	
public:
	static const Transform Identity;
};