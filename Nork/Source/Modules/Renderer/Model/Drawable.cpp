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

	static std::shared_ptr<Buffer> GetModelUbo()
	{
		static auto ubo = CreateModelUbo(2000);
		return ubo;
	}

	void InstancedDrawable::Draw(Shader& shader) const
	{
		GetModelUbo()->Bind().SetData(modelMatrices.data(), modelMatrices.size() * sizeof(glm::mat4));
		shader.Use().SetInt("instanced", 1);
		for (size_t i = 0; i < meshes.size(); i++)
		{
			// shader.SetBindlessHandle("diffuse", meshes[i].DiffuseMap()->GetBindlessHandle());
			// shader.SetBindlessHandle("normals", meshes[i].NormalMap()->GetBindlessHandle());
			// shader.SetBindlessHandle("roughness", meshes[i].RoughnessMap()->GetBindlessHandle());
			// shader.SetBindlessHandle("reflect", meshes[i].ReflectionMap()->GetBindlessHandle());

			shader.SetInt("materialIdx", meshes[i].second);
			meshes[i].first.DrawInstanced(modelMatrices.size());
			//meshes[i].BindTextures().DrawInstanced(modelMatrices.size());
		}
	}
	void InstancedDrawable::DrawTextureless(Shader& shader) const
	{
		GetModelUbo()->Bind().SetData(modelMatrices.data(), modelMatrices.size() * sizeof(glm::mat4));
		shader.Use().SetInt("instanced", 1);
		for (size_t i = 0; i < meshes.size(); i++)
		{
			shader.SetInt("materialIdx", meshes[i].second);
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
			// shader.SetBindlessHandle("diffuse", meshes[i].DiffuseMap()->GetBindlessHandle());
			// shader.SetBindlessHandle("normals", meshes[i].NormalMap()->GetBindlessHandle());
			// shader.SetBindlessHandle("roughness", meshes[i].RoughnessMap()->GetBindlessHandle());
			// shader.SetBindlessHandle("reflect", meshes[i].ReflectionMap()->GetBindlessHandle());
			meshes[i].first.Draw();
			// meshes[i].BindTextures().Draw();
		}
	}
	void SingleDrawable::DrawTextureless(Shader& shader) const
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
}