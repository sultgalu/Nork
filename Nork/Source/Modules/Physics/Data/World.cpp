#include "World.h"

namespace Nork::Physics
{
	void World::ClearColliderData()
	{
		verts.clear();
		colliderVerts.clear();
		colliderFaces.clear();
		faces.clear();
		faceVerts.clear();
		edges.clear();
		shapes.clear();
	}
	void World::AddCollider(const Collider& collider)
	{
		bool expand = this->verts.size() + collider.verts.size() > this->verts.capacity() ||
			this->colliderVerts.size() + collider.verts.size() > this->verts.capacity() ||
			this->colliderFaces.size() + collider.faces.size() > this->faces.capacity() ||
			this->faces.size() + collider.faces.size() > this->faces.capacity() ||
			this->faceVerts.size() + collider.faceVerts.size() > this->faceVerts.capacity() ||
			this->edges.size() + collider.edges.size() > this->edges.capacity();

		if (expand)
		{
			//Logger::Warning("No space for more Shapes, expanding");

			std::vector<std::pair<uint32_t, uint32_t>> sverts;
			std::vector<std::pair<uint32_t, uint32_t>> sedges;
			std::vector<std::pair<uint32_t, uint32_t>> sfaces;
			std::vector<std::pair<uint32_t, uint32_t>> sfVerts;

			for (size_t i = 0; i < shapes.size(); i++)
			{
				sverts.push_back(std::pair(shapes[i].verts.data() - this->verts.data(), shapes[i].verts.size()));
				sedges.push_back(std::pair(shapes[i].edges.data() - this->edges.data(), shapes[i].edges.size()));
				sfaces.push_back(std::pair(shapes[i].faces.data() - this->faces.data(), shapes[i].faces.size()));
				sfVerts.push_back(std::pair(shapes[i].faceVerts.data() - this->faceVerts.data(), shapes[i].faceVerts.size()));
			}

			this->verts.insert(this->verts.end(), collider.verts.begin(), collider.verts.end());
			this->colliderVerts.insert(this->colliderVerts.end(), collider.verts.begin(), collider.verts.end());
			this->faces.insert(this->faces.end(), collider.faces.begin(), collider.faces.end());
			this->faceVerts.insert(this->faceVerts.end(), collider.faceVerts.begin(), collider.faceVerts.end());
			this->colliderFaces.insert(this->colliderFaces.end(), collider.faces.begin(), collider.faces.end());
			this->edges.insert(this->edges.end(), collider.edges.begin(), collider.edges.end());

			for (size_t i = 0; i < shapes.size(); i++)
			{
				shapes[i].verts = std::span(this->verts.data() + sverts[i].first, sverts[i].second);
				shapes[i].colliderVerts = std::span(this->colliderVerts.data() + sverts[i].first, sverts[i].second);
				shapes[i].colliderFaces = std::span(this->colliderFaces.data() + sfaces[i].first, sfaces[i].second);
				shapes[i].edges = std::span(this->edges.data() + sedges[i].first, sedges[i].second);
				shapes[i].faces = std::span(this->faces.data() + sfaces[i].first, sfaces[i].second);
				shapes[i].faceVerts = std::span(this->faceVerts.data() + sfVerts[i].first, sfVerts[i].second);
			}
		}
		else
		{
			this->colliderVerts.insert(this->colliderVerts.end(), collider.verts.begin(), collider.verts.end());
			this->colliderFaces.insert(this->colliderFaces.end(), collider.faces.begin(), collider.faces.end());
			this->faceVerts.insert(this->faceVerts.end(), collider.faceVerts.begin(), collider.faceVerts.end());
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
			.faceVerts = std::span(this->faceVerts.end() - collider.faceVerts.size(), collider.faceVerts.size()),
			.center = center,
			.colliderCenter = center
		};

		shapes.push_back(newShape);
	}

	void World::UpdateTransform(Shape& shape, const glm::vec3& translate, const glm::quat& quaternion)
	{
		glm::mat4 rotation = glm::mat4_cast(quaternion);

		// shapes[i].center = rotation * glm::vec4(shapes[i].center, 1);
		// shapes[i].center += translate[i];
		for (size_t j = 0; j < shape.verts.size(); j++)
		{
			shape.verts[j] = rotation * glm::vec4(shape.colliderVerts[j], 1);
			shape.verts[j] += translate;
		}
		for (size_t j = 0; j < shape.faces.size(); j++)
		{
			shape.faces[j].norm = rotation * glm::vec4(shape.colliderFaces[j].norm, 1);
		}

		shape.center = glm::vec3(0);
		for (auto& vert : shape.verts)
		{
			shape.center += vert;
		}
		shape.center /= shape.verts.size();
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