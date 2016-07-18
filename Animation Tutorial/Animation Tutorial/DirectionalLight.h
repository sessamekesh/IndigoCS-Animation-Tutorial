#pragma once

#include "Color.h"
#include "Vec3.h"

struct DirectionalLight
{
public:
	Color AmbientColor;
	Color DiffuseColor;
	Color SpecularColor;
	Vec3 Direction;
	float SpecularPower;

public:
	DirectionalLight(Color ambient, Color diffuse, Color specular, Vec3 direction, float specPower);
	DirectionalLight(const DirectionalLight&) = default;
};