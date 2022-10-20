module Nork.Scene; 

#include "Core/ResourceManager.h"
import Application;

namespace Nork {

	static ResourceManager GetResMan()
	{
		return Application::Get().engine.resourceManager;
	}
	// --------------COMPONENT SERIALIZATION--------------

	template<class T>
	class JsonComponentSerializer
	{
	public:
		JsonComponentSerializer(const Entity& entity)
			:entity(entity)
		{}
		JsonObject Serialize(const T& component);
	private:
		const Entity& entity;
	};

	template<class T>
	class JsonComponentDeserializer
	{
	public:
		JsonComponentDeserializer(Entity& entity)
			:entity(entity)
		{}
		T& Deserialize(const JsonObject& json);
	private:
		Entity& entity;
	};

	using namespace Components;
	template<> JsonObject JsonComponentSerializer<Transform>::Serialize(const Transform& component)
	{
		glm::vec3 pos = component.localPosition, scale = component.localScale;
		glm::quat rot = component.localQuaternion;
		return JsonObject()
			.Property("position", JsonArray().Elements(&pos.x, 3))
			.Property("scale", JsonArray().Elements(&scale.x, 3))
			.Property("quaternion", JsonArray().Elements(&rot.w, 4));
	}
	template<> Transform& JsonComponentDeserializer<Transform>::Deserialize(const JsonObject& json)
	{
		auto& tr = entity.AddComponent<Transform>([&](Transform& tr)
			{
				json.Get<JsonArray>("position").Get(&tr.localPosition.x, 3);
				json.Get<JsonArray>("scale").Get(&tr.localScale.x, 3);
				json.Get<JsonArray>("quaternion").Get(&tr.localQuaternion.w, 4);
				tr.RecalcModelMatrix();
			});
		return tr;
	}

