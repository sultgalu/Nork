#include "JsonSerializer.h"
#include "App/Application.h"
#include "Components/All.h"

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
		auto& light = component.Data();
		return JsonObject()
			.Property("color", JsonArray().Elements(light->color))
			.Property("direction", JsonArray().Elements(light->direction))
			.Property("pos", JsonArray().Elements(component.position))
			.Property("rectangle", JsonArray().Elements(component.rectangle))
			.Property("outOfProj", light->outOfProjValue)
			.Property("sun", component.sun);
	}
	template<> DirLight& JsonComponentDeserializer<DirLight>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<DirLight>([&](DirLight& comp)
		{
				auto light = comp.Data();
				json.Get<JsonArray>("color").Get(light->color);
				json.Get<JsonArray>("direction").Get(light->direction);
				json.Get<JsonArray>("pos").Get(comp.position);
				json.Get<JsonArray>("rectangle").Get(comp.rectangle);
				json.Get("outOfProj", light->outOfProjValue);
				json.GetIfContains("sun", comp.sun);
			});
	}
	template<> JsonObject JsonComponentSerializer<DirShadowMap>::Serialize(const DirShadowMap& component)
	{
		auto& shadMap = component.shadowMap;
		auto& shadow = component.shadowMap->Shadow();
		return JsonObject()
			.Property("bias", shadow->bias)
			.Property("biasMin", shadow->biasMin)
			.Property("fbWidth", shadMap->fb->Width())
			.Property("fbHeight", shadMap->fb->Height());
	}
	template<> DirShadowMap& JsonComponentDeserializer<DirShadowMap>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<DirShadowMap>([&](DirShadowMap& comp)
			{
				auto& shadMap = comp.shadowMap;
				auto& shadow = comp.shadowMap->Shadow();
				json.Get("bias", shadow->bias)
					.Get("biasMin", shadow->biasMin);
				// TODO: createTexture gets called upon component creation as well
				shadMap->CreateTexture(json.Get<uint32_t>("fbWidth"), json.Get<uint32_t>("fbHeight"));
			});
	}

	template<> JsonObject JsonComponentSerializer<PointLight>::Serialize(const PointLight& component)
	{
		auto& light = component.Data();
		return JsonObject()
			.Property("color", JsonArray().Elements(light->color))
			.Property("position", JsonArray().Elements(light->position))
			.Property("linear", light->linear)
			.Property("quadratic", light->quadratic)
			.Property("intensity", component.GetIntensity());
	}
	template<> PointLight& JsonComponentDeserializer<PointLight>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<PointLight>([&](PointLight& comp)
			{
				auto light = comp.Data();
				json.Get<JsonArray>("color").Get(light->color);
				json.Get<JsonArray>("position").Get(light->position);
				json.Get("linear", light->linear);
				json.Get<float>("quadratic", light->quadratic);
				comp.SetIntensity(json.Get<float>("intensity"));
			});
	}
	template<> JsonObject JsonComponentSerializer<PointShadowMap>::Serialize(const PointShadowMap& component)
	{
		auto& shadMap = component.shadowMap;
		auto& shadow = component.shadowMap->Shadow();
		return JsonObject()
			.Property("bias", shadow->bias)
			.Property("biasMin", shadow->biasMin)
			.Property("far", shadow->far)
			.Property("near", shadow->near)
			.Property("fbSize", shadMap->fb->Width());
	}
	template<> PointShadowMap& JsonComponentDeserializer<PointShadowMap>::Deserialize(const JsonObject& json)
	{
		return entity.AddComponent<PointShadowMap>([&](PointShadowMap& comp)
			{
				auto& shadMap = comp.shadowMap;
				auto& shadow = comp.shadowMap->Shadow();
				json.Get("bias", shadow->bias)
					.Get("biasMin", shadow->biasMin)
					.Get("far", shadow->far)
					.Get("near", shadow->near);
				shadMap->CreateTexture(json.Get<uint32_t>("fbSize"));
			});
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

		JsonArray collsJson = JsonArray();
		for (auto& collNode : component.handle.Get().colliders) {

			auto& coll = collNode.local;
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
				.Property("faceVerts", facesVerts)
				.Property("offset", JsonArray().Elements(collNode.offset));
			collsJson.Element(collJson);
		}

		return JsonObject()
			.Property("kinem", kinemJson)
			.Property("coll", collsJson)
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

				auto colls = json.Get<JsonArray>("coll");
				comp.handle.Get().colliders.clear();

				for (auto& collComp : colls.Get<JsonObject>()) {
					auto& colliderNode = comp.handle.Get().colliders.emplace_back();
					collComp.Get<JsonArray>("offset").Get(colliderNode.offset);
					auto& coll = colliderNode.local;

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

					json.Get<JsonArray>("size").Get(comp.handle.Get().size);
					comp.handle.Get().OnLocalColliderChanged();
					comp.handle.Get().UpdateGlobalColliders();
				}
			});
	}

	template<> JsonObject JsonComponentSerializer<Camera>::Serialize(const Camera& component)
	{
		return JsonObject()
			.Property("position", JsonArray().Elements(&component.position.x, 3))
			.Property("up", JsonArray().Elements(&component.up.x, 3))
			.Property("farClip", component.farClip)
			.Property("FOV", component.FOV)
			.Property("nearClip", component.nearClip)
			.Property("pitch", component.pitch)
			.Property("ratio", component.ratio)
			.Property("yaw", component.yaw);
	}
	template<> Camera& JsonComponentDeserializer<Camera>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Camera>([&](Camera& comp)
			{
				json.Get<JsonArray>("position").Get(&comp.position.x, 3);
				json.Get<JsonArray>("up").Get(&comp.up.x, 3);
				json.Get("farClip", comp.farClip);
				json.Get("FOV", comp.FOV);
				json.Get("nearClip", comp.nearClip);
				json.Get("pitch", comp.pitch);
				json.Get("ratio", comp.ratio);
				json.Get("yaw", comp.yaw);
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
			.Property("uri", Renderer::AssetLoader::Instance().Uri(component.object->GetModel()).string());
	}
	template<> Drawable& JsonComponentDeserializer<Drawable>::Deserialize(const JsonObject& json)
	{
		auto& comp = entity.AddComponent<Drawable>([&](Drawable& comp)
			{
				auto uri = json.Get<std::string>("uri");
				try {
					comp.object->SetModel(Renderer::AssetLoader::Instance().LoadModel(uri));
				} catch (std::exception& e) {
					Logger::Error("Failed to set drawable model to uri ", uri, "\n\t", e.what());
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
		ser.TrySerializeComponent<DirShadowMap>();
		ser.TrySerializeComponent<PointShadowMap>();
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
				DirShadowMap,
				PointShadowMap,
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