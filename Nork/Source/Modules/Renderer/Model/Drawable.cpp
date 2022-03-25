#include "Model.h"
#include "../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	static std::shared_ptr<Buffer> CreateModelUbo(size_t count)
	{
		auto ubo = BufferBuilder()
			.Target(BufferTarget::UBO)
			.Usage(BufferUsage::DynamicDraw)
			.Data(nullptr, count * sizeof(glm::mat4))
			.Create();
		ubo->BindBase(5);
		return ubo;
	}
	static std::shared_ptr<Buffer> CreateMaterialLayoutUbo(size_t count)
	{
		auto ubo = BufferBuilder()
			.Target(BufferTarget::UBO)
			.Usage(BufferUsage::DynamicDraw)
			.Data(nullptr, count * sizeof(int))
			.Create();
		ubo->BindBase(7);
		return ubo;
	}

	static std::shared_ptr<Buffer> GetModelUbo()
	{
		static auto ubo = CreateModelUbo(0);
		return ubo;
	}

	static std::shared_ptr<Buffer> GetMaterialLayoutUbo()
	{
		static auto ubo = CreateMaterialLayoutUbo(0);
		return ubo;
	}

	/*void InstancedDrawable::Draw(Shader& shader) const
	{
		GetModelUbo()->Bind().SetData(modelMatrices.data(), modelMatrices.size() * sizeof(glm::mat4));
		GetMaterialLayoutUbo()->Bind();
		shader.Use().SetInt("instanced", 1);

		for (size_t i = 0; i < meshes.size(); i++)
		{
			int count = meshes[i].first.vao->GetVBO()->GetSize() / sizeof(Vertex);
			static std::vector<int> materialIndices(1000, -1);
			materialIndices.front() = meshes[i].second;
			GetMaterialLayoutUbo()->SetData(materialIndices.data(), materialIndices.size() * sizeof(int));

			meshes[i].first.DrawInstanced(modelMatrices.size());
		}
	}
	void InstancedDrawable::DrawTextureless(Shader& shader) const
	{
		GetModelUbo()->Bind().SetData(modelMatrices.data(), modelMatrices.size() * sizeof(glm::mat4));
		shader.Use().SetInt("instanced", 1);
		for (size_t i = 0; i < meshes.size(); i++)
		{
			meshes[i].first.DrawInstanced(modelMatrices.size());
		}
	}
	void SingleDrawable::Draw(Shader& shader) const
	{
		shader.Use()
			.SetMat4("model", modelMatrix)
			.SetInt("instanced", 0);
		for (size_t i = 0; i < meshes.size(); i++)
		{
			shader.SetInt("materialIdx", meshes[i].second);
			meshes[i].first.Draw();
		}
	}
	void SingleDrawable::DrawTextureless(Shader& shader) const
	{
		shader.Use()
			.SetMat4("model", modelMatrix)
			.SetInt("instanced", 0);
		for (size_t i = 0; i < meshes.size(); i++)
		{
			meshes[i].first.Draw();
		}
	}*/
	void MultiDrawCommand::Draw(Shader& shader) const
	{
		std::vector<uint32_t> materialIdxs;
		std::vector<glm::mat4> models;
		std::vector<VertexArray::DrawElementsIndirectCommand> indirectCommands;

		uint32_t baseInstance = 0;
		for (auto& element : elements)
		{
			// assume no instancing now
			for (auto& mesh : element.meshes)
			{
				materialIdxs.push_back(mesh.second->storageIdx);
				auto& meshRange = meshStorage.ranges[mesh.first->storageIdx];
				indirectCommands.push_back(VertexArray::DrawElementsIndirectCommand(
					meshRange.indexOffs, meshRange.indexCount, element.modelMatrices.size(), meshRange.vertexOffs, baseInstance));

				// modelMatrices for instances
				models.insert(models.end(), element.modelMatrices.begin(), element.modelMatrices.end());
				baseInstance += element.modelMatrices.size();
			}
		}
		// for (size_t i = 0; i < 120; i++)
		// {
		// 	materialIdxs.push_back(0);
		// }
		GetMaterialLayoutUbo()->Bind().Clear().Append(materialIdxs);
		GetModelUbo()->Bind().Clear().Append(models);

		shader.SetInt("instanced", 0);
		meshStorage.vao->Bind().MultiDrawInstanced(indirectCommands.data(), indirectCommands.size());
	}
}