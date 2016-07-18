#pragma once

#include "Matrix.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Color.h"

// Constants
const float PI = 3.141592653f;

// Operators
Vec3 operator*(const Vec3&, const Quaternion&);
Vec4 operator*(const Matrix&, const Vec4&);

// Helper Methods
Matrix PerspectiveLH(float fovY, float aspect, float nearZ, float farZ);
float Radians(float angle);
float Degrees(float radians);

// Conversions
