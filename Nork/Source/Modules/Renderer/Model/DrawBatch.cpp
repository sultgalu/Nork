#include "DrawBatch.h"

namespace Nork::Renderer {
	DrawBatch::DrawBatch(MatrixUBO& modelUBO, MaterialUBO& materialUBO, DefaultVAO& vao)
		: modelUBO(modelUBO), materialUBO(materialUBO), vao(vao)
	{
		using enum BufferStorageFlags;
		auto flags = WriteAccess | Persistent | Coherent;
		modelMatIdxUBO = BufferBuilder().Flags(flags).Target(BufferTarget::UBO).Data(nullptr, std::pow(15, 3) * 4 * sizeof(uint32_t)).Create();
		modelMatIdxUBO->BindBase(7).Map(BufferAccess::Write);

		drawCommand.vao = vao.GetVertexArray();
		drawCommand.ubos.push_back(modelMatIdxUBO);
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

		auto modelMatIdxs = (uint32_t*)modelMatIdxUBO->GetPersistentPtr();
		size_t count = 0;

		uint32_t baseInstance = 0;
		for (auto& element : elements)
		{
			modelMatIdxs[count++] = modelUBO.GetIdxFor(element.modelMatrix);
			modelMatIdxs[count++] = materialUBO.GetIdxFor(element.material->GetPtr());

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
		if (count % 4 != 0)
		{ // padding up to a uvec4
			modelMatIdxs[count++] = 0;
			modelMatIdxs[count++] = 0;
		}
	}
}