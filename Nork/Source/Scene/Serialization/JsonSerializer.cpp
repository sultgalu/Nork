#include "JsonSerializer.h"

#include "Modules/Serialization/Json.h"

namespace Nork {

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
			.Property("position", JsonArray().Elements((float*)&pos, 3))
			.Property("scale", JsonArray().Elements((float*)&scale, 3))
			.Property("quaternion", JsonArray().Elements((float*)&rot, 4));
	}
	template<> Transform JsonComponentSerializer<Transform>::Deserialize(const JsonObject& json)
	{
		glm::vec3 pos, scale;
		glm::quat rot;
		json.Get<JsonArray>("position").Get<float>((float*)&pos, 3);
		json.Get<JsonArray>("scale").Get<float>((float*)&scale, 3);
		json.Get<JsonArray>("quaternion").Get<float>((float*)&rot, 4);
		return Transform().SetPosition(pos).SetScale(scale).SetRotation(rot);
	}

	template<> JsonObject JsonComponentSerializer<DirLight>::Serialize(const DirLight& component)
	{
		return JsonObject()
			.Property("color", JsonArray().Elements((float*)&component.color, 3))
			.Property("direction", JsonArray().Elements((float*)&component.direction, 3));
	}
	template<> DirLight JsonComponentSerializer<DirLight>::Deserialize(const JsonObject& json)
	{
		DirLight comp;
		json.Get<JsonArray>("color").Get<float>((float*)&comp.color, 3);
		json.Get<JsonArray>("direction").Get<float>((float*)&comp.direction, 3);
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<PointLight>::Serialize(const PointLight& component)
	{
		return JsonObject()
			.Property("color", JsonArray().Elements((float*)&component.color, 3))
			.Property("position", JsonArray().Elements((float*)&component.position, 3))
			.Property("linear", component.linear)
			.Property("quadratic", component.quadratic)
			.Property("intensity", component.GetIntensity());
	}
	template<> PointLight JsonComponentSerializer<PointLight>::Deserialize(const JsonObject& json)
	{
		PointLight comp;
		json.Get<JsonArray>("color").Get<float>((float*)&comp.color, 3);
		json.Get<JsonArray>("position").Get<float>((float*)&comp.position, 3);
		comp.linear = json.Get<float>("linear");
		comp.quadratic = json.Get<float>("quadratic");
		comp.SetIntensity(json.Get<float>("intensity"));
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<DirShadow>::Serialize(const DirShadow& component)
	{
		return JsonObject()
			.Property("bias", component.bias)
			.Property("biasMin", component.biasMin)
			.Property("top", component.top)
			.Property("bottom", component.bottom)
			.Property("left", component.left)
			.Property("right", component.right)
			.Property("near", component.near)
			.Property("far", component.far)
			.Property("pcfSize", component.pcfSize)
			.Property("idx", component.idx)
			.Property("VP", JsonArray().Elements((float*)&component.VP, 4 * 4));
	}
	template<> DirShadow JsonComponentSerializer<DirShadow>::Deserialize(const JsonObject& json)
	{
		DirShadow comp;
		json.Get<JsonArray>("VP").Get<float>((float*)&comp.VP, 4 * 4);
		comp.bias = json.Get<float>("bias");
		comp.biasMin = json.Get<float>("biasMin");
		comp.top = json.Get<float>("top");
		comp.bottom = json.Get<float>("bottom");
		comp.left = json.Get<float>("left");
		comp.right = json.Get<float>("right");
		comp.far = json.Get<float>("far");
		comp.near = json.Get<float>("near");
		comp.idx = json.Get<float>("idx");
		comp.pcfSize = json.Get<float>("pcfSize");
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<Kinematic>::Serialize(const Kinematic& component)
	{
		return JsonObject()
			.Property("forces", JsonArray().Elements((float*)&component.forces, 3))
			.Property("velocity", JsonArray().Elements((float*)&component.velocity, 3))
			.Property("w", JsonArray().Elements((float*)&component.w, 3))
			.Property("mass", component.mass);
	}
	template<> Kinematic JsonComponentSerializer<Kinematic>::Deserialize(const JsonObject& json)
	{
		Kinematic comp;
		json.Get<JsonArray>("forces").Get<float>((float*)&comp.forces, 3);
		json.Get<JsonArray>("velocity").Get<float>((float*)&comp.velocity, 3);
		json.Get<JsonArray>("w").Get<float>((float*)&comp.w, 3);
		comp.mass = json.Get<float>("mass");
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<Camera>::Serialize(const Camera& component)
	{
		return JsonObject()
			.Property("position", JsonArray().Elements((float*)&component.position, 3))
			.Property("up", JsonArray().Elements((float*)&component.up, 3))
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
		json.Get<JsonArray>("position").Get<float>((float*)&comp.position, 3);
		json.Get<JsonArray>("up").Get<float>((float*)&comp.up, 3);
		comp.farClip = json.Get<float>("farClip");
		comp.FOV = json.Get<float>("FOV");
		comp.moveSpeed = json.Get<float>("moveSpeed");
		comp.nearClip = json.Get<float>("nearClip");
		comp.pitch = json.Get<float>("pitch");
		comp.ratio = json.Get<float>("ratio");
		comp.rotationSpeed = json.Get<float>("rotationSpeed");
		comp.yaw = json.Get<float>("yaw");
		comp.zoomSpeed = json.Get<float>("zoomSpeed");
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
		comp.tag = json.Get<std::string>("tag");
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<PointShadow>::Serialize(const PointShadow& component)
	{
		return JsonObject()
			.Property("bias", component.bias)
			.Property("biasMin", component.biasMin)
			.Property("near", component.near)
			.Property("far", component.far)
			.Property("blur", component.blur)
			.Property("radius", component.radius)
			.Property("idx", component.idx);
	}
	template<> PointShadow JsonComponentSerializer<PointShadow>::Deserialize(const JsonObject& json)
	{
		PointShadow comp;
		comp.bias = json.Get<float>("bias");
		comp.biasMin = json.Get<float>("biasMin");
		comp.blur = json.Get<float>("blur");
		comp.idx = json.Get<float>("idx");
		comp.far = json.Get<float>("far");
		comp.near = json.Get<float>("near");
		comp.radius = json.Get<float>("radius");
		return comp;
	}

	template<> JsonObject JsonComponentSerializer<Drawable>::Serialize(const Drawable& component)
	{
		return JsonObject()
			.Property("id", component.mesh->id);
	}
	template<> Drawable JsonComponentSerializer<Drawable>::Deserialize(const JsonObject& json)
	{
		return Drawable();
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
		ser.TrySerializeComponent<DirShadow>();
		ser.TrySerializeComponent<PointShadow>();
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
		return SerializeNode(node).ToString();
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
				DirShadow,
				PointShadow,
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
		auto rootJson = JsonObject::Parse(json);
		return DeserializeNode(rootJson, registry);
	}
}