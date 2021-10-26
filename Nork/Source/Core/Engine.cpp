#include "Engine.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Serialization/BinarySerializer.h"
#include "Modules/Renderer/Pipeline/StreamRenderer.h"
#include "Modules/Renderer/Pipeline/Capabilities.h"

namespace Nork
{
	static std::vector<uint8_t> dShadowIndices;
	static std::vector<uint8_t> pShadowIndices;
	static constexpr auto dShadIdxSize = Renderer::Config::LightData::dirShadowsLimit;
	static constexpr auto pShadIdxSize = Renderer::Config::LightData::pointShadowsLimit;

	static Data::Shader CreateShaderFromPath(std::string_view path)
	{
		std::ifstream stream(path.data());
		std::stringstream buf;
		buf << stream.rdbuf();

		auto data = Data::ShaderData{ .source = buf.str() };
		auto resource = Resource::CreateShader(data);
		stream.close();
		return Data::Shader(resource);
	}

	static Renderer::Data::Shader shader;
	static Renderer::Data::Shader pointShader;
	static Renderer::Data::Shader lineShader;

	DeferredData Engine::CreatePipelineResources()
	{
		using namespace Renderer::Utils::Texture;

		shader = CreateShaderFromPath("Source/Shaders/position.shader");
		pointShader = CreateShaderFromPath("Source/Shaders/point.shader");
		lineShader = CreateShaderFromPath("Source/Shaders/line.shader");
		return DeferredData (DeferredData::Shaders
			{
				.gPass = CreateShaderFromPath("Source/Shaders/gPass.shader"),
				.lPass = CreateShaderFromPath("Source/Shaders/lightPass.shader"),
				.skybox = CreateShaderFromPath("Source/Shaders/skybox.shader"),
			}, 0);
	} // TODO:: cannot do this well...

	static std::vector<Data::Model> GetModels(ECS::Registry& r)
	{
		std::vector<std::pair<std::vector<Data::Mesh>, glm::mat4>> result;
		auto& reg = r.GetUnderlying();

		auto view = reg.view<Components::Model, Components::Transform>();
		result.reserve(view.size_hint());

		for (auto& id : view)
		{
			auto& model = view.get(id)._Myfirst._Val;
			auto& tr = view.get(id)._Get_rest()._Myfirst._Val;

			result.push_back(std::pair(model.meshes, tr.GetModelMatrix()));
		}

		return result;
	}

	void Engine::OnDShadowAdded(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		shad.GetMutableData().idx = dShadowIndices.back();
		dShadowIndices.pop_back();

		// should handle it elsewhere
		auto light = reg.try_get<Components::DirLight>(id);
		if (light != nullptr)
			shad.RecalcVP(light->GetView());
	}
	void Engine::OnDShadowRemoved(entt::registry& reg, entt::entity id)
	{
		auto& shad = reg.get<Components::DirShadow>(id);
		dShadowIndices.push_back(shad.GetData().idx);
	}

	Engine::Engine(EngineConfig& config)
		: window(config.width, config.height), scene(Scene::Scene()), pipeline(CreatePipelineResources()),
		geometryFb(Renderer::Pipeline::GeometryFramebuffer(1920, 1080)),
		lightFb(Renderer::Pipeline::LightPassFramebuffer(geometryFb.Depth(), geometryFb.Width(), geometryFb.Height()))
	{
		using namespace Renderer::Pipeline;
		Renderer::Resource::DefaultResources::Init();

		dShadowFramebuffers.reserve(dShadIdxSize);
		pShadowFramebuffers.reserve(pShadIdxSize);
		for (int i = dShadIdxSize - 1; i > -1; i--)
		{
			dShadowIndices.push_back(i);
			dShadowFramebuffers.push_back(DirShadowFramebuffer(4000, 4000));
		}
		for (int i = pShadIdxSize - 1; i > -1; i--)
		{
			pShadowIndices.push_back(i);
			pShadowFramebuffers.push_back(PointShadowFramebuffer(1000, 1000));
		}

		auto& reg = scene.registry.GetUnderlyingMutable();
		reg.on_construct<Components::DirShadow>().connect<&Engine::OnDShadowAdded>(this);
		reg.on_destroy<Components::DirShadow>().connect<&Engine::OnDShadowRemoved>(this);
		reg.on_update<Components::DirShadow>().connect<&Engine::OnDShadowRemoved>(this);

		using namespace Renderer::Utils::Texture;
		auto skyboxResource = scene.resMan.GetCubemapTexture("Resources/Textures/skybox", ".jpg",
			TextureParams{ .wrap = Wrap::ClampToEdge, .filter = Filter::Linear, .magLinear = false, .genMipmap = false });
		pipeline.data.skyboxTex = skyboxResource.id;

		idMap = geometryFb.Extend<Format::R32UI>();
		lightFb.Extend<Format::R32UI>(idMap);

		colliders.push_back(MeshWorld<Vertex>::GetCube({ 2, 2, 2 }));
		colliders.push_back(MeshWorld<Vertex>::GetCube({ -1, -1, -1 }));
	}

