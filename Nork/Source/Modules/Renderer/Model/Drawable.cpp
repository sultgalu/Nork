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
		static auto ubo = CreateModelUbo(1001);
		return ubo;
	}

	static std::shared_ptr<Buffer> GetMaterialLayoutUbo()
	{
		static auto ubo = CreateMaterialLayoutUbo(1000);
		return ubo;
	}

	void InstancedDrawable::Draw(Shader& shader) const
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
	}
}