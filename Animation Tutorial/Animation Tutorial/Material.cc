#include "Material.h"

Material::Material(Color ambient, Color diffuse, Color specular)
	: AmbientColor(ambient)
	, DiffuseColor(diffuse)
	, SpecularColor(specular)
{}

const Material Material::BasicGray = Material(Color::Gray, Color::Gray, Color::White);