#include "pch.h"
#include "DrawState.h"
#include "../Objects/Buffer/BufferBuilder.h"

namespace Nork::Renderer {
	DrawState::DrawState()
		: vaoWrapper(1000 * 1000, 1000 * 1000),
		materialBuffer(1), modelMatrixBuffer(1)	
	{
		lightCountUBO = BufferBuilder()
			.Target(BufferTarget::UBO)
			.Flags(BufferStorageFlags::DynamicStorage)
			.Data(nullptr, sizeof(uint32_t) * 4)
			.Create();
		lightCountUBO->BindBase(0)
			.SubData(&lightCount, sizeof(lightCount));
		dirLightUBO.GetBuffer()->BindBase(1);
		dirShadowUBO.GetBuffer()->BindBase(2);
		pointLightUBO.GetBuffer()->BindBase(3);
		pointShadowUBO.GetBuffer()->BindBase(4);
		modelMatrixBuffer.GetBuffer()->BindBase(5);
		materialBuffer.GetBuffer()->BindBase(6);
	}
	std::shared_ptr<Material> DrawState::AddMaterial()
	{
		auto ptr = materialBuffer.Add(Data::Material());
		return std::make_shared<Material>(ptr);
	}
	std::shared_ptr<DirLight> DrawState::AddDirLight()
	{
		auto ptr = dirLightUBO.Add(Data::DirLight());
		lightCount.dirLight++;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
		return std::make_shared<DirLight>(ptr);
	}
	std::shared_ptr<DirShadow> DrawState::AddDirShadow(std::shared_ptr<Shader> shader, glm::uvec2 res, TextureFormat format)
	{
		auto ptr = dirShadowUBO.Add(Data::DirShadow());
		lightCount.dirShadow++;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
		return std::make_shared<DirShadow>(ptr, DirShadowMap(shader, res.x, res.y, format));
	}
	std::shared_ptr<PointLight> DrawState::AddPointLight()
	{
		auto ptr = pointLightUBO.Add(Data::PointLight());
		lightCount.pointLight++;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
		return std::make_shared<PointLight>(ptr);
	}
	std::shared_ptr<PointShadow> DrawState::AddPointShadow(std::shared_ptr<Shader> shader, uint32_t res, TextureFormat format)
	{
		auto ptr = pointShadowUBO.Add(Data::PointShadow());
		lightCount.pointShadow++;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
		return std::make_shared<PointShadow>(ptr, PointShadowMap(shader, res, format));
	}
}


