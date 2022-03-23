#pragma once

#include "Mesh.h"
#include "Material.h"
#include "../Objects/Shader/Shader.h"

namespace Nork::Renderer {
	struct IDrawable
	{
		virtual void Draw(Shader&) const = 0;
		virtual void DrawTextureless(Shader&) const = 0;
	};

	struct SingleDrawable : IDrawable
	{
		SingleDrawable(const std::vector<std::pair<Mesh, int>>& meshes, const glm::mat4& modelMatrix)
			: meshes(meshes), modelMatrix(modelMatrix), shader(shader)
		{}
		std::vector<std::pair<Mesh, int>> meshes;
		glm::mat4 modelMatrix;
		std::shared_ptr<Shader> shader;

		void Draw(Shader& shader) const  override;
		void DrawTextureless(Shader& shader) const override;
	};

	struct InstancedDrawable : IDrawable
	{
		InstancedDrawable(const std::vector<std::pair<Mesh, int>>& meshes, const std::vector<glm::mat4>& modelMatrices)
			: meshes(meshes), modelMatrices(modelMatrices), shader(shader)
		{}
		std::vector<std::pair<Mesh, int>> meshes;
		std::vector<glm::mat4> modelMatrices;
		std::shared_ptr<Shader> shader;

		void Draw(Shader& shader) const override;
		void DrawTextureless(Shader& shader) const override;
	};

	using DrawableIterator = std::function<void(std::function<void(const IDrawable&)>)>;
}