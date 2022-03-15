#pragma once

#include "Mesh.h"
#include "../Objects/Shader/Shader.h"

namespace Nork::Renderer {
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
}