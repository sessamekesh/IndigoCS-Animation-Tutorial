#include "Animation.h"
#include <assert.h>

Animation::Animation(std::string name, float duration, bool loopOnFinish)
	: staticBones_()
	, animatedBones_()
	, currentTime_(0.f)
	, endTime_(duration)
	, loop_(loopOnFinish)
	, name_(name)
{}

void Animation::AddStaticBone(std::string boneName, std::string parentName, Transform transform)
{
	staticBones_.insert({ boneName, StaticBone(boneName, parentName, transform) });
}

void Animation::AddAnimatedBone(std::string boneName, std::string parentName, BoneAnimation animationData)
{
	animatedBones_.insert({ boneName, AnimatedBone(boneName, parentName, animationData) });
}

std::vector<Matrix> Animation::GetBoneMatrixArray(const std::vector<std::string>& names, const std::vector<Transform>& offsets) const
{
	std::vector<Matrix> tr;

	tr.reserve(names.size());

	for (std::uint32_t idx = 0u; idx < names.size(); idx++)
	{
		Transform animatedTransform = GetAnimatedNodeTransformAtTime(names[idx], currentTime_);
		Transform staticTransform = GetStaticNodeTransform(names[idx]);
		//tr.push_back((/* bone.OffsetMatrix * */animatedTransform * staticTransform.Inverse()).GetTransformMatrix());
		tr.push_back((animatedTransform * offsets[idx]).GetTransformMatrix());
	}

	return std::move(tr);
}

// Inherited via IActor
bool Animation::Update(float dt)
{
	currentTime_ += dt;

	// I'm not sure I like this condition, if I'm setting equality later on.
	while (currentTime_ > endTime_)
	{
		if (loop_)
		{
			currentTime_ -= endTime_;
		}
		else
		{
			currentTime_ = endTime_;
		}
	}

	return true;
}

Transform Animation::GetAnimatedNodeTransformAtTime(std::string nodeName, float time) const
{
	// If empty, assume identity transform
	if (nodeName == "") return Transform::Identity;

	if (animatedBones_.count(nodeName) > 0u)
	{
		Transform parentTransform = GetAnimatedNodeTransformAtTime(animatedBones_.at(nodeName).ParentName, time);
		Transform childTransform = animatedBones_.at(nodeName).Animation.GetTransformAtTime(time);
		return parentTransform * childTransform;
	}
	else
	{
		assert(staticBones_.count(nodeName) > 0u);
		Transform parentTransform = GetAnimatedNodeTransformAtTime(staticBones_.at(nodeName).ParentName, time);
		Transform childTransform = staticBones_.at(nodeName).FromParentTransform;
		return parentTransform * childTransform;
	}
}

Transform Animation::GetStaticNodeTransform(std::string nodeName) const
{
	if (nodeName == "") return Transform::Identity;

	assert(staticBones_.count(nodeName) > 0u);

	return GetStaticNodeTransform(staticBones_.at(nodeName).ParentName) * staticBones_.at(nodeName).FromParentTransform;
}