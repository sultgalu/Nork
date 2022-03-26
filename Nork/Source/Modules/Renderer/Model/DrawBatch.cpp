#include "DrawBatch.h"

namespace Nork::Renderer {
	DrawBatch::DrawBatch(std::shared_ptr<VertexArray> vao)
	{
		modelUbo = BufferBuilder().Usage(BufferUsage::StreamDraw).Target(BufferTarget::UBO).Data(nullptr, 0).Create();
		materialUbo = BufferBuilder().Usage(BufferUsage::StreamDraw).Target(BufferTarget::UBO).Data(nullptr, 0).Create();
		modelUbo->BindBase(5);
		materialUbo->BindBase(7);

		drawCommand.vao = vao;
		drawCommand.ubos.push_back(modelUbo);
		drawCommand.ubos.push_back(materialUbo);
	}

	void DrawBatch::GenerateIndirectCommands()
	{
		drawCommand.indirects.clear();
		std::sort(elements.begin(), elements.end(), [](const BatchElement& left, const BatchElement& right)
			{
				return left.mesh->GetVertexOffset() < right.mesh->GetVertexOffset();
			});

		std::vector<uint32_t> materialIdxs;
		std::vector<glm::mat4> models;
		materialIdxs.reserve(elements.size());
		models.reserve(elements.size());

		uint32_t baseInstance = 0;
		for (auto& element : elements)
		{
			materialIdxs.push_back(element.material->GetBufferIndex());
			models.push_back(element.model);
			auto mesh = element.mesh;

			if (!drawCommand.indirects.empty() && drawCommand.indirects.back().baseVertex == mesh->GetVertexOffset()
				&& drawCommand.indirects.back().firstIndex == mesh->GetIndexOffset()) // test if it is same mesh reference (could expand it to vertex/index counts)
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
					mesh->GetIndexOffset(), mesh->GetIndexCount(), 1, mesh->GetVertexOffset(), baseInstance));
			}
		}
		while (materialIdxs.size() % 4 != 0)
		{ // padding up to a uvec4
			materialIdxs.push_back(0);
		}
		materialUbo->Bind().Clear().Append(materialIdxs);
		modelUbo->Bind().Clear().Append(models);
	}
}