#include "pch.h"
#include "include/ShaderView.h"

namespace Nork::Editor {
	ShaderView::ShaderView(std::shared_ptr<Renderer::Shader> shader)
		:shader(shader)
	{
		UpdateUniformValues();
	}
	void ShaderView::Content()
	{
		constexpr float floatSpeed = 0.01f;
		for (auto& uniform : uniformsInt)
		{
			if (ImGui::DragInt(uniform.first.c_str(), &uniform.second) && immediateMode)
			{
				shader->Use().SetInt(uniform.first, uniform.second);
			}
		}
		ImGui::Separator();
		for (auto& uniform : uniformsFloat)
		{
			if (ImGui::DragFloat(uniform.first.c_str(), &uniform.second, floatSpeed) && immediateMode)
			{
				shader->Use().SetFloat(uniform.first, uniform.second);
			}
		}
		ImGui::Separator();
		for (auto& uniform : uniformsVec2)
		{
			if (ImGui::DragFloat2(uniform.first.c_str(), &uniform.second.x, floatSpeed) && immediateMode)
			{
				shader->Use().SetVec2(uniform.first, uniform.second);
			}
		}
		ImGui::Separator();
		for (auto& uniform : uniformsVec3)
		{
			if (ImGui::DragFloat3(uniform.first.c_str(), &uniform.second.x, floatSpeed) && immediateMode)
			{
				shader->Use().SetVec3(uniform.first, uniform.second);
			}
		}
		ImGui::Separator();
		for (auto& uniform : uniformsVec4)
		{
			if (ImGui::DragFloat4(uniform.first.c_str(), &uniform.second.x, floatSpeed) && immediateMode)
			{
				shader->Use().SetVec4(uniform.first, uniform.second);
			}
		}
		if (!immediateMode)
		{
			if (ImGui::Button("Flush##Shaders"))
			{
				for (auto& uniform : uniformsInt)
					shader->SetInt(uniform.first, uniform.second);
				for (auto& uniform : uniformsFloat)
					shader->SetFloat(uniform.first, uniform.second);
				for (auto& uniform : uniformsVec2)
					shader->SetVec2(uniform.first, uniform.second);
				for (auto& uniform : uniformsVec3)
					shader->SetVec3(uniform.first, uniform.second);
				for (auto& uniform : uniformsVec4)
					shader->SetVec4(uniform.first, uniform.second);
			}
		}
		if (ImGui::Button("Reload All##Shaders"))
		{
			UpdateUniformValues();
		}
	}
	void ShaderView::UpdateUniformValues()
	{
		uniformsInt.clear();
		uniformsFloat.clear();
		uniformsVec2.clear();
		uniformsVec3.clear();
		uniformsVec4.clear();

		auto uniforms = shader->QueryAllUniformNamesAndTypes();
		for (auto& uniform : uniforms)
		{
			if (uniform.second == typeid(int).hash_code())
			{
				uniformsInt[uniform.first] = shader->GetInt(uniform.first);
			}
			else if (uniform.second == typeid(float).hash_code())
			{
				uniformsFloat[uniform.first] = shader->GetFloat(uniform.first);
			}
			else if (uniform.second == typeid(glm::vec2).hash_code())
			{
				uniformsVec2[uniform.first] = shader->GetVec2(uniform.first);
			}
			else if (uniform.second == typeid(glm::vec3).hash_code())
			{
				uniformsVec3[uniform.first] = shader->GetVec3(uniform.first);
			}
			else if (uniform.second == typeid(glm::vec4).hash_code())
			{
				uniformsVec4[uniform.first] = shader->GetVec4(uniform.first);
			}
		}
	}
}
