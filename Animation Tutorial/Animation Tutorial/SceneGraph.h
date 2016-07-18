#pragma once

#include "RootSceneNode.h"
#include <vector>
#include <map>
#include <memory>

class SceneGraph
{
public:
	SceneGraph();

	bool Update(float dt);
	bool Render();

	void AddSceneNode(const char* nodeName, std::shared_ptr<ISceneNode> sceneNode);
	std::shared_ptr<ISceneNode> GetNodeByName(std::string nodeName);
	RootSceneNode& GetRoot() { return sceneRoot_; }

private:
	RootSceneNode sceneRoot_;
	std::map<std::string, std::shared_ptr<ISceneNode>> sceneNodes_;
};