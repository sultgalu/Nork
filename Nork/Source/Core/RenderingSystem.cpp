#include "RenderingSystem.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Model/Object.h"
#include "ColliderPass.h"
#include "Components/All.h"

namespace Nork {
Renderer::Renderer* Renderer::Renderer::instance = nullptr;
std::shared_ptr<Renderer::Renderer> renderer;
static entt::registry& Registry() {
	return Scene::Current().registry;
}

void RenderingSystem::OnDrawableUpdated(entt::registry& reg, entt::entity id)
{
}
void RenderingSystem::OnDrawableRemoved(entt::registry& reg, entt::entity id)
{
}
void RenderingSystem::OnDrawableAdded(entt::registry& reg, entt::entity id)
{
	auto& dr = reg.get<Components::Drawable>(id);
	auto tr = reg.try_get<Components::Transform>(id);
	dr.sharedTransform = NewModelMatrix();
	if (tr)
		*dr.sharedTransform = tr->RecalcModelMatrix();
	if (dr.GetModel() == nullptr || dr.GetModel()->nodes.empty())
		dr.SetModel(AssetLoader::Instance().LoadModel(AssetLoader::Instance().CubeUri()));
}
void RenderingSystem::UpdateDirLightShadows()
{
	// Renderer::LightShadowIndices idxs;
	// shadowMapProvider.dShadMaps.clear();
	// 
	// auto ls = registry.view<Components::DirShadowMap>();
	// auto l = registry.view<Components::DirLight>(entt::exclude<Components::DirShadowMap>);
	// idxs.lightAndShadows.reserve(ls.size());
	// idxs.lights.reserve(l.size_hint());
	// for (auto [id, shadMap] : ls.each())
	// {
	// 	idxs.lightAndShadows.push_back({ shadMap.map.light.Index(), shadMap.map.shadow.Index() });
	// 	shadowMapProvider.dShadMaps.push_back(shadMap.map);
	// }
	// for (auto [id, light] : l.each())
	// {
	// 	idxs.lights.push_back(light.light.Index());
	// }
	// world.DirLightIndices(idxs);
}
void RenderingSystem::UpdatePointLightShadows()
{
	// Renderer::LightShadowIndices idxs;
	// shadowMapProvider.pShadMaps.clear();
	// 
	// auto ls = registry.view<Components::PointShadowMap>();
	// auto l = registry.view<Components::PointLight>(entt::exclude<Components::PointShadowMap>);
	// idxs.lightAndShadows.reserve(ls.size());
	// idxs.lights.reserve(l.size_hint());
	// for (auto [id, shadMap] : ls.each())
	// {
	// 	idxs.lightAndShadows.push_back({ shadMap.map.light.Index(), shadMap.map.shadow.Index() });
	// 	shadowMapProvider.pShadMaps.push_back(shadMap.map);
	// }
	// for (auto [id, light] : l.each())
	// {
	// 	idxs.lights.push_back(light.light.Index());
	// }
	// world.PointLightIndices(idxs);
}
void RenderingSystem::OnDLightAdded(entt::registry& reg, entt::entity id)
{
	auto& light = reg.get<Components::DirLight>(id);
	light.rendererLight = NewDirLight();
	light.RecalcVP();
	shouldUpdateDirLightAndShadows = true;
}
void RenderingSystem::OnPLightAdded(entt::registry& reg, entt::entity id)
{
	auto& light = reg.get<Components::PointLight>(id);
	auto& tr = reg.get_or_emplace<Components::Transform>(id);
	light.rendererLight = NewPointLight();
	light.SetIntensity(50);
	light.Data()->position = tr.Position();
	shouldUpdatePointLightAndShadows = true;
}
void RenderingSystem::OnDShadAdded(entt::registry& reg, entt::entity id)
{
	auto& light = reg.get<Components::DirLight>(id);
	auto& shadow = reg.get<Components::DirShadowMap>(id);
	shadow.shadowMap = Renderer::Resources::Instance().CreateShadowMap2D(1000, 1000);
	shadow.shadowMap->vp = light->VP;
	shouldUpdateDirLightAndShadows = true;
}
void RenderingSystem::OnPShadAdded(entt::registry& reg, entt::entity id)
{
	auto& light = reg.get<Components::PointLight>(id);
	auto& shadow = reg.get<Components::PointShadowMap>(id);
	shadow.shadowMap = Renderer::Resources::Instance().CreateShadowMapCube(100);
	shadow.shadowMap->position = light->position;
	shouldUpdatePointLightAndShadows = true;
}

void RenderingSystem::OnDShadRemoved(entt::registry& reg, entt::entity id)
{
	shouldUpdateDirLightAndShadows = true;
}
void RenderingSystem::OnPShadRemoved(entt::registry& reg, entt::entity id)
{
	shouldUpdatePointLightAndShadows = true;
}
void RenderingSystem::OnDLightRemoved(entt::registry& reg, entt::entity id)
{
	reg.remove<Components::DirShadowMap>(id);
	shouldUpdateDirLightAndShadows = true;
}
void RenderingSystem::OnPLightRemoved(entt::registry& reg, entt::entity id)
{
	reg.remove<Components::PointShadowMap>(id);
	shouldUpdatePointLightAndShadows = true;
}
struct Client : Renderer::Client
{
	void OnCommands() override
	{
	}
	uint32_t FillDrawBuffers(std::span<Renderer::DrawParams> params,
		std::span<vk::DrawIndexedIndirectCommand> commands) override
	{
		auto group = Registry().group<Components::Drawable>(entt::get<Components::Transform>);
		if (group.size() == 0)
			return 0;
		if (group.size() > params.size())
			std::unreachable();
		std::vector<Renderer::Object> objects;
		objects.reserve(group.size());

		auto add = [&](const Components::Drawable& dr, std::vector<Renderer::Object>& to)
		{
			for (size_t i = 0; i < dr.GetModel()->nodes.size(); i++)
			{
				auto& node = dr.GetModel()->nodes[i];
				for (auto& subMesh : node.mesh->subMeshes) {
					to.push_back(Renderer::Object{ .mesh = subMesh.mesh, .material = subMesh.material, .modelMatrix = dr.transforms[i] });
				}
			}
		};
		for (auto [id, dr, tr] : group.each())
		{
			if (true) // eg.: its material has a deferredShader
			{
				add(dr, objects);
			}
			if (true) // eg.: not opaque
			{
				//add(dr, shadowedObjects);
			}
		}

		std::sort(objects.begin(), objects.end(), [](const Renderer::Object& left, const Renderer::Object& right)
		{
			return left.mesh->vertices->offset < right.mesh->vertices->offset;
		});

		uint32_t commandCount = 0;
		uint32_t instanceCount = 0;
		vk::DrawIndexedIndirectCommand* lastCommand = nullptr;
		for (auto& obj : objects)
		{
			params[instanceCount].mmIdx = obj.modelMatrix->offset;
			params[instanceCount].matIdx = obj.material->deviceData->offset;

			auto& mesh = obj.mesh;

			if (commandCount > 0 && lastCommand->vertexOffset == mesh->vertices->offset
				&& lastCommand->firstIndex == mesh->indices->offset) // test if it is the same mesh reference (could expand it to vertex/index counts)
			{
				lastCommand->instanceCount++;
			}
			else
			{
				commands[commandCount].vertexOffset = mesh->vertices->offset;
				commands[commandCount].firstIndex = mesh->indices->offset;
				commands[commandCount].firstInstance = instanceCount;
				commands[commandCount].indexCount = mesh->indices->count;
				commands[commandCount].instanceCount = 1;
				lastCommand = &commands[commandCount];
				commandCount++;
			}
			instanceCount++;
		}
		// if (instanceCount % 2 != 0)
		// { // padding up to a uvec4
		// 	modelMatIdxs.push_back({ 0, 0 });
		// }
		return commandCount;
	}
	void FillLightBuffers(std::span<uint32_t> dIdxs, std::span<uint32_t> pIdxs,
		uint32_t& dlCount, uint32_t& dsCount, uint32_t& plCount, uint32_t& psCount) override
	{
		dlCount = 0; dsCount = 0; plCount = 0; psCount = 0;

		for (auto [ent, light, shad] : Registry().view<Components::DirLight, Components::DirShadowMap>().each())
		{
			dIdxs[dlCount * 2] = light.rendererLight->deviceData->offset;
			dIdxs[dlCount * 2 + 1] = shad.shadowMap->shadow->deviceData->offset;
			dlCount++;
			dsCount++;
		}
		for (auto [ent, light] : Registry().view<Components::DirLight>(entt::exclude<Components::DirShadowMap>).each())
		{
			dIdxs[dlCount * 2] = light.rendererLight->deviceData->offset;
			dlCount++;
		}
		for (auto [ent, light, shad] : Registry().view<Components::PointLight, Components::PointShadowMap>().each())
		{
			pIdxs[plCount * 2] = light.rendererLight->deviceData->offset;
			pIdxs[plCount * 2 + 1] = shad.shadowMap->shadow->deviceData->offset;
			plCount++;
			psCount++;
		}
		for (auto [ent, light] : Registry().view<Components::PointLight>(entt::exclude<Components::PointShadowMap>).each())
		{
			pIdxs[plCount * 2] = light.rendererLight->deviceData->offset;
			plCount++;
		}
	}
	void ProvideShadowMapsForUpdate(std::vector<std::shared_ptr<Renderer::DirShadowMap>>& dShadMaps,
		std::vector<std::shared_ptr<Renderer::PointShadowMap>>& pShadMaps) override
	{
		for (auto [ent, shadMap] : Registry().view<Components::DirShadowMap>().each())
		{
			if (shadMap.update)
				dShadMaps.push_back(shadMap.shadowMap);
		}
		for (auto [ent, shadMap] : Registry().view<Components::PointShadowMap>().each())
		{
			if (shadMap.update)
				pShadMaps.push_back(shadMap.shadowMap);
		}
	}
};
static Client rendererClient;
static RenderingSystem* instance;
RenderingSystem::~RenderingSystem()
{
	// clear every reference to renderer objects before deleting it
	AssetLoader::Instance().ClearCache();
	
	Registry().clear<Components::Drawable, Components::DirLight, Components::PointLight>();
	renderer = nullptr;
}
RenderingSystem::RenderingSystem()
{
	auto& registry = Registry();
	instance = this;
	registry.on_update<Components::Drawable>().connect<&RenderingSystem::OnDrawableUpdated>(this);
	registry.on_destroy<Components::Drawable>().connect<&RenderingSystem::OnDrawableRemoved>(this);
	registry.on_construct<Components::Drawable>().connect<&RenderingSystem::OnDrawableAdded>(this);

	registry.on_construct<Components::DirShadowMap>().connect<&RenderingSystem::OnDShadAdded>(this);
	registry.on_construct<Components::PointShadowMap>().connect<&RenderingSystem::OnPShadAdded>(this);
	registry.on_destroy<Components::DirShadowMap>().connect<&RenderingSystem::OnDShadRemoved>(this);
	registry.on_destroy<Components::PointShadowMap>().connect<&RenderingSystem::OnPShadRemoved>(this);

	registry.on_construct<Components::DirLight>().connect<&RenderingSystem::OnDLightAdded>(this);
	registry.on_construct<Components::PointLight>().connect<&RenderingSystem::OnPLightAdded>(this);
	registry.on_destroy<Components::DirLight>().connect<&RenderingSystem::OnDLightRemoved>(this);
	registry.on_destroy<Components::PointLight>().connect<&RenderingSystem::OnPLightRemoved>(this);

	dirLightObserver.connect(registry, entt::collector.update<Components::DirLight>());
	pointLightObserver.connect(registry, entt::collector.update<Components::Transform>()
		.where<Components::PointLight>().update<Components::PointLight>());
	transformObserver.connect(registry, entt::collector
		.update<Components::Transform>().where<Components::Drawable>()
		.update<Components::Drawable>()
		.group<Components::Drawable, Components::Transform>());

	renderer = std::make_shared<Renderer::Renderer>();
	renderer->client = &rendererClient;
	renderer->renderPasses.push_back(std::make_shared<ColliderPass>(renderer->mainImage));
	renderer->deviceDatas.push_back(ColliderPass::Instance().vertexBuffer);
	renderer->deviceDatas.push_back(ColliderPass::Instance().indexBuffer);
}
void RenderingSystem::UpdateLights()
{
	auto& registry = Scene::Current().registry;
	for (const auto ent : dirLightObserver)
	{
		auto& dl = registry.get<Components::DirLight>(ent);
		auto ds = registry.try_get<Components::DirShadowMap>(ent);
		dl.RecalcVP();
		if (dl.sun)
		{
			float height = glm::normalize(-dl->direction).y;
			float redness = std::clamp(height, 0.0f, 0.2f) * 5;
			redness = 1 - redness;
			dl->color2 = dl->color - redness * glm::vec4(0.0f, 0.7f, 1.0f, 0.0f);

			dl->color.a = std::clamp(height + 0.2f, 0.0f, 0.3f) * 2.0f;
			// shaders.skyShader->Use().SetVec3("lightPos", -dl.light->direction);
		}
		if (ds)
		{
			ds->shadowMap->vp = dl->VP;
			// for (auto& map : shadowMapProvider.dShadMaps)
			// { // change shadow FBs in provider
			// 	if (map.light == dl.light && map.fb != ds->map.fb)
			// 		map.fb = ds->map.fb;
			// }
		}
	}
	dirLightObserver.clear();
	for (const auto ent : pointLightObserver)
	{
		auto& pl = registry.get<Components::PointLight>(ent);
		auto ps = registry.try_get<Components::PointShadowMap>(ent);
		pl->position = registry.get<Components::Transform>(ent).Position();
		if (ps)
		{
			ps->shadowMap->position = pl->position;
			// for (auto& map : shadowMapProvider.pShadMaps)
			// { // change shadow FBs in provider
			// 	if (map.light == pl.light && map.fb != ps->map.fb)
			// 		map.fb = ps->map.fb;
			// }
		}
	}
	pointLightObserver.clear();
}
void RenderingSystem::Update()
{
	UpdateLights();
	for (auto& ent : transformObserver)
	{
		auto& dr = Registry().get<Components::Drawable>(ent);
		const auto& modelMatrix = Registry().get<Components::Transform>(ent).modelMatrix;
		*dr.sharedTransform = modelMatrix;
		for (size_t i = 0; i < dr.GetModel()->nodes.size(); i++)
		{
			auto& node = dr.GetModel()->nodes[i];
			if (node.localTransform.has_value())
				*dr.transforms[i] = modelMatrix * *node.localTransform;
		}
	}
	transformObserver.clear();
	if (camera)
	{
		renderer->resources->vp = camera->viewProjection;
		renderer->resources->viewPos = camera->position;
	}

	ColliderPass::Instance().UpdateBuffers(Registry());

	renderer->DrawFrame();
}
std::shared_ptr<Renderer::Image> RenderingSystem::LoadImage(const std::string& path)
{
	auto data = Renderer::LoadUtils::LoadImage(path, true);

	// format: is it linear or sRGB? usually the latter but should be checked
	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(data.width, data.height)))) + 1;

	auto texImg = std::make_shared<Renderer::Image>(data.width, data.height, Renderer::Vulkan::Format::rgba8Unorm,
		vk::ImageUsageFlagBits::eTransferSrc // blit (mipmap)
		| vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderSampledRead, mipLevels);

	texImg->Write(data.data.data(), data.data.size(), vk::ImageLayout::eShaderReadOnlyOptimal);
	return texImg;
}
RenderingSystem& RenderingSystem::Instance()
{
	return *instance;
}
}