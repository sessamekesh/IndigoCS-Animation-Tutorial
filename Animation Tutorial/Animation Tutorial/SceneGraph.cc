#include "SceneGraph.h"
#include "Logger.h"

SceneGraph::SceneGraph()
	: sceneRoot_()
	, sceneNodes_()
{}

bool SceneGraph::Update(float dt)
{
	return sceneRoot_.Update(dt);
}

bool SceneGraph::Render()
{
	return sceneRoot_.Render();
}

void SceneGraph::AddSceneNode(const char* nodeName, std::shared_ptr<ISceneNode> sceneNode)
{
	if (nodeName != nullptr)
	{
		sceneNodes_.emplace(std::string(nodeName), sceneNode);
	}
	
	sceneRoot_.AddChild(sceneNode);
}

std::shared_ptr<ISceneNode> SceneGraph::GetNodeByName(std::string nodeName)
{
	return sceneNodes_[nodeName];
}