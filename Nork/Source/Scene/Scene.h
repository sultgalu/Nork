#pragma once

#include "Components/All.h"

namespace Nork
{
	template<typename T>
	concept DefaultEmplaceable = true
		&& std::_Not_same_as<T, Components::Model>;

	template<typename T>
	concept DefaultRemovable = true
		&& std::_Not_same_as<T, Components::Model>;

	class Entity
	{
	public:
		Entity(entt::entity id, entt::registry& registry)
			: id(id), registry(registry)
		{}
		template<DefaultEmplaceable T, class... A>
		inline T& AddComponent(A... args)
		{
			return registry.emplace<T>(id, args...);
		}
		Components::Model& AddModel()
		{
			return registry.emplace<Components::Model>(id, GetDefaultCube());
		}
		template<DefaultRemovable T>
		inline bool RemoveComponent()
		{
			return registry.remove<T>(id) == 1;
		}
		template<std::same_as<Components::Model> T>
		inline bool RemoveComponent()
		{
			return registry.remove<Components::Model>(id) == 1;
		}
		template<class T>
		inline T& GetComponent()
		{
			return registry.get<T>(id);
		}
		template<class T>
		inline T* TryGetComponent()
		{
			return registry.try_get<T>(id);
		}
		template<class T>
		inline const T& GetComponent() const
		{
			return registry.get<T>(id);
		}
		template<class T>
		inline const T* TryGetComponent() const
		{
			return registry.try_get<T>(id);
		}
		template<class... T>
		inline bool HasAnyComponentsOf() const
		{
			return registry.any_of<T...>(id);
		}
		template<class... T>
		inline bool HasAllComponentsOf() const
		{
			return registry.all_of<T...>(id);
		}
		entt::entity Id() const { return id; }
	private:
		static Components::Model GetDefaultCube()
		{
			static auto cube = CreateDefaultCube();
			return cube;
		}
		static Components::Model CreateDefaultCube();

	private:
		entt::entity id;
		entt::registry& registry;
	};

	class SceneNode
	{
	public:
		void ForEachChildren(std::function<void(SceneNode&)> f)
		{
			for (size_t i = 0; i < children.size(); i++)
			{
				f(*children[i]);
				children[i]->ForEachChildren(f);
			}
		}

		SceneNode& AddChild(std::shared_ptr<SceneNode> node)
		{
			children.push_back(node);
			if (node->HasParent())
			{
				node->parent->RemoveChild(*node); // this can't happen before adding to our children (would create invalid reference)
			}
			node->parent = this;
			return *node;
		}

		SceneNode& AddChild(Entity entity)
		{
			children.push_back(std::make_shared<SceneNode>(entity, *this));
			return *children.back();
		}

		bool RemoveChild(SceneNode& node)
		{
			for (size_t i = 0; i < children.size(); i++)
			{
				if (*children[i] == node)
				{
					children.erase(children.begin() + i);
					return true;
				}
			}
			return false;
		}

		bool HasParent() { return parent != nullptr; }

		bool operator==(const SceneNode& other)
		{
			return other.entity.Id() == entity.Id();
		}

		Entity& GetEntity() { return entity; }
		const Entity& GetEntity() const { return entity; }
		SceneNode& GetParent() const { return *parent; }
		const std::vector<std::shared_ptr<SceneNode>>& GetChildren() const { return children; }
		std::vector<std::shared_ptr<SceneNode>>& GetChildren() { return children; }

		SceneNode(Entity entity, SceneNode& parent)
			: parent(&parent), entity(entity)
		{}
		static SceneNode CreateRoot(Entity entity)
		{
			SceneNode root(entity);
			root.parent = nullptr;
			return root;
		}
	private:
		SceneNode(Entity entity) : entity(entity)
		{}
	private:
		Entity entity;
		SceneNode* parent;
		std::vector<std::shared_ptr<SceneNode>> children = {};
	};

	typedef uint64_t uuid;
	class Scene
	{
	public:
		Scene(): registry(), root(SceneNode::CreateRoot(Entity(registry.create(), registry)))
		{
			mainCameraNode = &CreateNode();
			mainCameraNode->GetEntity().AddComponent<Components::Camera>();
			root.GetEntity().AddComponent<Components::Tag>().tag = "root";
		}
		SceneNode& CreateNode()
		{
			Entity entity(registry.create(), registry);
			return root.AddChild(entity);
		}
		SceneNode& CreateNode(SceneNode& parent)
		{
			Entity entity(registry.create(), registry);
			return parent.AddChild(entity);
		}
		void DeleteNode(SceneNode& node)
		{
			for (size_t i = 0; i < node.GetChildren().size(); i++)
			{
				node.GetParent().AddChild(node.GetChildren()[i]);
			}
			registry.destroy(node.GetEntity().Id());
			node.GetParent().RemoveChild(node);
		}
		/*Components::Model& AddModel(entt::entity id, std::string src = "")
		{
			ownedModels[id] = src;
			return registry.emplace<Components::Model>(id, GetModelByResource(
				src._Equal("") ? resMan.GetCube() : resMan.GetMeshes(src)));
		}*/
		void Load(std::string path);
		void Save(std::string path);
		inline void Reset()
		{
			registry = entt::registry();
		}
		inline Components::Camera& GetMainCamera()
		{
			return mainCameraNode->GetEntity().GetComponent<Components::Camera>();
		}
	private:
	public:
		entt::registry registry;
		SceneNode root;
		SceneNode* mainCameraNode;
	};
}