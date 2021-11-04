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

	template<>
	void CustomSerializer<Poly>::Serialize(std::vector<char>& buf, Poly& poly, Scene::Scene& scene, ECS::Id id)
	{
		auto startBufSize = buf.size();
		std::vector<uint32_t> neighsData; // size:data,data,data,size:data...
		for (size_t i = 0; i < poly.neighbours.size(); i++)
		{
			neighsData.push_back(poly.neighbours[i].size());
			for (uint32_t n : poly.neighbours[i])
			{
				neighsData.push_back(n);
			}
		}

		size_t sizes[4] = {
			poly.vertices.size() * sizeof(poly.vertices[0]),
			neighsData.size() * sizeof(neighsData[0]),
			poly.triangleIndices.size() * sizeof(poly.triangleIndices[0]),
			poly.edgeIndices.size() * sizeof(poly.edgeIndices[0])
		}; 
		buf.resize(startBufSize + sizeof(sizes) + sizes[0] + sizes[1] + sizes[2] + sizes[3]);

		std::memcpy(buf.data() + startBufSize, sizes, sizeof(sizes));
		std::memcpy(buf.data() + startBufSize + sizeof(sizes), poly.vertices.data(), sizes[0]);
		std::memcpy(buf.data() + startBufSize + sizeof(sizes) 
			+ sizes[0], neighsData.data(), sizes[1]);
		std::memcpy(buf.data() + startBufSize + sizeof(sizes)
			+ sizes[0] + sizes[1], poly.triangleIndices.data(), sizes[2]);
		std::memcpy(buf.data() + startBufSize + sizeof(sizes)
			+ sizes[0] + sizes[1] + sizes[2], poly.edgeIndices.data(), sizes[3]);
	}
	template<>
	size_t CustomSerializer<Poly>::Deserialize(const char* data, Scene::Scene& scene, ECS::Id id)
	{
		size_t* sizes = ((size_t*)data);
		const char* verts = data + sizeof(size_t) * 4;
		const char* neighs = verts + sizes[0];
		std::span<uint32_t> neighsData = std::span<uint32_t>((uint32_t*)(neighs), sizes[0] / sizeof(uint32_t));
		const char* tris = neighs + sizes[1];
		const char* edges = tris + sizes[2];

		Poly& poly = scene.AddComponent<Poly>(id);

		poly.vertices.resize(sizes[0] / sizeof(poly.vertices[0]));
		poly.neighbours.resize(sizes[0] / sizeof(poly.vertices[0]));
		poly.triangleIndices.resize(sizes[2] / sizeof(poly.triangleIndices[0]));
		poly.edgeIndices.resize(sizes[3] / sizeof(poly.edgeIndices[0]));

		std::memcpy(poly.vertices.data(), verts, sizes[0]);
		//std::memcpy(poly.neighbours.data(), neighs, sizes[1]);
		std::memcpy(poly.triangleIndices.data(), tris, sizes[2]);
		std::memcpy(poly.edgeIndices.data(), edges, sizes[3]);

		for (size_t i = 0; i < neighsData.size(); i++)
		{
			uint32_t count = neighsData[i];
			for (size_t j = i + 1; j <= count; j++)
			{
				poly.neighbours[i].insert(neighsData[j]);
			}

			i += count; // + 1 the loop
		}

		return sizeof(size_t) * 4 + sizes[0] + sizes[1] + sizes[2] + sizes[3];
	}
}