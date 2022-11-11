#include "ShadowMapStage.h"
#include "../../State/Capabilities.h"

bool Nork::Renderer::ShadowMapStage::Execute(Framebuffer& source, Framebuffer& destination)
{
	for (auto& map : provider->DirShadowMaps())
		RenderDirShadowMap(map);
	for (auto& map : provider->PointShadowMaps())
		RenderPointShadowMap(map);
    return false;
}

void Nork::Renderer::ShadowMapStage::RenderPointShadowMap(const PointShadowMap& shadowMap)
{
	shadowMap.fb->Bind().SetViewport().Clear();
	pShader->Use();

	auto& pos = shadowMap.light->position;

	glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.0f, shadowMap.shadow->near, shadowMap.shadow->far);

	std::vector<glm::mat4> VP;
	// Adding view
	VP.push_back(glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	VP.push_back(glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	for (int i = 0; i < VP.size(); i++)
	{
		VP[i] = projection * VP[i];
		pShader->SetMat4(("VP[" + std::to_string(i) + "]").c_str(), VP[i]);
	}

	pShader->SetFloat("far", shadowMap.shadow->far);
	pShader->SetVec3("ligthPos", pos);

	Capabilities()
		.Enable().DepthTest().CullFace();
	drawCommand->operator()();
}
void Nork::Renderer::ShadowMapStage::RenderDirShadowMap(const DirShadowMap& shadowMap)
{
	shadowMap.fb->Bind().SetViewport().Clear();
	dShader->Use().SetMat4("VP", shadowMap.light->VP);

	Capabilities()
		.Enable().DepthTest().CullFace();

	drawCommand->operator()();
}
