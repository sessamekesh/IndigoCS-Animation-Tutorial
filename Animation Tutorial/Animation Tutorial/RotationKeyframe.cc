#include "RotationKeyframe.h"

Quaternion RotationKeyframe::LERP(const RotationKeyframe & kf1, const RotationKeyframe & kf2, float time)
{
	float ratio = (time - kf1.Time) / (kf2.Time - kf1.Time);

	// TODO KAM: Not sure if LERP is appropriate here
	return std::move(
		Quaternion(
			(1.f - ratio) * kf1.Rotation.w + ratio * kf2.Rotation.w,
			(1.f - ratio) * kf1.Rotation.x + ratio * kf2.Rotation.x,
			(1.f - ratio) * kf1.Rotation.y + ratio * kf2.Rotation.y,
			(1.f - ratio) * kf1.Rotation.z + ratio * kf2.Rotation.z
			)
		);
}
