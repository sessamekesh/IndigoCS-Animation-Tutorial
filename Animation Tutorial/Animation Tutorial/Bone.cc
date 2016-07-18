#include "Bone.h"

AnimatedBone::AnimatedBone(std::string name, std::string parentName, BoneAnimation animation)
	: Name(name)
	, ParentName(parentName)
	, Animation(animation)
{}

StaticBone::StaticBone(std::string name, std::string parentName, Transform fromParentTransform)
	: Name(name)
	, ParentName(parentName)
	, FromParentTransform(fromParentTransform)
{}