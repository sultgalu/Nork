#pragma once

#include "Entity.h"

namespace Nork {

	class SceneNode
	{
	public:
		SceneNode(Entity entity, SceneNode& parent)
			: parent(&parent), entity(entity)
		{}
		SceneNode(Entity entity) : entity(entity), parent(nullptr)
		{}

		std::shared_ptr<SceneNode> AddChild(Entity entity);
		void AddChild(std::shared_ptr<SceneNode> node);
		bool RemoveChild(SceneNode& node);
		void ForEachDescendants(std::function<void(SceneNode&)> f);

		bool operator==(const SceneNode& other) const
		{
			return other.entity.Id() == entity.Id();
		}

		bool HasParent() { return parent != nullptr; }
		Entity& GetEntity() { return entity; }
		const Entity& GetEntity() const { return entity; }
		SceneNode& GetParent() const { return *parent; }
		const std::vector<std::shared_ptr<SceneNode>>& GetChildren() const { return children; }
		std::vector<std::shared_ptr<SceneNode>>& GetChildren() { return children; }
	private:
		Entity entity;
		SceneNode* parent;
		std::vector<std::shared_ptr<SceneNode>> children = {};
	};
}