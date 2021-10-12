#include "pch.h"
#include "Serializer.h"
#include "Components/All.h"

namespace Nork::Serialization
{
	struct Header
	{
		unsigned int compID;
		int size, compCount;
		Header(unsigned int id = 0, int count = 0, int size = 0)
			: compID(id), compCount(count), size(size)
		{
		}
	};

	template<typename T>
	std::ostream& writeTo(std::ostream& s, T val)
	{
		return s.write((char*)&val, sizeof(T));
	}
	template<typename T>
	T readFrom(std::istream& s)
	{
		T val;
		s.read((char*)&val, sizeof(T));
		return val;
	}

	class __Serializer
	{
	public:
		virtual int SerializeAll(const entt::registry& reg, std::ostream& s) const = 0;
		virtual void DeserializeAll(entt::registry& reg, std::istream& s, int count) = 0;
	};

	template<typename T>
	class _Serializer : public __Serializer
	{
	public:
		int SerializeAll(const entt::registry& reg, std::ostream& s) const override // {{id,s,data}...}} -> count
		{
			int count = 0;
			reg.view<T>().each([&](auto ent, auto& comp)
				{
					writeTo(s, ent);
					int sizePtr = s.tellp();
					writeTo(s, 0);

					int begin = s.tellp();
					Serialize(comp, s);
					int end = s.tellp();

					writeTo(s.seekp(sizePtr), end - begin).seekp(end);
					count++;
				});
			return count;
		}
		void DeserializeAll(entt::registry& reg, std::istream& s, int count) override // (count){{id,s,data}...}}
		{
			for (int i = 0; i < count; i++)
			{
				auto id = readFrom<entt::entity>(s);
				if (!reg.valid(id))
					reg.create(id);

				int begin1 = s.tellg();
				auto size = readFrom<int>(s);

				int begin = s.tellg();
				Deserialize(reg.emplace<T>(id), s);
				int end = s.tellg();

				if (end - begin != size)
				{
					std::cout << "ERR::COMP SIZE NOT AS SPECIFIED: ["
						<< std::to_string(end - begin) << ":" << std::to_string(begin1) << "]\n";
					s.seekg(begin + size); // correct the problem from serializer
				}
			}

		}

		void Serialize(const T& comp, std::ostream& s) const;
		void Deserialize(T& comp, std::istream& s);
	};

	inline static std::unordered_map<unsigned int, __Serializer*> serializerMap;

	template<typename T>
	void AddSerializer(int id)
	{
		serializerMap[id] = new _Serializer<T>();
	}

	void Init()
	{
		using namespace Components;
		AddSerializer<Transform>(1);
		AddSerializer<Tag>(2);
		AddSerializer<DirLight>(3);
		AddSerializer<PointLight>(4);
		AddSerializer<Model>(5);
		AddSerializer<Camera>(6);

		Logger::Info("Serializer initialized");
	}
	void DeserializeRegistry(ECS::Registry& reg, std::istream& s) // {Count}{{comp,s,c}{{id,s,data}...}}
	{
		auto size = readFrom<int>(s);
		int count = readFrom<int>(s);
		Header header;

		int begin = s.tellg();
		for (int i = 0; i < count; i++)
		{
			header = readFrom<Header>(s);
			int begin = s.tellg();
			if (serializerMap.count(header.compID) > 0)
			{
				serializerMap[header.compID]->DeserializeAll(reg.GetUnderlyingMutable(), s, header.compCount);
				int end = s.tellg();
				if (end - begin != header.size)
				{
					std::cout << "ERR::COMPONENTs SIZE BAD: [" << std::to_string(end - begin) << ":" << std::to_string(header.size) << "]\n";
					s.seekg(begin + header.size); // correct the deserialization error. (shouldn't happen cause class already takes care of it / comp)
				}
			}
			else
			{
				std::cout << "ERR::NO SERIALIZER FOR: [" << std::to_string(header.compID) << "\n";
				s.seekg(begin + header.size); // no deser, skip these components.
			}
		}
		int end = s.tellg();
		if (end - begin != size)
		{
			std::cout << " s";
		}
	}
	void SerializeRegistry(const ECS::Registry& reg, std::ostream& s)
	{
		int count = serializerMap.size();
		int sizePtr = s.tellp();
		writeTo(s, 0);
		writeTo(s, count);

		int begin = s.tellp();
		for (auto& ser : serializerMap)
		{
			int headerP = s.tellp();
			Header header(ser.first, 0, 0);
			writeTo(s, header);

			int begin = s.tellp();
			header.compCount = ser.second->SerializeAll(reg.GetUnderlying(), s);
			int end = s.tellp();
			header.size = end - begin;

			writeTo(s.seekp(headerP), header).seekp(end);
		}
		int end = s.tellp();
		writeTo(s.seekp(sizePtr), end - begin).seekp(end);
	}

	// Use this for structs that only has primitive data, no pointers / runtime references
#define DEFINE_DEFAULT_IMPL(T) inline void _Serializer<T>::Serialize(const T& comp, std::ostream& s) const { s.write((char*)&comp, sizeof(T)); } \
	inline void _Serializer<T>::Deserialize(T& comp, std::istream& s){ s.read((char*)&comp, sizeof(T)); }

	using namespace Components;
	DEFINE_DEFAULT_IMPL(Transform);
	DEFINE_DEFAULT_IMPL(Camera);
	DEFINE_DEFAULT_IMPL(DirLight);
	DEFINE_DEFAULT_IMPL(PointLight);

	void _Serializer<Tag>::Serialize(const Tag& tag, std::ostream& s) const
	{
		int size = tag.tag.size();
		writeTo(s, size);
		s.write(tag.tag.c_str(), size);
	}
	void _Serializer<Tag>::Deserialize(Tag& tag, std::istream& s)
	{
		int size = readFrom<int>(s);
		tag.tag.clear();
		s.read(tag.tag.data(), size);
	}

	void _Serializer<Model>::Serialize(const Model& m, std::ostream& s) const
	{
		Logger::Error("Model Serialization is not implemented yet.");
		/*auto path = ResourceManager::GetModelPath(m);
		writeTo<int>(s, path.size());
		s.write(path.c_str(), path.size());*/
	}
	void _Serializer<Model>::Deserialize(Model& comp, std::istream& s)
	{
		Logger::Error("Model Deserialization is not implemented yet.");
		/*int size = readFrom<int>(s);
		std::string path;
		path.resize(size);
		s.read(path.data(), size);

		comp = ResourceManager::GetModel(path);*/
	}
}
