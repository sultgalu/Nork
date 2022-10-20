export module Nork.Renderer:DrawBatch;

export import :Mesh;
export import :Material;
export import :TypedBuffers;
export import :DrawCommand;

export namespace Nork::Renderer {

	struct BatchElement
	{
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
		std::shared_ptr<glm::mat4*> modelMatrix;
	};
	struct DrawBatch
	{
		DrawBatch(MatrixUBO& modelUBO, MaterialUBO& materialUBO, VAO& vao);
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
		VAO& vao;
	};
}