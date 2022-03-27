#include "DrawBatch.h"

namespace Nork::Renderer {
	DrawBatch::DrawBatch(MatrixUBO& modelUBO, MaterialUBO& materialUBO, VAO& vao)
		: modelUBO(modelUBO), materialUBO(materialUBO), vao(vao)
	{
		using enum BufferStorageFlags;
		auto flags = WriteAccess | Persistent | Coherent;
		modelUbo = BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, std::pow(15, 3) * 2 * sizeof(uint32_t)).Create();
		materialUbo = BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, std::pow(15, 3) * 2 * sizeof(uint32_t)).Create();
		modelUbo->BindBase(8).Map(BufferAccess::Write);
		materialUbo->BindBase(7).Map(BufferAccess::Write);

		drawCommand.vao = vao.GetVertexArray();
		drawCommand.ubos.push_back(modelUbo);
		drawCommand.ubos.push_back(materialUbo);
	}

	void DrawBatch::GenerateIndirectCommands()
	{
		if (elements.size() == 0)
			return;
		drawCommand.indirects.clear();
		std::sort(elements.begin(), elements.end(), [](const BatchElement& left, const BatchElement& right)
			{
				return *left.mesh->GetVertexPtr() < *right.mesh->GetVertexPtr();
			});

		auto materialIdxs = (uint32_t*)materialUbo->GetPersistentPtr();
		auto models = (uint32_t*)modelUbo->GetPersistentPtr();
		size_t count = 0;

		uint32_t baseInstance = 0;
		for (auto& element : elements)
		{
			materialIdxs[count] = materialUBO.GetIdxFor(element.material->GetPtr());
			models[count++] = modelUBO.GetIdxFor(element.modelMatrix);

			auto mesh = element.mesh;

			if (!drawCommand.indirects.empty() && drawCommand.indirects.back().baseVertex == vao.GetVertexWrapper().GetIdxFor(mesh->GetVertexPtr())
				&& drawCommand.indirects.back().firstIndex == vao.GetIndexWrapper().GetIdxFor(mesh->GetIndexPtr())) // test if it is same mesh reference (could expand it to vertex/index counts)
			{
				drawCommand.indirects.back().instanceCount++;
			}
			else
			{
				if (!drawCommand.indirects.empty())
				{
					baseInstance += drawCommand.indirects.back().instanceCount;
				}
				drawCommand.indirects.push_back(VertexArray::DrawElementsIndirectCommand(
					vao.GetIndexWrapper().GetIdxFor(mesh->GetIndexPtr()), mesh->GetIndexCount(), 1, vao.GetVertexWrapper().GetIdxFor(mesh->GetVertexPtr()), baseInstance));
			}
		}
		while (count % 4 != 0)
		{ // padding up to a uvec4
			materialIdxs[count] = 0;
			models[count++] = 0;
		}
	}
}