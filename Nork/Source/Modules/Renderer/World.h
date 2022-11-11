#pragma once

#include "Storage/SmartMappedBuffer.h"
#include "Objects/VertexArray/VertexArray.h"
#include "Data/Lights.h"
#include "Model/Object.h"

namespace Nork::Renderer {
	struct LightShadowIndices
	{
		std::vector<std::array<uint32_t, 2>> lightAndShadows;
		std::vector<uint32_t> lights;
	};

	using DirLight = UBO<Data::DirLight>::Element;
	using PointLight = UBO<Data::PointLight>::Element;
	using DirShadow = UBO<Data::DirShadow>::Element;
	using PointShadow = UBO<Data::PointShadow>::Element;
	using ModelMatrix = UBO<glm::mat4>::Element;

	class World
	{
	public:
		World();

		Mesh AddMesh(uint32_t verts, uint32_t indices);
		Mesh AddMesh(const std::vector<Data::Vertex>&, const std::vector<uint32_t>&);
		Material AddMaterial();
		ModelMatrix AddModel();
		// Adds a Mesh, Material, and ModelMatrix, returns them in an Object structure
		Object CreateObject(uint32_t vertCount, uint32_t indexCount);

		DirLight AddDirLight();
		PointLight AddPointLight();
		DirShadow AddDirShadow();
		PointShadow AddPointShadow();

		void DirLightIndices(const LightShadowIndices&);
		void PointLightIndices(const LightShadowIndices&);
	public:
		UBO<glm::mat4> modelMatrices;
		UBO<Data::Material> materials;

		UBO<Data::DirLight> dirLights;
		UBO<Data::DirShadow> dirShadows;
		UBO<Data::PointLight> pointLights;
		UBO<Data::PointShadow> pointShadows;
		
		std::shared_ptr<Buffer> dirIndices;
		std::shared_ptr<Buffer> pointIndices;
		
		VBO<Data::Vertex> vertices;
		IBO indices;

		std::shared_ptr<VertexArray> vao;
	};
}