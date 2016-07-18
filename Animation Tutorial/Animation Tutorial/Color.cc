#include "Color.h"

const Color Color::White = Color(1.f, 1.f, 1.f, 1.f);
const Color Color::Black = Color(0.f, 0.f, 0.f, 1.f);
const Color Color::Marina = Color(84.f / 255.f, 224.f/ 255.f, 239.f / 255.f, 1.f); // http://www.colourlovers.com/palette/4219000/Across_the_Pond
const Color Color::Gray = Color(0.5f, 0.5f, 0.5f, 1.f);

Color Color::operator*(float o) const
{
	return Color(r * o, g * o, b * o, a * o);
}