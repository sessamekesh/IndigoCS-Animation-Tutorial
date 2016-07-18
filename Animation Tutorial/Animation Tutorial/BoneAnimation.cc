#include "BoneAnimation.h"
#include <algorithm>
#include "Logger.h"

BoneAnimation::BoneAnimation(std::vector<PositionKeyframe> positions, std::vector<RotationKeyframe> rotations, std::vector<ScaleKeyframe> scales)
	: positions_(positions)
	, rotations_(rotations)
	, scales_(scales)
{
	std::sort(positions_.begin(), positions_.end(), [](PositionKeyframe kf1, PositionKeyframe kf2) { return kf1.Time < kf2.Time; });
	std::sort(rotations_.begin(), rotations_.end(), [](RotationKeyframe kf1, RotationKeyframe kf2) { return kf1.Time < kf2.Time; });
	std::sort(scales_.begin(), scales_.end(), [](ScaleKeyframe kf1, ScaleKeyframe kf2) { return kf1.Time < kf2.Time; });
}

Transform BoneAnimation::GetTransformAtTime(float time) const
{
	Vec3 pos;
	Quaternion rot;
	Vec3 scl;

	if (time <= positions_.front().Time)
	{
		pos = positions_.front().Translation;
	}
	else if (time >= positions_.back().Time)
	{
		pos = positions_.back().Translation;
	}
	else
	{
		// Find interpolated position
		std::uint32_t idx = 0u;
		while (idx < positions_.size() - 1u
			&& positions_[idx].Time <= time
			&& positions_[idx + 1].Time <= time)
		{
			idx++;
		}
		
		pos = PositionKeyframe::LERP(positions_[idx], positions_[idx + 1u], time);
	}

	if (time <= rotations_.front().Time)
	{
		rot = rotations_.front().Rotation;
	}
	else if (time >= rotations_.back().Time)
	{
		rot = rotations_.back().Rotation;
	}
	else
	{
		// Find interpolated rotation
		std::uint32_t idx = 0u;
		while (idx < rotations_.size() - 1u
			&& rotations_[idx].Time <= time
			&& rotations_[idx + 1].Time <= time)
		{
			idx++;
		}

		rot = RotationKeyframe::LERP(rotations_[idx], rotations_[idx + 1u], time);
	}

	if (time <= scales_.front().Time)
	{
		scl = scales_.front().Scale;
	}
	else if (time >= scales_.back().Time)
	{
		scl = scales_.back().Scale;
	}
	else
	{
		// Find interpolated scaling
		std::uint32_t idx = 0u;
		while (idx < scales_.size() - 1u
			&& scales_[idx].Time <= time
			&& scales_[idx + 1].Time <= time)
		{
			idx++;
		}

		scl = ScaleKeyframe::LERP(scales_[idx], scales_[idx + 1u], time);
	}

	return std::move(Transform(pos, rot, scl));
}