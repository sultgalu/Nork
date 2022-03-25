#pragma once

#include "../Model/Material.h"
#include "../Objects/Buffer/Buffer.h"

namespace Nork::Renderer {
	class MaterialStorage
	{
	public:
		MaterialStorage(size_t initialCount = 0);
		std::shared_ptr<Material> Add();
		std::vector<std::shared_ptr<Material>> Add(size_t count);
		void FreeObsoleteData(bool shrinkToFit = false);
		void Update(std::shared_ptr<Material> material);
		std::vector<std::shared_ptr<Material>> materials;
		std::shared_ptr<Buffer> ubo;
	};
}