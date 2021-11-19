#include "Engine.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Serialization/BinarySerializer.h"
#include "Modules/Renderer/Pipeline/StreamRenderer.h"
#include "Modules/Renderer/Pipeline/Capabilities.h"
#include "Modules/Physics/Pipeline/CollisionDetectionGPU.h"

namespace Nork
{
	static std::vector<uint8_t> dShadowIndices;
	static std::vector<uint8_t> pShadowIndices;
	static constexpr auto dShadIdxSize = 10;
	static constexpr auto pShadIdxSize = 10;

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
		lightMan.SetDebug(CreateShaderFromPath("Source/Shaders/debug.shader"));
		return DeferredData (DeferredData::Shaders
			{
				.gPass = CreateShaderFromPath("Source/Shaders/gPass.shader"),
				.lPass = CreateShaderFromPath("Source/Shaders/lightPass.shader"),
				.skybox = CreateShaderFromPath("Source/Shaders/skybox.shader"),
				//.extra = CreateShaderFromPath("Source/Shaders/debug.shader"),
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
		lightFb(Renderer::Pipeline::LightPassFramebuffer(geometryFb.Depth(), geometryFb.Width(), geometryFb.Height())),
		pSystem(), lightMan(CreateShaderFromPath("Source/Shaders/lightCull.shader"))
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

		using namespace Renderer::Utils::Texture;
		auto skyboxResource = scene.resMan.GetCubemapTexture("Resources/Textures/skybox", ".jpg",
			TextureParams{ .wrap = Wrap::ClampToEdge, .filter = Filter::Linear, .magLinear = false, .genMipmap = false });
		pipeline.data.skyboxTex = skyboxResource.id;

		idMap = geometryFb.Extend<Format::R32UI>();
		lightFb.Extend<Format::R32UI>(idMap);
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
			Profiler::Clear();
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
	
		std::vector<Data::DirShadow> DS;
		std::vector<Data::PointShadow> PS;
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
			DL.push_back(light);
			DS.push_back(shadow);

			lightMan.DrawDirShadowMap(light, shadow, models, dShadowFramebuffers[shadow.idx], dShadowMapShader);
			pipeline.UseShadowMap(shadow, dShadowFramebuffers[shadow.idx]);
		}
		for (auto& id : pLightsWS)
		{
			auto& light = pLightsWS.get(id)._Myfirst._Val.GetData();
			auto& shadow = pLightsWS.get(id)._Get_rest()._Myfirst._Val.GetData();

			PL.push_back(light);
			PS.push_back(shadow);

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
		auto& cam = scene.GetMainCamera();
		lightMan.SetDirLightData(DL, DS);
		//lightMan.SetPointLightData(PL, PS);
		lightMan.SetPointLightData(PL, PS, cam.view, cam.projection);
		lightMan.Update();
	}

	static std::vector<std::pair<std::string, float>> deltas;
	std::vector<std::pair<std::string, float>> Engine::GetDeltas()
	{
		return deltas;
	}
	void Engine::PhysicsUpdate()
	{
		static Timer deltaTimer(-20);
		float delta = deltaTimer.ElapsedSeconds();
		deltaTimer.Restart();
		if (delta > 0.2f)
			return;

		using namespace Physics;
		using namespace Components;

		if (!physicsUpdate) return;
		//delta = 0.001f;
		delta *= physicsSpeed;
		deltas.clear();
		Timer t;

		auto& reg = scene.registry.GetUnderlyingMutable();
		auto view = reg.view<Components::Transform, Kinematic>(entt::exclude_t<Polygon>());
		auto collOnlyView = reg.view<Components::Transform, Polygon>(entt::exclude_t<Kinematic>());
		auto collView = reg.view<Components::Transform, Kinematic, Polygon>();
		deltas.push_back(std::pair("get entt views", t.Reset()));

		pWorld.kinems.clear();

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Polygon& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{ 
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass, 
					.isStatic = false, .forces = kin.forces });
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{ 
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = glm::vec3(0),.w = glm::vec3(0), .mass = 1, 
					.isStatic = true, .forces = glm::vec3(0),  });
			});

		view.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				pWorld.kinems.push_back(Physics::KinematicData{
					.position = tr.position, .quaternion = tr.quaternion,
					.velocity = kin.velocity, .w = kin.w, .mass = kin.mass, 
					.isStatic = false, .forces = kin.forces });
			});
		deltas.push_back(std::pair("update kinems", t.Reset()));

		static bool first = true;
		if (updatePoliesForPhysics)
		{
			std::vector<Collider> colls;

			collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Polygon& poly)
				{
					colls.push_back(poly.AsCollider());
				});

			collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
				{
					colls.push_back(poly.AsCollider());
				});

			pSystem.SetColliders(colls);
			updatePoliesForPhysics = false;
		}
		static std::vector<glm::vec3> translations;
		static std::vector<glm::quat> quaternions;
		translations.clear();
		translations.reserve(reg.size<Transform>());
		quaternions.clear();
		quaternions.reserve(reg.size<Transform>());
		deltas.push_back(std::pair("clear model bufs", t.Reset()));

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Polygon& poly)
			{
				translations.push_back(tr.position);
				quaternions.push_back(tr.quaternion);
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Polygon& poly)
			{
				translations.push_back(tr.position);
				quaternions.push_back(tr.quaternion);
			});
		deltas.push_back(std::pair("fill model buf", t.Reset()));

		pSystem.SetModels(translations, quaternions);
		deltas.push_back(std::pair("psystem.setmodels", t.Reset()));
		pSystem.Update(delta);
		deltas.push_back(std::pair("psystem.update", t.Reset()));

		uint32_t i = 0;

		auto kinView = reg.view<Transform, Kinematic>();
		deltas.push_back(std::pair("getKinView", t.Reset()));
		kinView.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				tr.position = pWorld.kinems[i].position;
				kin.velocity = pWorld.kinems[i].velocity;
				kin.forces = pWorld.kinems[i].forces;
				tr.quaternion = pWorld.kinems[i].quaternion;
				kin.w = pWorld.kinems[i++].w;
			});
		deltas.push_back(std::pair("read back psystem results", t.Reset()));
	}
	void Engine::DrawHitboxes()
	{
		static Renderer::Pipeline::StreamRenderer<glm::vec3, float, uint32_t> renderer;
		static Renderer::Pipeline::StreamRenderer<glm::vec3> renderer2;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPointSize(pointSize);

		using namespace Renderer;
		using namespace Capabilities;
		using enum Capability;

		auto view = scene.registry.GetUnderlyingMutable().view<Components::Transform, Polygon>();
		
		auto asVertices = [](std::span<glm::vec3> verts, glm::mat4 model)
		{
			std::vector<Components::Vertex> res;
			res.reserve(verts.size());
			for (size_t i = 0; i < verts.size(); i++)
			{
				res.push_back(Components::Vertex(glm::vec3(model * glm::vec4(verts[i], 1))));
			}
			return res;
		};

		if (drawPolies || sat)
		{
			Capabilities::With<Enable<FaceCulling, Depth>, Disable<Blend>, Set<BlendFunc<GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA>>>([&]()
				{
					Capabilities::With<Enable<Blend>, Disable<Depth, FaceCulling>>([&]()
						{
							if (drawPolies)
							{
								view.each([&](entt::entity id, Components::Transform& tr, Polygon& poly)
									{
										std::vector<Components::Vertex> verts = asVertices(poly.vertices, tr.TranslationRotationMatrix());
										renderer.UploadVertices(std::span(verts));

										if (drawTriangles)
										{
											shader.Use();
											renderer.DrawAsTriangle(*(reinterpret_cast<std::vector<uint32_t>*>(&poly.tris)));
										}
										if (drawLines)
										{
											auto faces = poly.GetFaces();
											auto edges = poly.GetEdges(faces);
											lineShader.Use();
											renderer.DrawAsLines(std::span((uint32_t*)(edges.data()), edges.size() * 2));
										}
										if (drawPoints)
										{
											std::vector<uint32_t> seq(poly.vertices.size());
											for (size_t i = 0; i < poly.vertices.size(); i++)
												seq.push_back(i);
											pointShader.Use();
											renderer.DrawAsPoints(seq);
										}
										if (drawPoints)
										{
											renderer2.UploadVertices(std::span(pSystem.contactPoints));
											pointShader.SetVec4("colorDefault", glm::vec4(1, 1, 1, 1));
											renderer2.DrawAsPoints(pSystem.contactPoints.size());

											//auto md = Physics::MD::MinkowskiDifference(pWorld.shapes[0].verts, pWorld.shapes[1].verts);
											//renderer2.UploadVertices(std::span(md));
											//renderer2.DrawAsPoints(md.size());
											
											/*pointShader.SetVec4("colorDefault", glm::vec4(0, 1, 0, 1));
											std::vector<glm::vec3> Center{ {0, 0, 0} };
											renderer2.UploadVertices(std::span(Center));
											renderer2.DrawAsPoints(1);*/

											pointShader.SetVec4("colorDefault", pointColor);
										}

										
									});
							}
						});
				});
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
	void Engine::ViewProjectionUpdate()
	{
		auto& camera = scene.GetMainCamera();

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

	static Renderer::Data::Shader setupShaderRes, aabbShaderRes, satShaderRes;
	std::array<GLuint, 3> Physics::CollisionDetectionGPU::Get_Setup_AABB_SAT_Shaders()
	{
		setupShaderRes = CreateShaderFromPath("Source/Shaders/colliderTransform.shader");
		aabbShaderRes = CreateShaderFromPath("Source/Shaders/aabb.shader");
		satShaderRes = CreateShaderFromPath("Source/Shaders/sat.shader");

		return { setupShaderRes.GetProgram() , aabbShaderRes.GetProgram() , satShaderRes.GetProgram() };
	}
	void Physics::CollisionDetectionGPU::FreeShaders()
	{

	}
}