#include "JsonSerializer.h"
#include "App/Application.h"

namespace Nork {
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
			.Property("quaternion", JsonArray().Elements(&rot.x, 4));
	}
	template<> Transform& JsonComponentDeserializer<Transform>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<Transform>([&](Transform& tr)
			{
				json.Get<JsonArray>("position").Get(&tr.localPosition.x, 3);
				json.Get<JsonArray>("scale").Get(&tr.localScale.x, 3);
				json.Get<JsonArray>("quaternion").Get(&tr.localQuaternion.x, 4);
				tr.RecalcModelMatrix();
			});
	}

	template<> JsonObject JsonComponentSerializer<DirLight>::Serialize(const DirLight& component)
	{
		return JsonObject();
			/*.Property("color", JsonArray().Elements(component.light->color))
			.Property("direction", JsonArray().Elements(component.light->direction))
			.Property("pos", JsonArray().Elements(component.position))
			.Property("rectangle", JsonArray().Elements(component.rectangle))
			.Property("outOfProj", component.light->outOfProjValue)
			.Property("sun", component.sun);*/
	}
	template<> DirLight& JsonComponentDeserializer<DirLight>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<DirLight>(/*[&](DirLight& comp)
			{
				json.Get<JsonArray>("color").Get(comp.light->color);
				json.Get<JsonArray>("direction").Get(comp.light->direction);
				json.Get<JsonArray>("pos").Get(comp.position);
				json.Get<JsonArray>("rectangle").Get(comp.rectangle);
				json.Get("outOfProj", comp.light->outOfProjValue);
				json.GetIfContains("sun", comp.sun);
			}*/);
	}
	template<> JsonObject JsonComponentSerializer<DirShadowMap>::Serialize(const DirShadowMap& component)
	{
		return JsonObject();
		/*
			.Property("bias", component.map.shadow->bias)
			.Property("bibiasMinas", component.map.shadow->biasMin)
			.Property("fbWidth", component.map.fb->Width())
			.Property("fbHeight", component.map.fb->Height())
			.Property("fbDepth", (int)component.map.fb->Depth()->GetAttributes().format);
			*/
	}
	template<> DirShadowMap& JsonComponentDeserializer<DirShadowMap>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<DirShadowMap>(/*[&](DirShadowMap& comp)
			{
				json.Get("bias", comp.map.shadow->bias)
					.Get("biasMin", comp.map.shadow->biasMin);
				comp.map.SetFramebuffer(json.Get<uint32_t>("fbWidth"), json.Get<uint32_t>("fbHeight"),
					(Renderer::TextureFormat)json.Get<int>("fbDepth")); 
			}*/);
	}

	template<> JsonObject JsonComponentSerializer<PointLight>::Serialize(const PointLight& component)
	{
		return JsonObject();/*
			.Property("color", JsonArray().Elements(&component.light->color.x, 3))
			.Property("position", JsonArray().Elements(&component.light->position.x, 3))
			.Property("linear", component.light->linear)
			.Property("quadratic", component.light->quadratic)
			.Property("intensity", component.GetIntensity());*/
	}
	template<> PointLight& JsonComponentDeserializer<PointLight>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<PointLight>(/*[&](PointLight& comp)
			{
				json.Get<JsonArray>("color").Get(&comp.light->color.x, 3);
				json.Get<JsonArray>("position").Get(&comp.light->position.x, 3);
				json.Get("linear", comp.light->linear);
				json.Get<float>("quadratic", comp.light->quadratic);
				comp.SetIntensity(json.Get<float>("intensity"));
			}*/);
	}
	template<> JsonObject JsonComponentSerializer<PointShadowMap>::Serialize(const PointShadowMap& component)
	{
		return JsonObject()/*
			.Property("bias", component.map.shadow->bias)
			.Property("biasMin", component.map.shadow->biasMin)
			.Property("far", component.map.shadow->far)
			.Property("near", component.map.shadow->near)
			.Property("fbSize", component.map.fb->Width())
			.Property("fbDepth", (int)component.map.fb->Depth()->GetAttributes().format)*/;
	}
	template<> PointShadowMap& JsonComponentDeserializer<PointShadowMap>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<PointShadowMap>(/*[&](PointShadowMap& comp)
			{
				json.Get("bias", comp.map.shadow->bias)
					.Get("biasMin", comp.map.shadow->biasMin)
					.Get("far", comp.map.shadow->far)
					.Get("near", comp.map.shadow->near);
				comp.map.SetFramebuffer(json.Get<uint32_t>("fbSize"), 
					(Renderer::TextureFormat)json.Get<int>("fbDepth"));
			}*/);
	}
	template<> JsonObject JsonComponentSerializer<Components::Physics>::Serialize(const Components::Physics& component)
	{
		auto& kinem = component.Kinem();
		auto kinemJson = JsonObject()
			.Property("position", JsonArray().Elements(kinem.position))
			.Property("velocity", JsonArray().Elements(kinem.velocity))
			.Property("forces", JsonArray().Elements(kinem.forces))
			.Property("quaternion", JsonArray().Elements(&kinem.quaternion.x, 4))
			.Property("w", JsonArray().Elements(kinem.w))
			.Property("torque", JsonArray().Elements(kinem.torque))
			.Property("mass", kinem.mass)
			.Property("I", kinem.I)
			.Property("friction", kinem.friction)
			.Property("elasticity", kinem.elasticity)
			.Property("isStatic", kinem.isStatic)
			.Property("applyGravity", kinem.applyGravity);

		auto& coll = component.handle.Get().localColl;
		JsonArray points;
		JsonArray edges;
		JsonArray faces;
		JsonArray facesVerts;
		for (auto& vert : coll.verts)
		{
			points.Element(JsonArray().Elements(vert));
		}
		for (auto& edge : coll.edges)
		{
			edges.Element(JsonArray().Elements(&edge.first, 2));
		}
		for (auto& face : coll.faces)
		{
			JsonObject f;
			f.Property("normal", JsonArray().Elements(face.norm));
			f.Property("vertIdx", face.vertIdx);
			faces.Element(f);
		}
		for (auto& face : coll.faceVerts)
		{
			facesVerts.Element(JsonArray().Elements(face));
		}
		JsonObject collJson = JsonObject()
			.Property("verts", points)
			.Property("edges", edges)
			.Property("faces", faces)
			.Property("faceVerts", facesVerts);

		return JsonObject()
			.Property("kinem", kinemJson)
			.Property("coll", collJson)
			.Property("size", JsonArray().Elements(component.handle.Get().size));
	}
	template<> Components::Physics& JsonComponentDeserializer<Components::Physics>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<Components::Physics>([&](Components::Physics& comp)
			{
				auto kinemComp = json.Get<JsonObject>("kinem");
				auto& kinem = comp.Kinem();
				kinemComp.Get<JsonArray>("position").Get(kinem.position);
				kinemComp.Get<JsonArray>("velocity").Get(kinem.velocity);
				kinemComp.Get<JsonArray>("forces").Get(kinem.forces);
				kinemComp.Get<JsonArray>("quaternion").Get(&kinem.quaternion.x, 4);
				kinemComp.Get<JsonArray>("w").Get(kinem.w);
				kinemComp.Get<JsonArray>("torque").Get(kinem.torque);
				kinemComp.Get("mass", kinem.mass);
				kinemComp.Get("I", kinem.I);
				kinemComp.Get("friction", kinem.friction);
				kinemComp.Get("elasticity", kinem.elasticity);
				kinemComp.Get("isStatic", kinem.isStatic);
				kinemComp.Get("applyGravity", kinem.applyGravity);

				auto collComp = json.Get<JsonObject>("coll");
				auto coll = Nork::Physics::Collider();
				auto verts = collComp.Get<JsonArray>("verts");
				auto edges = collComp.Get<JsonArray>("edges");
				auto faces = collComp.Get<JsonArray>("faces");
				auto faceVerts = collComp.Get<JsonArray>("faceVerts");
				coll.verts.resize(verts.Size());
				for (size_t i = 0; i < verts.Size(); i++)
				{
					verts.Get<JsonArray>(i).Get(coll.verts[i]);
				}
				coll.edges.resize(edges.Size());
				for (size_t i = 0; i < edges.Size(); i++)
				{
					edges.Get<JsonArray>(i).Get(&coll.edges[i].first, 2);
				}
				coll.faces.resize(faces.Size());
				for (size_t i = 0; i < faces.Size(); i++)
				{
					JsonObject face = faces.Get<JsonObject>(i);
					face.Get<JsonArray>("normal").Get(coll.faces[i].norm);
					face.Get("vertIdx", coll.faces[i].vertIdx);
				}
				coll.faceVerts.resize(faceVerts.Size());
				for (size_t i = 0; i < faceVerts.Size(); i++)
				{
					faceVerts.Get<JsonArray>(i).Get(coll.faceVerts[i]);
				}

				comp.handle.Get().localColl = coll;
				json.Get<JsonArray>("size").Get(comp.handle.Get().size);
				comp.handle.Get().UpdateCollider();
			});
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
			.Property("uri", ModelResources::Instance().Uri(component.GetModel()).string());
	}
	template<> Drawable& JsonComponentDeserializer<Drawable>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Drawable>([&](Drawable& comp)
			{
				auto id = json.Get<std::string>("uri");
				comp.SetModel(ModelResources::Instance().Get(id));
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
		ser.TrySerializeComponent<Components::Physics>();
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
				Components::Physics,
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