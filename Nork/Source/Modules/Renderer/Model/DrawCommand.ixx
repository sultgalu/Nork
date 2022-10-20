export module Nork.Renderer:DrawCommand;

import :VertexArray;
import :Shader;

export namespace Nork::Renderer {
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

	struct DrawCommandMultiIndirect : DrawCommandBase
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
}