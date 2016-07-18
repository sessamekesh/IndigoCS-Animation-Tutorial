#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(Color ambient, Color diffuse, Color specular, Vec3 direction, float specPower)
	: AmbientColor(ambient)
	, DiffuseColor(diffuse)
	, SpecularColor(specular)
	, Direction(direction)
	, SpecularPower(specPower)
{}