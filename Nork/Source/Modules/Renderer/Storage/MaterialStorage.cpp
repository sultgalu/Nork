#include "pch.h"
#include "MaterialStorage.h"
#include "../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	MaterialStorage::MaterialStorage(size_t initialCount)
	{
		ubo = BufferBuilder()
			.Target(BufferTarget::UBO)
			.Usage(BufferUsage::StaticDraw)
			.Data(nullptr, initialCount * sizeof(Model::Material))
			.Create();
		ubo->BindBase(6);
	}
	void MaterialStorage::Update(std::shared_ptr<Material> material)
	{
		auto model = material->ToModel();
		ubo->Bind().SetData(&model, sizeof(Model::Material), material->storageIdx * sizeof(Model::Material));
	}
	std::shared_ptr<Material> MaterialStorage::Add()
	{
		auto material = std::make_shared<Material>(ubo->GetSize() / sizeof(Model::Material));
		materials.push_back(material);

		auto model = material->ToModel();
		ubo->Bind().Append(&model, sizeof(Model::Material));

		return material;
	}
	std::vector<std::shared_ptr<Material>> MaterialStorage::Add(size_t count)
	{
		std::vector<std::shared_ptr<Material>> materials;
		ubo->Bind().Reserve(ubo->GetSize() + count * sizeof(Model::Material));
		for (size_t i = 0; i < count; i++)
		{
			materials.push_back(Add());
		}
		return materials;
	}
	void MaterialStorage::FreeObsoleteData(bool shrinkToFit)
	{
		std::vector<std::pair<size_t, size_t>> eraseRanges;
		for (auto& material : materials)
		{
			if (material.use_count() <= 1)
			{
				eraseRanges.push_back({ material->storageIdx * sizeof(Model::Material), sizeof(Model::Material) });
			}
		}

		ubo->Bind().Erase(eraseRanges);

		if (shrinkToFit)
		{
			ubo->ShrinkToFit();
		}

		for (int i = (int)materials.size() - 1; i >= 0; i--)
		{
			if (materials[i].use_count() <= 1)
			{
				materials.erase(materials.begin() + i);
			}
		}
	}
}