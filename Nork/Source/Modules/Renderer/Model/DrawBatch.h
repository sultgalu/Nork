#pragma once

#include "../Objects/Shader/Shader.h"
#include "../Objects/VertexArray/VertexArray.h"
#include "../Objects/Buffer/BufferBuilder.h"
#include "../Storage/TypedBuffers.h"
#include "Mesh.h"
#include "Material.h"

namespace Nork::Renderer {
	struct DrawCommandBase
	{
		std::vector<std::shared_ptr<Buffer>> ubos;
		std::shared_ptr<VertexArray> vao;
		
		virtual void Draw(Shader& shader) const = 0;
		void BindUbos() const
		{
			for (auto& ubo : ubos)
			{
				ubo->BindBase();
			}
		}
	};

	struct DrawCommandMultiIndirect: DrawCommandBase
	{
		std::vector<VertexArray::DrawElementsIndirectCommand> indirects;
		void Draw(Shader& shader) const override
		{
			if (indirects.size() > 0)
			{
				BindUbos();
				vao->Bind().MultiDrawInstanced(indirects.data(), indirects.size());
			}
		}
	};

	struct BatchElement
	{
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
		std::shared_ptr<glm::mat4*> modelMatrix;
	};
	struct DrawBatch
	{
		DrawBatch(MatrixUBO& modelUBO, MaterialUBO& materialUBO, DefaultVAO& vao);
		void Clear() { elements.clear(); drawCommand.indirects.clear(); }
		void AddElement(const BatchElement& element) { elements.push_back(element); }
		std::vector<BatchElement>& GetElements() { return elements; }

		const DrawCommandMultiIndirect& GetDrawCommand() { return drawCommand; }
		DrawBatch& GenerateDrawCommand()
		{
			GenerateIndirectCommands();
			return *this;
		}
	private:
		void GenerateIndirectCommands();
	private:
		DrawCommandMultiIndirect drawCommand;
		std::vector<BatchElement> elements;
		std::shared_ptr<Buffer> modelMatIdxUBO;

		MatrixUBO& modelUBO;
		MaterialUBO& materialUBO;
		DefaultVAO& vao;
	};
}