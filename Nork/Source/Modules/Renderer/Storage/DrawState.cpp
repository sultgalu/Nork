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
	std::shared_ptr<DirShadow> DrawState::AddDirShadow(std::shared_ptr<DirLight> light, std::shared_ptr<Shader> shader, glm::uvec2 res, TextureFormat format)
	{
		auto ptr = dirShadowUBO.Add(Data::DirShadow());

		if (dirShadowUBO.GetIdxFor(ptr) < dirLightUBO.GetIdxFor(light->ptrRef))
		{
			dirLightUBO.Swap(dirShadowUBO.GetIdxFor(ptr), dirLightUBO.GetIdxFor(light->ptrRef));
		}

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
	std::shared_ptr<PointShadow> DrawState::AddPointShadow(std::shared_ptr<PointLight> light, std::shared_ptr<Shader> shader, uint32_t res, TextureFormat format)
	{
		auto ptr = pointShadowUBO.Add(Data::PointShadow());

		if (pointShadowUBO.GetIdxFor(ptr) < pointLightUBO.GetIdxFor(light->ptrRef))
		{
			pointLightUBO.Swap(pointShadowUBO.GetIdxFor(ptr), pointLightUBO.GetIdxFor(light->ptrRef));
		}

		lightCount.pointShadow++;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
		return std::make_shared<PointShadow>(ptr, PointShadowMap(shader, res, format));
	}
	void DrawState::RemoveDirShadow(std::shared_ptr<DirShadow> shad)
	{
		auto idxBefore = dirShadowUBO.GetIdxFor(shad->ptrRef);
		dirShadowUBO.Swap(shad->ptrRef, dirShadowUBO.Back());
		auto idxAfter = dirShadowUBO.GetIdxFor(shad->ptrRef);
		dirLightUBO.Swap(idxBefore, idxAfter);

		dirShadowUBO.Erase(shad->ptrRef);
		lightCount.dirShadow--;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
	}
	void DrawState::RemoveDirLight(std::shared_ptr<DirLight> light)
	{
		auto idxBefore = dirLightUBO.GetIdxFor(light->ptrRef);
		dirLightUBO.Swap(light->ptrRef, dirLightUBO.Back());
		auto idxAfter = dirLightUBO.GetIdxFor(light->ptrRef);
		if (idxBefore < lightCount.dirShadow)
		{ // had shadow
			dirShadowUBO.Swap(idxBefore, lightCount.dirShadow - 1);
			dirShadowUBO.Erase(lightCount.dirShadow - 1);
			lightCount.dirShadow--;
		}
		dirLightUBO.Erase(light->ptrRef);
		lightCount.dirLight--;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
	}
	void DrawState::RemovePointShadow(std::shared_ptr<PointShadow> shad)
	{
		auto idxBefore = pointShadowUBO.GetIdxFor(shad->ptrRef);
		pointShadowUBO.Swap(shad->ptrRef, pointShadowUBO.Back());
		auto idxAfter = pointShadowUBO.GetIdxFor(shad->ptrRef);
		pointLightUBO.Swap(idxBefore, idxAfter);

		pointShadowUBO.Erase(shad->ptrRef);
		lightCount.pointShadow--;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
	}
	void DrawState::RemovePointLight(std::shared_ptr<PointLight> light)
	{
		auto idxBefore = pointLightUBO.GetIdxFor(light->ptrRef);
		pointLightUBO.Swap(light->ptrRef, pointLightUBO.Back());
		auto idxAfter = pointLightUBO.GetIdxFor(light->ptrRef);
		if (idxBefore < lightCount.dirShadow)
		{ // had shadow
			pointShadowUBO.Swap(idxBefore, lightCount.pointShadow - 1);
			pointShadowUBO.Erase(lightCount.pointShadow - 1);
			lightCount.pointShadow--;
		}
		pointLightUBO.Erase(light->ptrRef);
		lightCount.pointLight--;
		lightCountUBO->Bind().SubData(&lightCount, sizeof(lightCount));
	}
}


