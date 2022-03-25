#pragma once

#include "Material.h"
#include "../Objects/Shader/Shader.h"
#include "../Storage/MeshStorage.h"

namespace Nork::Renderer {
	/*struct IDrawable
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

	using DrawableIterator = std::function<void(std::function<void(const IDrawable&)>)>;*/

	struct IDrawCommand
	{
		virtual void Draw(Shader&) const = 0;
	};

	struct MultiDrawCommand : public IDrawCommand
	{
		MultiDrawCommand(MeshStorage& meshStorage)
			: meshStorage(meshStorage)
		{}

		struct InstancedDraw
		{// all meshes will be drawn modelMatrices.size() times
			std::vector<std::pair<std::shared_ptr<Mesh>, std::shared_ptr<Material>>> meshes;
			std::vector<glm::mat4> modelMatrices;
		};

		void Draw(Shader&) const override;

		std::vector<InstancedDraw> elements;
		MeshStorage& meshStorage;
	};
}