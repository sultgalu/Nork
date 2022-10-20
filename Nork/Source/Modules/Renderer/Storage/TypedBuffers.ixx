export module Nork.Renderer:TypedBuffers;

export import :TypedBufferWrapper;
export import Nork.Renderer.Data;

export namespace Nork::Renderer {
	using DefaultVBO = VBO<Data::Vertex>;

	using MaterialUBO = UBO<Data::Material>;
	using MatrixUBO = UBO<glm::mat4>;

	using DirLightUBO = UBO<Data::DirLight>;
	using DirShadowUBO = UBO<Data::DirShadow>;
	using PointLightUBO = UBO<Data::PointLight>;
	using PointShadowUBO = UBO<Data::PointShadow>;
}