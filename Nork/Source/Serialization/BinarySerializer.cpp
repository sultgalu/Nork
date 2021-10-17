#include "pch.h"
#include "BinarySerializer.h"
#include "Components/All.h"

namespace Nork::Serialization
{
	using namespace Components;

	template<>
	void CustomSerializer<Tag>::Serialize(std::vector<char>& buf, Tag& tag, Scene::Scene& scene, ECS::Id id)
	{
		auto startBufSize = buf.size();

		size_t size = tag.tag.size();
		buf.resize(startBufSize + sizeof(size_t) + size);

		*((size_t*)(buf.data() + startBufSize)) = size;
		std::memcpy(buf.data() + startBufSize + sizeof(size_t), tag.tag.data(), size);
	}
	template<>
	size_t CustomSerializer<Tag>::Deserialize(const char* data, Scene::Scene& scene, ECS::Id id)
	{
		size_t size = *((size_t*)data);
		scene.AddComponent<Tag>(id).tag = std::string(data + sizeof(size_t), size);
		return sizeof(size_t) + size;
	}

	template<>
	void CustomSerializer<Model>::Serialize(std::vector<char>& buf, Model& comp, Scene::Scene& scene, ECS::Id id)
	{
		auto startBufSize = buf.size();
		std::string& src = scene.ownedModels[id];
		Logger::Info("Serializing model with source path: ", src);

		buf.resize(startBufSize + sizeof(size_t) + src.size());

		*((size_t*)(buf.data()  + startBufSize)) = src.size();
		std::memcpy(buf.data() + startBufSize + sizeof(size_t), src.data(), src.size());
	}
	template<>
	size_t CustomSerializer<Model>::Deserialize(const char* data, Scene::Scene& scene, ECS::Id id)
	{
		size_t size = *((size_t*)data);
		std::string src = std::string(data + sizeof(size_t), size);
		scene.AddModel(id, src);

		return sizeof(size_t) + size;
	}

	template<>
	void CustomSerializer<std::string>::Serialize(std::vector<char>& buf, std::string& str, Scene::Scene& scene, ECS::Id id)
	{
		auto startBufSize = buf.size();

		size_t size = str.size();
		buf.resize(startBufSize + sizeof(size_t) + size);

		*((size_t*)(buf.data() + startBufSize)) = size;
		std::memcpy(buf.data() + startBufSize + sizeof(size_t), str.data(), size);
	}
	template<>
	size_t CustomSerializer<std::string>::Deserialize(const char* data, Scene::Scene& scene, ECS::Id id)
	{
		size_t size = *((size_t*)data);
		scene.AddComponent<std::string>(id) = std::string(data + sizeof(size_t), size);
		return sizeof(size_t) + size;
	}
}