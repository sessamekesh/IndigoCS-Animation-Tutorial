#include "maffs.h"

#include <DirectXMath.h> // TODO KAM: Remove dependency on DirectXMath

// http://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
Vec3 operator*(const Vec3 &v, const Quaternion &q)
{
	Vec3 u(q.x, q.y, q.z);

	float s = q.w;

	return u * 2.f * Vec3::Dot(u, v)
		+ v * (s * s - Vec3::Dot(u, u))
		+ Vec3::Cross(u, v) * 2.f * s;
}

Vec4 operator*(const Matrix & m, const Vec4 & v)
{
	return Vec4(
		m._11 * v.x + m._12 * v.y + m._13 * v.z + m._14 * v.w,
		m._21 * v.x + m._22 * v.y + m._23 * v.z + m._24 * v.w,
		m._31 * v.x + m._32 * v.y + m._33 * v.z + m._34 * v.w,
		m._41 * v.x + m._42 * v.y + m._43 * v.z + m._44 * v.w
		);
}

Matrix PerspectiveLH(float fovY, float aspect, float nearZ, float farZ)
{
	DirectX::XMFLOAT4X4 tr;
	Matrix m;
	DirectX::XMStoreFloat4x4(&tr, DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
	
	for (std::uint8_t i = 0u; i < 4u; i++)
	{
		for (std::uint8_t j = 0u; j < 4u; j++)
		{
			m.m[i][j] = tr.m[i][j];
		}
	}

	return m;
}

float Radians(float angle)
{
	return angle * PI / 180.f;
}

float Degrees(float radians)
{
	return radians * 180.f / PI;
}