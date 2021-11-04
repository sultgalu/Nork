#include "World.h"

namespace Nork::Physics
{
	Shape& World::AddShape(std::vector<glm::vec3>& verts, std::vector<Edge>& edges, std::vector<Face>& faces, std::vector<glm::vec3>& fNorm, glm::vec3& center)
	{
		bool expand = this->verts.size() + verts.size() > this->verts.capacity() ||
			this->faces.size() + faces.size() > this->faces.capacity() ||
			this->edges.size() + edges.size() > this->edges.capacity() ||
			this->fNorm.size() + verts.size() > this->fNorm.capacity();
		if (expand)
		{
			//Logger::Warning("No space for more Shapes, expanding");

			std::vector<std::pair<uint32_t, uint32_t>> sverts;
			std::vector<std::pair<uint32_t, uint32_t>> sedges;
			std::vector<std::pair<uint32_t, uint32_t>> sfaces;
			std::vector<std::pair<uint32_t, uint32_t>> sfNorm;

			for (size_t i = 0; i < shapes.size(); i++)
			{
				sverts.push_back(std::pair(shapes[i].verts.data() - this->verts.data(), shapes[i].verts.size()));
				sedges.push_back(std::pair(shapes[i].edges.data() - this->edges.data(), shapes[i].edges.size()));
				sfaces.push_back(std::pair(shapes[i].faces.data() - this->faces.data(), shapes[i].faces.size()));
				sfNorm.push_back(std::pair(shapes[i].fNorm.data() - this->fNorm.data(), shapes[i].fNorm.size()));
			}

			this->verts.insert(this->verts.end(), verts.begin(), verts.end());
			this->faces.insert(this->faces.end(), faces.begin(), faces.end());
			this->edges.insert(this->edges.end(), edges.begin(), edges.end());
			this->fNorm.insert(this->fNorm.end(), fNorm.begin(), fNorm.end());

			for (size_t i = 0; i < shapes.size(); i++)
			{
				shapes[i].verts = std::span(this->verts.data() + sverts[i].first, sverts[i].second);
				shapes[i].edges = std::span(this->edges.data() + sedges[i].first, sedges[i].second);
				shapes[i].faces = std::span(this->faces.data() + sfaces[i].first, sfaces[i].second);
				shapes[i].fNorm = std::span(this->fNorm.data() + sfNorm[i].first, sfNorm[i].second);
			}
		}
		else
		{
			this->verts.insert(this->verts.end(), verts.begin(), verts.end());
			this->faces.insert(this->faces.end(), faces.begin(), faces.end());
			this->edges.insert(this->edges.end(), edges.begin(), edges.end());
			this->fNorm.insert(this->fNorm.end(), fNorm.begin(), fNorm.end());
		}
		
		Shape newShape = Shape{
			.verts = std::span(this->verts.end() - verts.size(), verts.size()),
			.edges = std::span(this->edges.end() - edges.size(), edges.size()),
			.faces = std::span(this->faces.end() - faces.size(), faces.size()),
			.fNorm = std::span(this->fNorm.end() - fNorm.size(), fNorm.size()),
			.center = center,
		};

		return shapes.emplace_back(newShape);
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