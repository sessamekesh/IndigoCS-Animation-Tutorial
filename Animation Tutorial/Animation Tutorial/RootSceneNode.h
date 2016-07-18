#pragma once

#include "ISceneNode.h"
#include <vector>

class RootSceneNode : public ISceneNode
{
public:
	RootSceneNode();

	// Inherited via ISceneNode
	virtual bool Update(float dt) override;
	virtual bool Render() override;

	std::vector<std::shared_ptr<ISceneNode>>& Children() { return children_; }
	void AddChild(std::shared_ptr<ISceneNode> newChild) { children_.push_back(newChild); }

private:
	std::vector<std::shared_ptr<ISceneNode>> children_;
};