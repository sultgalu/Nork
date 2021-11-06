#include "Engine.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/DefaultResources.h"
#include "Serialization/BinarySerializer.h"
#include "Modules/Renderer/Pipeline/StreamRenderer.h"
#include "Modules/Renderer/Pipeline/Capabilities.h"
#include "Modules/Physics/CollisionDetection/SAT.h"
#include "Modules/Physics/CollisionDetection/GJK.h"
#include "Modules/Physics/CollisionDetection/Clip.h"
#include "Modules/Physics/CollisionDetection/AABB.h"

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
	void Engine::UpdatePoliesForPhysics()
	{
		using namespace Components;

		auto& reg = scene.registry.GetUnderlyingMutable();
		auto collView = reg.view<Components::Transform, Poly>();

		pWorld.edges.clear();
		pWorld.faces.clear();
		pWorld.verts.clear();
		pWorld.fNorm.clear();
		pWorld.shapes.clear();
		//pWorld.ClearShapeData();

		int i = 0;
		// order!! -> start with kinems
		Timer t2;
		collView.each([&](entt::entity id, Transform& tr, Poly& poly)
			{
				poly.AddToWorld(pWorld, tr.TranslationRotationMatrix());
			});
		targetDelta = t2.Elapsed();
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
		static Timer t(-20);
		float delta = t.ElapsedSeconds();
		t.Restart();
		if (delta > 0.2f) 
			return;

		using namespace Physics;
		using namespace Components;
		static auto compute = CreateShaderFromPath("Source/Shaders/collision.shader");

		if (!physicsUpdate) return;

		auto& reg = scene.registry.GetUnderlyingMutable();
		auto view = reg.view<Components::Transform, Kinematic>(entt::exclude_t<Poly>());
		auto collOnlyView = reg.view<Components::Transform, Poly>(entt::exclude_t<Kinematic>());
		auto collView = reg.view<Components::Transform, Kinematic, Poly>();

		pWorld.kinems.clear();

		collView.each([&](entt::entity id, Transform& tr, Kinematic& kin, Poly& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{ .position = tr.position, .quaternion = tr.quaternion, 
					.velocity = kin.velocity, .aVelUp = kin.aVelUp, .aVelSpeed = kin.aVelSpeed, .mass = kin.mass, .isStatic = false, .forces = kin.forces });
			});

		collOnlyView.each([&](entt::entity id, Transform& tr, Poly& poly)
			{
				pWorld.kinems.push_back(Physics::KinematicData{ .position = tr.position, .quaternion = tr.quaternion,
					.velocity = glm::vec3(0), .aVelUp = glm::vec3(0), .aVelSpeed = 0, .mass = 1, .isStatic = true, .forces = glm::vec3(0)});
			});

		view.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				pWorld.kinems.push_back(Physics::KinematicData{ .position = tr.position, .quaternion = tr.quaternion, 
					.velocity = kin.velocity,  .aVelUp = kin.aVelUp, .aVelSpeed = kin.aVelSpeed,.mass = kin.mass, .isStatic = false, .forces = kin.forces });
			});

		UpdatePoliesForPhysics();

		compute.Use();
		pSystem.Update(pWorld, delta);

		uint32_t i = 0;

		auto kinView = reg.view<Transform, Kinematic>();
		kinView.each([&](entt::entity id, Transform& tr, Kinematic& kin)
			{
				tr.position = pWorld.kinems[i].position;
				kin.velocity = pWorld.kinems[i].velocity;
				kin.aVelSpeed = pWorld.kinems[i].aVelSpeed;
				kin.aVelUp = pWorld.kinems[i].aVelUp;
				kin.forces = pWorld.kinems[i].forces;
				tr.quaternion = pWorld.kinems[i++].quaternion;
			});
	}
	void Engine::DrawHitboxes()
	{
		/*auto world1 = colliders[0].AsWorld();
		auto world2 = colliders[1].AsWorld();
		auto shape1 = world1.shapes[0];
		auto shape2 = world2.shapes[0];

		if (gjk) gjkRes = Physics::GJK(shape1.verts, shape2.verts).GetResult(shape1.center, shape2.center);
		if (clip) clipRes = Physics::Clip(shape1, shape2).GetResult();
		if(aabb) aabbRes = Physics::AABBTest(shape1, shape2).GetResult();*/

		static Renderer::Pipeline::StreamRenderer<glm::vec3, float, uint32_t> renderer;
		static Renderer::Pipeline::StreamRenderer<glm::vec3> renderer2;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPointSize(pointSize);

		using namespace Renderer;
		using namespace Capabilities;
		using enum Capability;

		auto view = scene.registry.GetUnderlyingMutable().view<Components::Transform, Poly>();
		
		if (drawPolies || sat)
		{
			Capabilities::With<Enable<FaceCulling, Depth>, Disable<Blend>, Set<BlendFunc<GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA>>>([&]()
				{
					Capabilities::With<Enable<Blend>, Disable<Depth, FaceCulling>>([&]()
						{
							if (drawPolies)
							{
								view.each([&](entt::entity id, Components::Transform& tr, Poly& poly)
									{
										renderer.UploadVertices(poly.vertices, tr.TranslationRotationMatrix());

										if (drawTriangles)
										{
											shader.Use();
											renderer.DrawAsTriangle(*(reinterpret_cast<std::vector<uint32_t>*>(&poly.triangleIndices)));

										}
										if (drawLines)
										{
											lineShader.Use();
											renderer.DrawAsLines(*(reinterpret_cast<std::vector<uint32_t>*>(&poly.edgeIndices)));
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
											renderer2.UploadVertices(std::span(pSystem.clipContactPoints));
											pointShader.SetVec4("colorDefault", glm::vec4(1, 1, 1, 1));
											renderer2.DrawAsPoints(pSystem.clipContactPoints.size());
											pointShader.SetVec4("colorDefault", pointColor);
										}
									});
							}
							
							/*if (!sat) return;
							for (size_t i = 0; i < pSystem.detectionResults.size(); i++)
							{
								auto& satRes = pSystem.detectionResults[i].second;
								auto& objs = pSystem.detectionResults[i].first;
								auto shape1 = pWorld.shapes[objs.first];
								auto shape2 = pWorld.shapes[objs.second];

								if (satRes.resType == satRes.EdgeAndEdge)
								{
									std::vector<Components::Vertex> data;
									data.push_back(shape1.FirstVertFromEdge(satRes.edgeAndEdge.first));
									data.push_back(shape1.SecondVertFromEdge(satRes.edgeAndEdge.first));
									data.push_back(shape2.FirstVertFromEdge(satRes.edgeAndEdge.second));
									data.push_back(shape2.SecondVertFromEdge(satRes.edgeAndEdge.second));

									data.push_back(shape1.EdgeMiddle(satRes.edgeAndEdge.first));
									data.push_back(shape1.EdgeMiddle(satRes.edgeAndEdge.first) + satRes.direction * satRes.distance);

									renderer.UploadVertices(std::span(data));
									lineShader.Use();
									lineShader.SetVec4("colorDefault", glm::vec4(1, 1, 0, 1));
									renderer.DrawAsLines(data.size() - 2);
									lineShader.SetVec4("colorDefault", glm::vec4(1, 0, 0, 1));
									renderer.DrawAsLines(2, data.size() - 2);
									lineShader.SetVec4("colorDefault", lineColor);
								}
								else
								{
									bool useFaceSecond = satRes.resType == satRes.VertAndFace;
									std::vector<Components::Vertex> data;
									data.push_back(useFaceSecond ? satRes.vertAndFace.first : satRes.faceAndVert.second);
									auto verts = useFaceSecond ? shape2.VerticesVector(satRes.vertAndFace.second)
										: shape1.VerticesVector(satRes.faceAndVert.first);
									for (size_t i = 0; i < verts.size(); i++)
									{
										data.push_back(verts[i].get());
									}

									auto faceCenter = useFaceSecond ? shape2.FaceCenter(satRes.vertAndFace.second)
										: shape1.FaceCenter(satRes.faceAndVert.first);
									data.push_back(faceCenter);
									data.push_back(faceCenter + satRes.direction * satRes.distance);

									renderer.UploadVertices(std::span(data));
									pointShader.Use();
									pointShader.SetVec4("colorDefault", glm::vec4(1, 1, 0, 1));
									renderer.DrawAsPoints(1);
									pointShader.SetVec4("colorDefault", pointColor);

									shader.Use();
									shader.SetVec4("colorDefault", glm::vec4(1, 1, 0, 1));
									renderer.DrawAsTriangle(verts.size(), 1);
									shader.SetVec4("colorDefault", pointColor);

									lineShader.Use();
									lineShader.SetVec4("colorDefault", glm::vec4(1, 0, 0, 1));
									renderer.DrawAsLines(2, verts.size() + 1);
									lineShader.SetVec4("colorDefault", lineColor);
								}
							}*/

							/*if (sat)
							{
								auto satRes = Physics::SAT(shape1, shape2).GetResult();
								this->satRes = satRes.distance > 0;

								if (resolveCollision && this->satRes)
								{
									glm::vec3 translate = satRes.direction * satRes.distance;
									translate /= 2.0f;
									for (size_t i = 0; i < colliders[0].vertices.size(); i++)
									{
										colliders[0].vertices[i].pos += translate;
									}
									for (size_t i = 0; i < colliders[1].vertices.size(); i++)
									{
										colliders[1].vertices[i].pos -= translate;
									}
								}

								
							}*/

							
						});
				});
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