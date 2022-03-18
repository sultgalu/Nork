#pragma once

#include "Mesh.h"
#include "../Objects/Shader/Shader.h"

namespace Nork::Renderer {
	struct IDrawable
	{
		virtual void Draw(Shader&) const = 0;
		virtual void DrawTextureless(Shader&) const = 0;
	};

	struct SingleDrawable : IDrawable
	{
		SingleDrawable(const std::vector<Mesh>& meshes, const glm::mat4& modelMatrix)
			: meshes(meshes), modelMatrix(modelMatrix), shader(shader)
		{}
		std::vector<Mesh> meshes;
		glm::mat4 modelMatrix;
		std::shared_ptr<Shader> shader;

		void Draw(Shader& shader) const  override;
		void DrawTextureless(Shader& shader) const override;
	};

	struct InstancedDrawable : IDrawable
	{
		InstancedDrawable(const std::vector<Mesh>& meshes, const std::vector<glm::mat4>& modelMatrices)
			: meshes(meshes), modelMatrices(modelMatrices), shader(shader)
		{}
		std::vector<Mesh> meshes;
		std::vector<glm::mat4> modelMatrices;
		std::shared_ptr<Shader> shader;

		void Draw(Shader& shader) const override;
		void DrawTextureless(Shader& shader) const override;
	};

	class Model
	{
	public:
		Model(const std::vector<Mesh>& meshes, glm::mat4 modelMatrix)
			: meshes(meshes), modelMatrix(modelMatrix)
		{}
		void Draw()
		{
			for (size_t i = 0; i < meshes.size(); i++)
			{
				meshes[i].BindTextures().Draw();
			}
		}
		void DrawTextureless()
		{
			for (size_t i = 0; i < meshes.size(); i++)
			{
				meshes[i].Draw();
			}
		}
		void Draw(Shader& shader)
		{
			shader.SetMat4("model", modelMatrix);
			Draw();
		}
		void DrawTextureless(Shader& shader)
		{
			shader.SetMat4("model", modelMatrix);
			DrawTextureless();
		}
		static Model Cube()
		{
			static Model cube = Model({ Mesh::Cube() }, glm::identity<glm::mat4>());
			return cube;
		}
		void SetModelMatrix(const glm::mat4& m)
		{
			modelMatrix = m;
		}
		std::vector<Mesh>& Meshes()
		{
			return meshes;
		}
	private:
		std::vector<Mesh> meshes;
		glm::mat4 modelMatrix;
	};

	using ModelIterator = std::function<void(std::function<void(Model&)>)>;
	using DrawableIterator = std::function<void(std::function<void(const IDrawable&)>)>;
}