	template<> JsonObject JsonComponentSerializer<DirLight>::Serialize(const DirLight& component)
	{
		auto json = JsonObject()
			.Property("color", JsonArray().Elements(&component.light->color.x, 3))
			.Property("direction", JsonArray().Elements(&component.light->direction.x, 3))
			.Property("outOfProj", component.light->outOfProjValue)
			.Property("sun", component.sun);
		if (component.shadow != nullptr)
		{
			json.Property("shadow", JsonObject()
				.Property("bias", component.shadow->bias)
				.Property("biasMin", component.shadow->biasMin));
		}
		return json;
	}
	template<> DirLight& JsonComponentDeserializer<DirLight>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<DirLight>([&](DirLight& comp)
			{
				json.Get<JsonArray>("color").Get(&comp.light->color.x, 3);
				json.Get<JsonArray>("direction").Get(&comp.light->direction.x, 3);
				json.Get("outOfProj", comp.light->outOfProjValue);
				json.GetIfContains("sun", comp.sun);
				if (json.Contains("shadow"))
				{
					entity.AddComponent<DirShadowRequest>();
					json.Get<JsonObject>("shadow")
						.Get("bias", comp.shadow->bias)
						.Get("biasMin", comp.shadow->biasMin);
					comp.shadow->Update();
				}
				comp.RecalcVP();
				comp.light->Update();
			});
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<PointLight>::Serialize(const PointLight& component)
	{
		auto json = JsonObject()
			.Property("color", JsonArray().Elements(&component.light->color.x, 3))
			.Property("position", JsonArray().Elements(&component.light->position.x, 3))
			.Property("linear", component.light->linear)
			.Property("quadratic", component.light->quadratic)
			.Property("intensity", component.GetIntensity());
		if (component.shadow != nullptr)
		{
			json.Property("shadow", JsonObject()
				.Property("bias", component.shadow->bias)
				.Property("biasMin", component.shadow->biasMin)
				.Property("far", component.shadow->far)
				.Property("near", component.shadow->near));
		}
		return json;
	}
	template<> PointLight& JsonComponentDeserializer<PointLight>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<PointLight>([&](PointLight& comp)
			{
				json.Get<JsonArray>("color").Get(&comp.light->color.x, 3);
				json.Get<JsonArray>("position").Get(&comp.light->position.x, 3);
				json.Get("linear", comp.light->linear);
				json.Get<float>("quadratic", comp.light->quadratic);
				comp.SetIntensity(json.Get<float>("intensity"));
				comp.light->Update();
				if (json.Contains("shadow"))
				{
					entity.AddComponent<PointShadowRequest>();
					json.Get<JsonObject>("shadow")
						.Get("bias", comp.shadow->bias)
						.Get("biasMin", comp.shadow->biasMin)
						.Get("far", comp.shadow->far)
						.Get("near", comp.shadow->near);
					comp.shadow->Update();
				}
			});
		return comp;
	}
	template<> JsonObject JsonComponentSerializer<Kinematic>::Serialize(const Kinematic& component)
	{
		return JsonObject()
			.Property("forces", JsonArray().Elements(&component.forces.x, 3))
			.Property("velocity", JsonArray().Elements(&component.velocity.x, 3))
			.Property("w", JsonArray().Elements(&component.w.x, 3))
			.Property("mass", component.mass);
	}
	template<> Kinematic& JsonComponentDeserializer<Kinematic>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Kinematic>([&](Kinematic& comp)
			{
				json.Get<JsonArray>("forces").Get(&comp.forces.x, 3);
				json.Get<JsonArray>("velocity").Get(&comp.velocity.x, 3);
				json.Get<JsonArray>("w").Get(&comp.w.x, 3);
				json.Get("mass", comp.mass);
			});
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<Camera>::Serialize(const Camera& component)
	{
		return JsonObject()
			.Property("position", JsonArray().Elements(&component.position.x, 3))
			.Property("up", JsonArray().Elements(&component.up.x, 3))
			.Property("farClip", component.farClip)
			.Property("FOV", component.FOV)
			.Property("moveSpeed", component.moveSpeed)
			.Property("nearClip", component.nearClip)
			.Property("pitch", component.pitch)
			.Property("ratio", component.ratio)
			.Property("rotationSpeed", component.rotationSpeed)
			.Property("yaw", component.yaw)
			.Property("zoomSpeed", component.zoomSpeed);
	}
	template<> Camera& JsonComponentDeserializer<Camera>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Camera>([&](Camera& comp)
			{
				json.Get<JsonArray>("position").Get(&comp.position.x, 3);
				json.Get<JsonArray>("up").Get(&comp.up.x, 3);
				json.Get("farClip", comp.farClip);
				json.Get("FOV", comp.FOV);
				json.Get("moveSpeed", comp.moveSpeed);
				json.Get("nearClip", comp.nearClip);
				json.Get("pitch", comp.pitch);
				json.Get("ratio", comp.ratio);
				json.Get("rotationSpeed", comp.rotationSpeed);
				json.Get("yaw", comp.yaw);
				json.Get("zoomSpeed", comp.zoomSpeed);
				comp.Update();
			});
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<Tag>::Serialize(const Tag& component)
	{
		return JsonObject()
			.Property("tag", component.tag);
	}
	template<> Tag& JsonComponentDeserializer<Tag>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Tag>([&](Tag& comp)
			{
				json.Get("tag", comp.tag);
			});
		return comp;
	}
	template<> JsonObject JsonComponentSerializer<Drawable>::Serialize(const Drawable& component)
	{
		return JsonObject()
			.Property("id", *GetResMan().IdFor(component.model));
	}
	template<> Drawable& JsonComponentDeserializer<Drawable>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Drawable>([&](Drawable& comp)
			{
				auto id = json.Get<std::string>("id");
				comp.model = GetResMan().GetModel(id);
			});
		return comp;
	}
	template<> JsonObject JsonComponentSerializer<Collider>::Serialize(const Collider& component)
	{
		JsonArray points;
		JsonArray edges;
		JsonArray faces;
		for (auto& point : component.Points())
		{
			points.Element(JsonArray().Elements(&point.x, 3));
		}
		for (auto& edge : component.Edges())
		{
			edges.Element(JsonArray().Elements(&edge.first, 2));
		}
		for (auto& face : component.Faces())
		{
			JsonObject f;
			f.Property("normal", JsonArray().Elements(&face.normal.x, 3));
			f.Property("points", JsonArray().Elements(face.points));
			faces.Element(f);
		}
		return JsonObject().Property("points", points).Property("edges", edges).Property("faces", faces);
	}
	template<> Collider& JsonComponentDeserializer<Collider>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Collider>([&](Collider& comp)
			{
				auto points = json.Get<JsonArray>("points");
				auto edges = json.Get<JsonArray>("edges");
				auto faces = json.Get<JsonArray>("faces");
				for (size_t i = 0; i < points.Size(); i++)
				{
					glm::vec3 p;
					points.Get<JsonArray>(i).Get(&p.x, 3);
					comp.AddPoint(p);
				}
				for (size_t i = 0; i < edges.Size(); i++)
				{
					Collider::Edge e;
					edges.Get<JsonArray>(i).Get(&e.first, 2);
					comp.AddEdge(e.first, e.second);
				}
				for (size_t i = 0; i < faces.Size(); i++)
				{
					Collider::Face f;
					JsonObject face = faces.Get<JsonObject>(i);
					face.Get<JsonArray>("normal").Get(&f.normal.x, 3);
					face.Get<JsonArray>("points").Get(f.points);
					comp.AddFace(f);
				}
			});
		return comp;
	}

	template<class T>
	static T DeserializeComponent(const JsonObject& json)
	{
		return JsonComponentSerializer<T>::Deserialize(json);
	}

	class JsonComponentArraySerializer
	{
	public:
		JsonComponentArraySerializer(const Entity& entity)
			: array(array), entity(entity)
		{}
		template<class T>
		void TrySerializeComponent()
		{
			auto* component = entity.TryGetComponent<T>();
			if (component)
			{
				array.Element(JsonComponentSerializer<T>(entity).Serialize(*component).Property("type", typeid(T).name()));
			}
		}
		JsonArray& Get() { return array; }
	private:
		JsonArray array;
		const Entity& entity;
	};

	template<class T, class... Rest>
	static void TryDeserializeComponent(Entity& entity, const JsonObject& json)
	{
		if (json.Get<std::string>("type") == typeid(T).name())
		{
			JsonComponentDeserializer<T>(entity).Deserialize(json);
			return;
		}
		if constexpr (sizeof...(Rest) > 0)
		{
			TryDeserializeComponent<Rest...>(entity, json);
		}
	}

	// --------------------SERIALIZE-----------------------

	static JsonObject SerializeEntity(const Entity& entity)
	{
		JsonComponentArraySerializer ser(entity);
		ser.TrySerializeComponent<Tag>();
		ser.TrySerializeComponent<Transform>();
		ser.TrySerializeComponent<Drawable>();
		ser.TrySerializeComponent<DirLight>();
		ser.TrySerializeComponent<PointLight>();
		ser.TrySerializeComponent<Kinematic>();
		ser.TrySerializeComponent<Camera>();
		ser.TrySerializeComponent<Collider>();

		JsonObject json;

		json.Property("id", (size_t)entity.Id());
		json.Property("components", ser.Get());
		return json;
	}
	static JsonObject SerializeNode(SceneNode& node)
	{
		JsonObject json = SerializeEntity(node.GetEntity());
		JsonArray children;
		for (auto& child : node.GetChildren())
		{
			children.Element(SerializeNode(*child));
		}
		json.Property("children", children);
		return json;
	}
	std::string JsonSerializer::Serialize(SceneNode& node)
	{
		return SerializeNode(node).ToStringFormatted();
	}

	// --------------------DESERIALIZE-----------------------

	static Entity DeserializeEntity(const JsonObject& json, entt::registry& reg)
	{
		auto id = (entt::entity)json.Get<size_t>("id");
		auto actual = reg.create(id);
		if (id != actual)
		{
			Logger::Warning("Failed to create entity with hint=", (size_t)id, ". actual=", (size_t)actual);
		}
		Entity entity(id, reg);

		for (auto& comp : json.Get<JsonArray>("components").Get<JsonObject>())
		{
			TryDeserializeComponent<
				Transform,
				DirLight,
				PointLight,
				Kinematic,
				Tag,
				Drawable,
				Camera,
				Collider
			>(entity, comp);
		}
		return entity;
	}
	static std::shared_ptr<SceneNode> DeserializeNode(const JsonObject& json, entt::registry& reg)
	{
		auto node = std::make_shared<SceneNode>(DeserializeEntity(json, reg));

		JsonArray children = json.Get<JsonArray>("children");
		for (auto& child : children.Get<JsonObject>())
		{
			node->AddChild(DeserializeNode(child, reg));
		}
		return node;
	}
	std::shared_ptr<SceneNode> JsonSerializer::Deserialize(const std::string& json)
	{
		auto rootJson = JsonObject::ParseFormatted(json);
		return DeserializeNode(rootJson, registry);
	}
}