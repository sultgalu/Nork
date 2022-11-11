#include "DrawObjectsCommand.h"
#include "../../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	void DrawObjectsCommand::operator()() const
	{
		if (indirects.size() > 0)
		{
			materialModelIndices->BindBase(7);
			vao->Bind().MultiDrawInstanced(indirects.data(), indirects.size());
		}
	}
	DrawObjectsCommand::DrawObjectsCommand(std::shared_ptr<VertexArray> vao, std::span<Object> objects)
		: vao(vao)
	{
		if (objects.size() == 0)
			return;

		std::sort(objects.begin(), objects.end(), [](const Object& left, const Object& right)
			{
				return left.mesh.Vertices().Index() < right.mesh.Vertices().Index();
			});

		std::vector<std::array<uint32_t, 2>> modelMatIdxs;
		modelMatIdxs.reserve(objects.size() + 1);

		uint32_t baseInstance = 0;
		for (auto& obj : objects)
		{
			modelMatIdxs.push_back({ obj.modelMatrix.Index(), obj.material.Element().Index() });

			auto& mesh = obj.mesh;

			if (!indirects.empty() && indirects.back().baseVertex == mesh.Vertices().Index()
				&& indirects.back().firstIndex == mesh.Indices().Index()) // test if it is the same mesh reference (could expand it to vertex/index counts)
			{
				indirects.back().instanceCount++;
			}
			else
			{
				if (!indirects.empty())
				{
					baseInstance += indirects.back().instanceCount;
				}
				indirects.push_back(VertexArray::DrawElementsIndirectCommand(
					mesh.Indices().Index(), mesh.Indices().Count(), 1, mesh.Vertices().Index(), baseInstance));
			}
		}
		if (modelMatIdxs.size() % 2 != 0)
		{ // padding up to a uvec4
			modelMatIdxs.push_back({ 0, 0 });
		}

		materialModelIndices = BufferBuilder()
			.Target(BufferTarget::UBO)
			.Data(modelMatIdxs.data(), modelMatIdxs.size() * sizeof(uint32_t) * 2)
			.Create();
	}
}