#pragma once

#include "Mesh.h"

namespace Nork::Renderer {
	class Model
	{
	public:
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
	private:
		std::vector<Mesh> meshes;
		glm::mat4 modelMatrix;
	};
}