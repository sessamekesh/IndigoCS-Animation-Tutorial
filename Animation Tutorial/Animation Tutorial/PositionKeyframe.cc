#include "PositionKeyframe.h"

Vec3 PositionKeyframe::LERP(const PositionKeyframe& kf1, const PositionKeyframe& kf2, float time)
{
	float ratio = (time - kf1.Time) / (kf2.Time - kf1.Time);

	// TODO KAM: Not sure if std::move is appropriate here
	return std::move(kf1.Translation * (1.f - ratio) + kf2.Translation * ratio);
}