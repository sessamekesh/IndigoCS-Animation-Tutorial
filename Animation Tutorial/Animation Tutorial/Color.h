#pragma once

class Color
{
public:
	Color(float x, float y, float b, float a)
		: r(x), g(y), z(b), w(a)
	{}
	Color() = delete;
	Color(const Color&) = default;

	const static Color White;
	const static Color Black;
	const static Color Marina;
	const static Color Gray;

	Color operator*(float) const;

public:
	union { float r; float x; };
	union { float g; float y; };
	union { float b; float z; };
	union { float a; float w; };
};