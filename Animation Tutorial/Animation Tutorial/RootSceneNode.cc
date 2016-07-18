#include "RootSceneNode.h"

RootSceneNode::RootSceneNode()
	: ISceneNode()
{}

bool RootSceneNode::Update(float dt)
{
	bool isValid = true;
	for (auto child : children_)
	{
		isValid &= child->Update(dt);
	}
	return isValid;
}

bool RootSceneNode::Render()
{
	bool isValid = true;
	for (auto child : children_)
	{
		isValid &= child->Render();
	}
	return isValid;
}