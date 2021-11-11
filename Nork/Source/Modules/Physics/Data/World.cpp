#include "World.h"

namespace Nork::Physics
{
	void World::ClearColliderData()
	{
		verts.clear();
		colliderVerts.clear();
		faces.clear();
		edges.clear();
		shapes.clear();
	}
	void World::AddCollider(const Collider& collider)
	{
		bool expand = this->verts.size() + verts.size() > this->verts.capacity() ||
			this->colliderVerts.size() + collider.verts.size() > this->verts.capacity() ||
			this->colliderFaces.size() + collider.faces.size() > this->faces.capacity() ||
			this->faces.size() + collider.faces.size() > this->faces.capacity() ||
			this->edges.size() + collider.edges.size() > this->edges.capacity();

		if (expand)
		{
			//Logger::Warning("No space for more Shapes, expanding");

			std::vector<std::pair<uint32_t, uint32_t>> sverts;
			std::vector<std::pair<uint32_t, uint32_t>> sedges;
			std::vector<std::pair<uint32_t, uint32_t>> sfaces;

			for (size_t i = 0; i < shapes.size(); i++)
			{
				sverts.push_back(std::pair(shapes[i].verts.data() - this->verts.data(), shapes[i].verts.size()));
				sedges.push_back(std::pair(shapes[i].edges.data() - this->edges.data(), shapes[i].edges.size()));
				sfaces.push_back(std::pair(shapes[i].faces.data() - this->faces.data(), shapes[i].faces.size()));
			}

			this->verts.insert(this->verts.end(), collider.verts.begin(), collider.verts.end());
			this->colliderVerts.insert(this->colliderVerts.end(), collider.verts.begin(), collider.verts.end());
			this->faces.insert(this->faces.end(), collider.faces.begin(), collider.faces.end());
			this->colliderFaces.insert(this->colliderFaces.end(), collider.faces.begin(), collider.faces.end());
			this->edges.insert(this->edges.end(), collider.edges.begin(), collider.edges.end());

			for (size_t i = 0; i < shapes.size(); i++)
			{
				shapes[i].verts = std::span(this->verts.data() + sverts[i].first, sverts[i].second);
				shapes[i].colliderVerts = std::span(this->colliderVerts.data() + sverts[i].first, sverts[i].second);
				shapes[i].colliderFaces = std::span(this->colliderFaces.data() + sfaces[i].first, sfaces[i].second);
				shapes[i].edges = std::span(this->edges.data() + sedges[i].first, sedges[i].second);
				shapes[i].faces = std::span(this->faces.data() + sfaces[i].first, sfaces[i].second);
			}
		}
		else
		{
			this->colliderVerts.insert(this->colliderVerts.end(), collider.verts.begin(), collider.verts.end());
			this->colliderFaces.insert(this->colliderFaces.end(), collider.faces.begin(), collider.faces.end());
			this->verts.insert(this->verts.end(), collider.verts.begin(), collider.verts.end());
			this->faces.insert(this->faces.end(), collider.faces.begin(), collider.faces.end());
			this->edges.insert(this->edges.end(), collider.edges.begin(), collider.edges.end());
		}

		auto center = Center(collider.verts);

		Shape newShape = Shape{
			.colliderVerts = std::span(this->colliderVerts.end() - collider.verts.size(), collider.verts.size()),
			.colliderFaces = std::span(this->colliderFaces.end() - collider.faces.size(), collider.faces.size()),
			.verts = std::span(this->verts.end() - collider.verts.size(), collider.verts.size()),
			.edges = std::span(this->edges.end() - collider.edges.size(), collider.edges.size()),
			.faces = std::span(this->faces.end() - collider.faces.size(), collider.faces.size()),
			.center = center,
			.colliderCenter = center
		};

		shapes.push_back(newShape);
	}

	void World::UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions)
	{
		for (size_t i = 0; i < shapes.size(); i++)
		{
			glm::mat4 rotation = glm::mat4_cast(quaternions[i]);

			shapes[i].center = rotation * glm::vec4(shapes[i].center, 1);
			shapes[i].center += translate[i];
			for (size_t j = 0; j < shapes[i].verts.size(); j++)
			{
				shapes[i].verts[j] = rotation * glm::vec4(shapes[i].colliderVerts[j], 1);
				shapes[i].verts[j] += translate[i];
			}
			for (size_t j = 0; j < shapes[i].faces.size(); j++)
			{
				shapes[i].faces[j].norm = rotation * glm::vec4(shapes[i].colliderFaces[j].norm, 1);
			}
		}
	}

	void World::Remove(Shape& shape)
	{
		std::memmove(shape.verts.data(), shape.verts.data() + shape.verts.size(), shape.verts.size_bytes());
		std::memmove(shape.faces.data(), shape.faces.data() + shape.faces.size(), shape.faces.size_bytes());
		std::memmove(shape.edges.data(), shape.edges.data() + shape.edges.size(), shape.edges.size_bytes());

		verts.resize(verts.size() - shape.verts.size());
		faces.resize(faces.size() - shape.faces.size());
		edges.resize(edges.size() - shape.edges.size());
	}
}