#include "SceneNode.h"

namespace Nork {

	void SceneNode::ForEachDescendants(std::function<void(SceneNode&)> f)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			f(*children[i]);
			children[i]->ForEachDescendants(f);
		}
	}
	void SceneNode::AddChild(std::shared_ptr<SceneNode> node)
	{
		children.push_back(node);
		if (node->HasParent())
		{
			node->parent->RemoveChild(*node); // this can't happen before adding to our children (would create invalid reference)
		}
		node->parent = this;
	}
	std::shared_ptr<SceneNode> SceneNode::AddChild(Entity entity)
	{
		children.push_back(std::make_shared<SceneNode>(entity, *this));
		return children.back();
	}
	bool SceneNode::RemoveChild(SceneNode& node)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			if ((*children[i]) == node)
			{
				children.erase(children.begin() + i);
				return true;
			}
		}
		return false;
	}
}