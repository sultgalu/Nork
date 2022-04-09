#include "JsonSerializer.h"
#include "App/Application.h"

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
		static JsonObject Serialize(const T& component);
		static T Deserialize(const JsonObject& json);
	};

	using namespace Components;
	template<> JsonObject JsonComponentSerializer<Transform>::Serialize(const Transform& component)
	{
		glm::vec3 pos = component.GetPosition(), scale = component.GetScale();
		glm::quat rot = component.GetRotation();
		return JsonObject()
			.Property("position", JsonArray().Elements(&pos.x, 3))
			.Property("scale", JsonArray().Elements(&scale.x, 3))
			.Property("quaternion", JsonArray().Elements(&rot.x, 4));
	}
	template<> Transform JsonComponentSerializer<Transform>::Deserialize(const JsonObject& json)
	{
		glm::vec3 pos, scale;
		glm::quat rot;
		json.Get<JsonArray>("position").Get(&pos.x, 3);
		json.Get<JsonArray>("scale").Get(&scale.x, 3);
		json.Get<JsonArray>("quaternion").Get(&rot.x, 4);
		return Transform().SetPosition(pos).SetScale(scale).SetRotation(rot);
	}

	template<> JsonObject JsonComponentSerializer<DirLight>::Serialize(const DirLight& component)
	{
		auto json = JsonObject()
			.Property("color", JsonArray().Elements(&component.light->color.x, 3))
			.Property("direction", JsonArray().Elements(&component.light->direction.x, 3))
			.Property("outOfProj", component.light->outOfProjValue);
		if (component.shadow != nullptr)
		{
			json.Property("shadow", JsonObject()
				.Property("", component.shadow->bias)
				.Property("", component.shadow->biasMin));
		}
		return json;
	}
	template<> DirLight JsonComponentSerializer<DirLight>::Deserialize(const JsonObject& json)
	{
		DirLight comp;
		json.Get<JsonArray>("color").Get(&comp.light->color.x, 3);
		json.Get<JsonArray>("direction").Get(&comp.light->direction.x, 3);
		json.Get("outOfProj", comp.light->outOfProjValue);
		if (json.Contains("shadow"))
		{
			Logger::Error("Deserializing Shadows is not implemented yet");
		}
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
				.Property("", component.shadow->bias)
				.Property("", component.shadow->biasMin)
				.Property("", component.shadow->far)
				.Property("", component.shadow->near));
		}
		return json;
	}
	template<> PointLight JsonComponentSerializer<PointLight>::Deserialize(const JsonObject& json)
	{
		PointLight comp;
		json.Get<JsonArray>("color").Get(&comp.light->color.x, 3);
		json.Get<JsonArray>("position").Get(&comp.light->position.x, 3);
		json.Get("linear", comp.light->linear);
		json.Get<float>("quadratic", comp.light->quadratic);
		comp.SetIntensity(json.Get<float>("intensity"));
		if (json.Contains("shadow"))
		{
			Logger::Error("Deserializing Shadows is not implemented yet");
		}
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
	template<> Kinematic JsonComponentSerializer<Kinematic>::Deserialize(const JsonObject& json)
	{
		Kinematic comp;
		json.Get<JsonArray>("forces").Get(&comp.forces.x, 3);
		json.Get<JsonArray>("velocity").Get(&comp.velocity.x, 3);
		json.Get<JsonArray>("w").Get(&comp.w.x, 3);
		json.Get("mass", comp.mass);
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
	template<> Camera JsonComponentSerializer<Camera>::Deserialize(const JsonObject& json)
	{
		Camera comp;
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
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<Tag>::Serialize(const Tag& component)
	{
		return JsonObject()
			.Property("tag", component.tag);
	}
	template<> Tag JsonComponentSerializer<Tag>::Deserialize(const JsonObject& json)
	{
		Tag comp;
		json.Get("tag", comp.tag);
		return comp;
	}
	template<> JsonObject JsonComponentSerializer<Drawable>::Serialize(const Drawable& component)
	{
		return JsonObject()
			.Property("id", *GetResMan().PathForModel(component.model.meshes[0].mesh));
	}
	template<> Drawable JsonComponentSerializer<Drawable>::Deserialize(const JsonObject& json)
	{
		auto id = json.Get<std::string>("id");
		Drawable dr;
		dr.model = GetResMan().GetModel(id);
		return dr;
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
				array.Element(JsonComponentSerializer<T>::Serialize(*component).Property("type", typeid(T).name()));
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
			entity.AddComponent<T>() = JsonComponentSerializer<T>::Deserialize(json);
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
				Camera
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