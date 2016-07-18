#pragma once

#include "Color.h"

struct Material
{
public:
	Color AmbientColor;
	Color DiffuseColor;
	Color SpecularColor;

public:
	Material(Color ambient, Color diffuse, Color specular);

public:
	const static Material BasicGray;
};