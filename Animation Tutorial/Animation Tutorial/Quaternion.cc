#include "Quaternion.h"
#include <algorithm>

Quaternion::Quaternion() : x(0.f), y(0.f), z(0.f), w(1.f)
{}

Quaternion::Quaternion(Vec3 axis, float angle)
	: w(cosf(angle / 2.f))
	, x(sinf(angle / 2.f) * axis.x)
	, y(sinf(angle / 2.f) * axis.y)
	, z(sinf(angle / 2.f) * axis.z)
{
	Normalize();
}

Quaternion::Quaternion(float W, float X, float Y, float Z)
	: w(W)
	, x(X)
	, y(Y)
	, z(Z)
{
	Normalize();
}

Quaternion Quaternion::Inverse() const
{
	return Quaternion(-w, x, y, z);
}

Quaternion Quaternion::operator*(const Quaternion& o) const
{
	// TODO KAM: Is this math right? I'm not sure.
	return Quaternion(
		o.w * w - o.x * x - o.y * y - o.z * z,
		o.w * x + o.x * w - o.y * z + o.z * y,
		o.w * y + o.x * z + o.y * w - o.z * x,
		o.w * z - o.x * y + o.y * x + o.z * w);
}

Quaternion& Quaternion::operator*=(const Quaternion& o)
{
	return *this = *this * o;
}

// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
Quaternion Quaternion::FromMatrix(const Matrix & m)
{
	float sx = Vec3(m._11, m._21, m._31).Magnitude();
	float sy = Vec3(m._12, m._22, m._32).Magnitude();
	float sz = Vec3(m._13, m._23, m._33).Magnitude();

	float w = sqrtf(std::max(0.f, 1.f + m._11 / sx + m._22 / sy + m._33 / sz)) / 2.f;
	float x = std::copysignf(sqrtf(std::max(0.f, 1.f + m._11 / sx - m._22 / sy - m._33 / sz)) / 2.f, m._32 - m._23);
	float y = std::copysignf(sqrtf(std::max(0.f, 1.f - m._11 / sx + m._22 / sy - m._33 / sz)) / 2.f, m._13 - m._31);
	float z = std::copysignf(sqrtf(std::max(0.f, 1.f - m._11 / sx - m._22 / sy + m._33 / sz)) / 2.f, m._21 - m._12);

	return Quaternion(w, x, y, z);

	//float trace = m._11 + m._22 + m._33;
	//if (trace > 0.f)
	//{
	//	float s = 0.5f / sqrtf(trace + 1.f);
	//	return Quaternion(0.25f / s, (m._32 - m._23) * s, (m._13 - m._31) * s, (m._21 - m._12) * s);
	//}
	//else
	//{
	//	if (m._11 > m._22 && m._11 > m._33)
	//	{
	//		float s = 2.f * sqrtf(1.f + m._11 - m._22 - m._33);
	//		return Quaternion((m._32 - m._23) / s, 0.25f * s, (m._12 + m._21) / s, (m._13 + m._31) / s);
	//	}
	//	else if (m._22 > m._33)
	//	{
	//		float s = 2.f * sqrtf(1.f + m._22 - m._11 - m._33);
	//		return Quaternion((m._13 - m._31) / s, (m._12 + m._21) / s, 0.25f * s, (m._23 + m._32) / s);
	//	}
	//	else
	//	{
	//		float s = 2.f * sqrtf(1.f + m._33 - m._11 - m._22);
	//		return Quaternion((m._21 - m._12) / s, (m._13 + m._31) / s, (m._23 + m._32) / s, 0.25f * s);
	//	}
	//}
}

void Quaternion::Normalize()
{
	float mag = sqrt(x * x + w * w + y * y + z * z);
	x /= mag;
	y /= mag;
	z /= mag;
	w /= mag;
}