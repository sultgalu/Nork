#include "World.h"

namespace Nork::Physics
{
	Shape& World::AddShape(std::vector<glm::vec3>& verts, std::vector<Edge>& edges, std::vector<Face>& faces, std::vector<glm::vec3>& fNorm)
	{
		this->verts.insert(this->verts.end(), verts.begin(), verts.end());
		this->faces.insert(this->faces.end(), faces.begin(), faces.end());
		this->edges.insert(this->edges.end(), edges.begin(), edges.end());
		this->fNorm.insert(this->fNorm.end(), fNorm.begin(), fNorm.end());

		return shapes.emplace_back(Shape{
			.verts = std::span(this->verts.begin(), verts.size()),
			.edges = std::span(this->edges.begin(), edges.size()),
			.faces = std::span(this->faces.begin(), faces.size()),
			.fNorm = std::span(this->fNorm.begin(), fNorm.size()),
			.center = Center(std::span(this->verts.begin(), verts.size())),
			});
	}

	void World::Remove(Shape& shape)
	{
		std::memmove(shape.verts.data(), shape.verts.data() + shape.verts.size(), shape.verts.size_bytes());
		std::memmove(shape.faces.data(), shape.faces.data() + shape.faces.size(), shape.faces.size_bytes());
		std::memmove(shape.edges.data(), shape.edges.data() + shape.edges.size(), shape.edges.size_bytes());
		std::memmove(shape.fNorm.data(), shape.fNorm.data() + shape.fNorm.size(), shape.fNorm.size_bytes());

		verts.resize(verts.size() - shape.verts.size());
		faces.resize(faces.size() - shape.faces.size());
		edges.resize(edges.size() - shape.edges.size());
		fNorm.resize(fNorm.size() - shape.fNorm.size());
	}
}