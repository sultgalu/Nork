#include "World.h"
#include "Objects/Buffer/BufferBuilder.h"
#include "Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork::Renderer {
	World::World()
		: dirLights(5), dirShadows(5), pointLights(5), pointShadows(5), 
		modelMatrices(10 * 1000), materials(100), vertices(100'1000), indices(100'1000)
	{
		vao = VertexArrayBuilder()
			.VBO(vertices.GetBuffer())
			.IBO(indices.GetBuffer())
			.Attributes({ 3, 3, 2, 3 })
			.Create();

		dirLights.GetBuffer()->BindBase(1);
		dirShadows.GetBuffer()->BindBase(2);
		pointLights.GetBuffer()->BindBase(3);
		pointShadows.GetBuffer()->BindBase(4);
		modelMatrices.GetBuffer()->BindBase(5);
		materials.GetBuffer()->BindBase(6);
		DirLightIndices(LightShadowIndices()); // add empty indexes
		PointLightIndices(LightShadowIndices());
	}
	Mesh World::AddMesh(const std::vector<Data::Vertex>& verts, const std::vector<uint32_t>& inds)
	{
		auto mesh = AddMesh(verts.size(), inds.size());
		mesh.Vertices().CopyFrom(verts.data(), verts.size());
		mesh.Indices().CopyFrom(inds.data(), inds.size());
		return mesh;
	}
	Mesh World::AddMesh(uint32_t vertCount, uint32_t idxCount)
	{
		auto mesh = Mesh(vertices.Allocate(vertCount), indices.Allocate(idxCount));
		if (vertices.DidCreateNewBuffer() || indices.DidCreateNewBuffer())
		{
			vao->ChangeBuffers(vertices.GetBuffer(), indices.GetBuffer());
		}
		return mesh;
	}
	Material World::AddMaterial()
	{
		return Material(materials.Allocate());
	}
	UBO<glm::mat4>::Element World::AddTransform()
	{
		return modelMatrices.Allocate();
	}
	UBO<Data::DirLight>::Element World::AddDirLight()
	{
		return dirLights.Allocate();
	}
	UBO<Data::PointLight>::Element World::AddPointLight()
	{
		return pointLights.Allocate();
	}
	UBO<Data::DirShadow>::Element World::AddDirShadow()
	{
		return dirShadows.Allocate();
	}
	UBO<Data::PointShadow>::Element World::AddPointShadow()
	{
		return pointShadows.Allocate();
	}
	Object World::CreateObject(uint32_t vertCount, uint32_t indexCount)
	{
		return Object{
			.mesh = AddMesh(vertCount, indexCount),
			.material = AddMaterial(),
			.modelMatrix = AddTransform()
		};
	}
	static std::vector<uint32_t> ToData(const LightShadowIndices& idxs)
	{
		std::vector<uint32_t> data;
		data.reserve(2 + idxs.lightAndShadows.size() * 2 + idxs.lights.size());
		data.push_back(idxs.lights.size() + idxs.lightAndShadows.size());
		data.push_back(idxs.lightAndShadows.size());
		data.push_back(0);
		data.push_back(0);
		for (auto& idx : idxs.lightAndShadows)
		{ // first add lights with shadows
			data.push_back(idx[0]);
			data.push_back(idx[1]);
		}
		data.insert(data.end(), idxs.lights.begin(), idxs.lights.end());
		return data;
	}
	void World::DirLightIndices(const LightShadowIndices& idxs)
	{
		auto data = ToData(idxs);
		dirIndices = BufferBuilder()
			.Target(BufferTarget::UBO)
			.Data(data.data(), data.size() * sizeof(uint32_t))
			.Create();
		dirIndices->BindBase(8);
	}
	void World::PointLightIndices(const LightShadowIndices& idxs)
	{
		auto data = ToData(idxs);
		pointIndices = BufferBuilder()
			.Target(BufferTarget::UBO)
			.Data(data.data(), data.size() * sizeof(uint32_t))
			.Create();
		pointIndices->BindBase(9);
	}
}