	void Engine::Launch()
	{
		auto& sender = appEventMan.GetSender();
		while (window.IsRunning())
		{
			sender.Send(Event::Types::OnUpdate());
			sender.Send(Event::Types::OnRenderUpdate());
			
			PhysicsUpdate(); // NEW
			auto models = GetModels(this->scene.registry);
			SyncComponents();
			ViewProjectionUpdate();
			UpdateLights();
			pipeline.DrawScene(models, lightFb, geometryFb);
			if (drawSky)
				pipeline.DrawSkybox();
			DrawHitboxes();
			FramebufferBase::UseDefault();
			sender.Send(Event::Types::RenderUpdated());
			window.Refresh();

			sender.Send(Event::Types::Updated());
		}
	}
	void Engine::ReadId(int x, int y)
	{
		uint32_t res;
		Renderer::Utils::Other::ReadPixels(lightFb.GetFBO(), lightFb.ColorAttForExtension(0), x, y, Utils::Texture::Format::R32UI, &res);
		if (res != 0)
		{
			appEventMan.GetSender().Send(Event::Types::IdQueryResult(x, y, res));
		}
	}
	Engine::~Engine()
	{
	}
	void Engine::SyncComponents()
	{
		auto& reg = scene.registry.GetUnderlyingMutable();
		auto pls = reg.view<Components::Transform, Components::PointLight>();

		for (auto& id : pls)
		{
			auto& tr = pls.get(id)._Myfirst._Val;
			auto& pl = pls.get(id)._Get_rest()._Myfirst._Val;

			pl.GetMutableData().position = tr.position;
		}
	}
	void Engine::UpdateLights()
	{
		auto& reg = scene.registry.GetUnderlying();
		auto dLightsWS = reg.view<Components::DirLight, Components::DirShadow>();
		auto pLightsWS = reg.view<Components::PointLight, Components::PointShadow>();
		auto dLights = reg.view<Components::DirLight>(entt::exclude<Components::DirShadow>);
		auto pLights = reg.view<Components::PointLight>(entt::exclude<Components::PointShadow>);
		// for shadow map drawing only
		static auto pShadowMapShader = CreateShaderFromPath("Source/Shaders/pointShadMap.shader");
		static auto dShadowMapShader = CreateShaderFromPath("Source/Shaders/dirShadMap.shader");

		auto modelView = reg.view<Components::Model, Components::Transform>();
		std::vector<std::pair<std::vector<Data::Mesh>, glm::mat4>> models;
		for (auto& id : modelView)
		{
			models.push_back(std::pair(modelView.get(id)._Myfirst._Val.meshes, modelView.get(id)._Get_rest()._Myfirst._Val.GetModelMatrix()));
		}
		// ---------------------------
	
		std::vector<std::pair<Data::DirLight, Data::DirShadow>> DS;
		std::vector<std::pair<Data::PointLight, Data::PointShadow>> PS;
		std::vector<Data::DirLight> DL;
		std::vector<Data::PointLight> PL;
		DS.reserve(dLightsWS.size_hint());
		PS.reserve(pLightsWS.size_hint());
		DL.reserve(dLights.size_hint());
		PL.reserve(pLights.size_hint());

		for (auto& id : dLightsWS)
		{
			const auto& light = dLightsWS.get(id)._Myfirst._Val.GetData();
			const auto& shadow = dLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
			auto pair = std::pair<Data::DirLight, Data::DirShadow>(light, shadow);
			DS.push_back(pair);

			lightMan.DrawDirShadowMap(light, shadow, models, dShadowFramebuffers[shadow.idx], dShadowMapShader);
			pipeline.UseShadowMap(shadow, dShadowFramebuffers[shadow.idx]);
		}
		for (auto& id : pLightsWS)
		{
			auto& light = pLightsWS.get(id)._Myfirst._Val.GetData();
			auto& shadow = pLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();
			PS.push_back(std::pair<Data::PointLight, Data::PointShadow>(light, shadow));

			lightMan.DrawPointShadowMap(light, shadow, models, pShadowFramebuffers[shadow.idx], pShadowMapShader);
			pipeline.UseShadowMap(shadow, pShadowFramebuffers[shadow.idx]);
		}
		for (auto& id : dLights)
		{
			DL.push_back(dLights.get(id)._Myfirst._Val.GetData());
		}
		for (auto& id : pLights)
		{
			PL.push_back(pLights.get(id)._Myfirst._Val.GetData());
		}

		lightMan.Update(DS, PS, DL, PL);

	}
	void Engine::PhysicsUpdate()
	{
		using namespace Physics;
		auto& reg = scene.registry.GetUnderlyingMutable();

		auto view = reg.view<Components::Transform, Components::Model>();

		for (auto id1 : view)
		{
			auto tr1 = view.get(id1)._Myfirst._Val;
			auto& model = view.get(id1)._Get_rest()._Myfirst._Val;
			
			model.meshes[0].colliding = false;

			BoxCollider bc1 = BoxCollider{ .x = tr1.scale.x, .y = tr1.scale.y , .z = tr1.scale.z };

			for (auto id2 : view)
			{
				if (id1 == id2)
					continue;

				auto tr2 = view.get(id2)._Myfirst._Val;
				BoxCollider bc2 = BoxCollider{ .x = tr2.scale.x, .y = tr2.scale.y , .z = tr2.scale.z };

				if (BroadTest(bc1, bc2, tr1.position, tr2.position, tr1.RotationMatrix(), tr2.RotationMatrix()))
				{
					model.meshes[0].colliding = true;
					break;
				}
			}
		}

	}
	void Engine::DrawHitboxes()
	{
		auto col1 = colliders[0].AsCollider();
		auto col2 = colliders[1].AsCollider();

		std::vector<Vertex> planeVerts;

		if (faceQ)
		{
			auto fq = Physics::GetFQ(col1, col2);
			auto fq2 = Physics::GetFQ(col2, col1);
			float val = (fq.faceIdx == -1 || fq2.faceIdx == -1) ? 0 : 1;
			for (auto& vert : colliders[0].vertices)
			{
				vert.selected = val;
			}
			for (auto& vert : colliders[1].vertices)
			{
				vert.selected = val;
			}
		}
		else
		{
			auto fq = Physics::GetEQ(col1, col2);
			//auto fq2 = Physics::GetEQ(col2, col1);
			float val = (fq.distance > 0) ? 0 : 1;
			for (auto& vert : colliders[0].vertices)
			{
				vert.selected = val;
			}
			for (auto& vert : colliders[1].vertices)
			{
				vert.selected = val;
			}
			auto middle = col1.points[fq.edge1[0]] + col1.points[fq.edge1[1]];
			middle /= 2;
			planeVerts.push_back(Vertex(middle));
			planeVerts.push_back(Vertex(middle + fq.normal));
		}
		
		std::vector<Vertex> normVerts;
		std::vector<Vertex> centVerts = { Vertex(col1.center, true), Vertex(col2.center, true)};

		for (auto& face : col1.faces)
		{
			auto center = (col1.points[face.idxs[0]] + col1.points[face.idxs[1]] + col1.points[face.idxs[2]]) / 3.0f;
			auto p2 = center + face.normal;
			normVerts.push_back(Vertex(center));
			normVerts.push_back(Vertex(p2));
		}
		for (auto& face : col2.faces)
		{
			auto center = (col2.points[face.idxs[0]] + col2.points[face.idxs[1]] + col2.points[face.idxs[2]]) / 3.0f;
			auto p2 = center + face.normal;
			normVerts.push_back(Vertex(center));
			normVerts.push_back(Vertex(p2));
		}

		static Renderer::Pipeline::StreamRenderer<glm::vec3, float, uint32_t> renderer;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPointSize(pointSize);

		using namespace Renderer;
		using namespace Capabilities;
		using enum Capability;
		
		if (drawTriangles || drawLines || drawPoints)
		{
			for (auto& mesh : colliders)
			{
				renderer.UploadVertices(std::span(mesh.vertices));

				Capabilities::With<Enable<FaceCulling, Depth>, Disable<Blend>, Set<BlendFunc<GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA>>>([&]()
					{
						Capabilities::With<Enable<Blend>, Disable<Depth, FaceCulling>>([&]()
							{
								if (drawTriangles)
								{
									shader.Use();
									renderer.DrawAsTriangle(*(reinterpret_cast<std::vector<uint32_t>*>(&mesh.triangleIndices)));
								}

								if (drawLines || drawPoints)
								{
									Capabilities::With<Disable<Depth>>([&]()
										{
											if (drawLines)
											{
												lineShader.Use();
												renderer.DrawAsLines(*(reinterpret_cast<std::vector<uint32_t>*>(&mesh.edgeIndices)));
											}
											if (drawPoints)
											{
												std::vector<uint32_t> seq(mesh.vertices.size());
												for (size_t i = 0; i < mesh.vertices.size(); i++)
													seq.push_back(i);
												pointShader.Use();
												renderer.DrawAsPoints(seq);
											}
											if (drawLines)
											{
												lineShader.Use();
												renderer.UploadVertices(std::span(normVerts));
												renderer.DrawAsLines(normVerts.size());
												lineShader.SetVec4("colorDefault", glm::vec4(1, 0, 0, 1));
												renderer.UploadVertices(std::span(planeVerts));
												renderer.DrawAsLines(planeVerts.size());
												lineShader.SetVec4("colorDefault", lineColor);
											}
											if (drawLines)
											{
												glPointSize(pointSize * 5);
												pointShader.Use();
												renderer.UploadVertices(std::span(centVerts));
												renderer.DrawAsPoints(centVerts.size());
												glPointSize(pointSize);
											}
										});
								}
							});
					});
			}
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
	void Engine::ViewProjectionUpdate()
	{
		auto optional = GetActiveCamera();
		if (optional.has_value())
		{
			auto& camera = *optional.value();

			shader.Use();
			shader.SetMat4("VP", camera.viewProjection);
			shader.SetVec4("colorDefault", triangleColor);
			shader.SetVec4("colorSelected", glm::vec4(selectedColor, triAlpha));
			
			pointShader.Use();
			pointShader.SetMat4("VP", camera.viewProjection);
			pointShader.SetFloat("aa", pointAA);
			pointShader.SetFloat("size", pointInternalSize);
			pointShader.SetVec4("colorDefault", pointColor);
			pointShader.SetVec4("colorSelected", glm::vec4(selectedColor, pointAlpha));

			lineShader.Use();
			lineShader.SetMat4("VP", camera.viewProjection);
			lineShader.SetFloat("width", lineWidth);
			lineShader.SetVec4("colorDefault", lineColor);
			lineShader.SetVec4("colorSelected", glm::vec4(selectedColor, lineAlpha));

			//lightMan.dShadowMapShader->SetMat4("VP", vp);
			pipeline.data.shaders.gPass.Use();
			pipeline.data.shaders.gPass.SetMat4("VP", camera.viewProjection);

			pipeline.data.shaders.lPass.Use();
			pipeline.data.shaders.lPass.SetVec3("viewPos", camera.position);

			pipeline.data.shaders.skybox.Use();
			auto vp = camera.projection * glm::mat4(glm::mat3(camera.view));
			pipeline.data.shaders.skybox.SetMat4("VP", vp);
		}
		
	}
}