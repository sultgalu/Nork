export module Nork.Renderer.Data:Vertex;

export namespace Nork::Renderer::Data {
	struct Vertex
	{
		glm::vec3 position, normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
	};
}