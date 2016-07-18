#pragma once

#include "IRenderable.h"
#include "IActor.h"
#include "Transform.h"

class ISceneNode : public IActor, public IRenderable
{
public:
	ISceneNode()
		: transform_()
	{}

	ISceneNode(Transform transform)
		: transform_(transform)
	{}

	virtual bool Update(float dt) = 0;
	virtual bool Render() = 0;

protected:
	Transform transform_;
};