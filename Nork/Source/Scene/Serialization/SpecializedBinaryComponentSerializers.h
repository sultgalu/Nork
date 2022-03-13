#pragma once

#include "BinaryComponentSerializer.h"
#include "Components/All.h"

namespace Nork
{
	using namespace Components;

	template<>
	inline void BinaryComponentSerializer<Tag>::operator<<(Tag& tag)
	{
		size_t size = tag.tag.size();
		writer << tag.tag.size();
		writer.Write(tag.tag.data(), size);
	}
	template<>
	inline void BinaryComponentDeserializer<Tag>::operator>>(Tag& tag)
	{
		size_t size;
		reader >> size;
		tag.tag = reader.ReadStr(size);
	}

	template<>
	inline void BinaryComponentSerializer<Model>::operator<<(Model& model)
	{
		std::string src = "";
		Logger::Info("Serializing model with source path: ", src);

		writer << src.size();
		writer << src;
	}
	template<>
	inline void BinaryComponentDeserializer<Model>::operator>>(Model& model)
	{
		std::abort();
		size_t size;
		reader >> size;

		std::string src = std::string(reader.ReadStr(size));
		//if (src._Equal(""))
		//	scene.AddModel(id);
	}
}