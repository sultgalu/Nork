#include "Engine.h"

#include "Modules/Renderer/LoadUtils.h"
#include "Core/InputState.h"
#include "App/Application.h"
#include "Modules/Renderer/State/Capabilities.h"
#include "Modules/Renderer/Objects/Framebuffer/FramebufferBuilder.h"
#include "Modules/Renderer/DrawUtils.h"
#include "PolygonBuilder.h"

namespace Nork
{
	Engine* _engine;
	static constexpr bool MULTITHREAD_PHX = false;
	Engine& Engine::Get()
	{
		return *_engine;
	}
	Engine::Engine()
		: uploadSem(1), updateSem(1),
		scriptSystem(scene)
	{
		_engine = this;
		scene.registry.on_construct<Components::Drawable>().connect<&Engine::OnDrawableAdded>(this);
		scene.registry.on_construct<Components::Physics>().connect<&Engine::OnPhysicsAdded>(this);
		scene.registry.on_destroy<Components::Physics>().connect<&Engine::OnPhysicsRemoved>(this);
		transformObserver.connect(scene.registry, entt::collector.update<Components::Transform>());
		renderingSystem.globalShaderUniform.lineColor.a = 0.6f;
		renderingSystem.globalShaderUniform.lineColor.r += 0.1f;
		renderingSystem.globalShaderUniform.lineColor.g += 0.2f;
		renderingSystem.globalShaderUniform.triColor.a = 0.8f;
	}
	void Engine::OnDrawableAdded(entt::registry& reg, entt::entity id)
	{
		auto& dr = reg.get<Components::Drawable>(id);
		dr.model = resourceManager.GetModel("");
	
		auto* tr = reg.try_get<Components::Transform>(id);
		dr.modelMatrix = renderingSystem.drawState.modelMatrixBuffer.Add(glm::identity<glm::mat4>());
	}
	void Engine::OnPhysicsAdded(entt::registry& reg, entt::entity id)
	{
		auto& obj = reg.get<Components::Physics>(id);

		if (reg.any_of<Components::Transform>(id))
		{
			auto& tr = reg.get<Components::Transform>(id);
			obj.handle = physicsSystem.pWorld.AddObject(Physics::Object(Physics::Collider::Cube(), tr.Scale(),
				Physics::KinematicData { .position = tr.Position(), .quaternion = tr.Quaternion() }));
		}
		else
		{
			obj.handle = physicsSystem.pWorld.AddObject(Physics::Object(Physics::Collider::Cube()));
		}
	}
	void Engine::OnPhysicsRemoved(entt::registry& reg, entt::entity id)
	{
		auto& obj = reg.get<Components::Physics>(id);
		physicsSystem.pWorld.RemoveObject(obj.handle);
	}
	void Engine::Update()
	{
		if (physicsUpdate)
		{
			if (MULTITHREAD_PHX)
			{
				uploadSem.acquire();
				updateSem.acquire();
			}
			else
			{
				if (scriptUpdated)
				{
					physicsSystem.Upload(scene.registry, true);
					scriptUpdated = false;
				}
				physicsSystem.Update(scene.registry);
			}

			physicsSystem.Download(scene.registry); // get changes from physics (writes to local Transform)
			UpdateGlobalTransforms(); // apply local changes to global Transform

			if (scriptUpdate)
			{
				scriptSystem.Update();
				UpdateGlobalTransforms();
			}
			scriptUpdated = true; // editor can also update

			if (MULTITHREAD_PHX)
			{
				updateSem.release();
				uploadSem.release();
			}
		}
		else
		{
			UpdateGlobalTransforms(); // Editor changes
		}
		renderingSystem.BeginFrame();
		renderingSystem.Update(); // draw full updated data
		renderingSystem.EndFrame(); 
		Profiler::Clear();
		window.Refresh();
	}
	void Engine::StartPhysics(bool startScript)
	{
		if (physicsUpdate)
			return;
		if (startScript)
		{
			scriptUpdate = true;
			scriptSystem.Update();
		}
		// start physics thread
		physicsSystem.Upload(scene.registry, true);
		physicsUpdate = true;
		if (MULTITHREAD_PHX)
		{
			physicsThread = LaunchPhysicsThread();
		}
	}
	void Engine::StopPhysics()
	{
		scriptUpdate = false;
		physicsUpdate = false;
		if (MULTITHREAD_PHX)
		{
			physicsThread->join();
			delete physicsThread;
		}
	}

	std::thread* Engine::LaunchPhysicsThread()
	{
		return new std::thread([&]()
			{
				while (physicsUpdate)
				{
					updateSem.acquire(); // don't let upload happen until done with updating
					if (scriptUpdated) // if downloading 
					{
						physicsSystem.Upload(scene.registry, true);
						scriptUpdated = false;
					}
					physicsSystem.Update(scene.registry);
					updateSem.release();
					uploadSem.acquire(); // wait until any uploading is done
					uploadSem.release();
				}
			});
	}

	void Engine::UpdateGlobalTransforms()
	{
		auto& reg = scene.registry;
		scene.root->ForEachDescendants([&](SceneNode& node)
			{
				auto* tr = node.GetEntity().TryGetComponent<Components::Transform>();
				if (tr)
				{
					for (auto& child : node.GetChildren())
					{
						auto* childTr = child->GetEntity().TryGetComponent<Components::Transform>();
						if (childTr)
						{
							tr->UpdateChild(*childTr);
						}
					}
					auto& parent = node.GetParent();
					if (!parent.GetEntity().HasComponent<Components::Transform>())
					{
						tr->UpdateGlobalWithoutParent();
					}
				}
			});
	}

	void Engine::UpdateTransformMatrices()
	{
		auto& reg = scene.registry;
		for (auto entity : transformObserver)
		{
			reg.get<Components::Transform>(entity).RecalcModelMatrix();
		}
	}

	CollidersStage::CollidersStage(Scene& scene, Shaders& shaders)
		: scene(scene), shaders(shaders)
	{
		using namespace Renderer;
		vao = VertexArrayBuilder().Attributes({ 3 })
			.VBO(BufferBuilder()
				.Target(BufferTarget::Vertex)
				.Data(nullptr, 1000 * sizeof(glm::vec3))
				.CreateMutable(BufferUsage::StreamDraw))
			.Create();
		fb = FramebufferBuilder()
			.Attachments(FramebufferAttachments()
				.Depth(TextureBuilder()
					.Params(TextureParams::FramebufferTex2DParams())
					.Attributes(TextureAttributes{ .width = 1920, .height = 1080, .format = TextureFormat::Depth32 })
					.Create2DEmpty())
				.Color(TextureBuilder()
					.Params(TextureParams::FramebufferTex2DParams())
					.Attributes(TextureAttributes{ .width = 1920, .height = 1080, .format = TextureFormat::RGBA })
					.Create2DEmpty(), 0))
			.Create();
	}
	bool CollidersStage::Execute(Renderer::Framebuffer& src, Renderer::Framebuffer& dst)
	{
		using namespace Renderer;

		using namespace Components;
		//std::vector<Collider> colls;
		std::vector<std::array<uint32_t, 3>> tris;
		std::vector<std::array<uint32_t, 2>> edges;
		std::vector<glm::vec3> verts;
		scene.registry.view<Transform, Components::Physics>()
			.each([&](entt::entity id, Transform& tr, Components::Physics& phx)
				{
					//colls.push_back(Collider(phx.Collider()));
					auto& coll = phx.Collider();
					auto poly = PolygonBuilder(phx.Collider()).BuildMesh();
					tris.resize(tris.size() + poly.triangles.size());
					std::transform(poly.triangles.begin(), poly.triangles.end(), tris.begin() + tris.size() - poly.triangles.size(), [&verts](const std::array<uint32_t, 3>& tri)
						{
							return std::array<uint32_t, 3> { tri[0] + (uint32_t)verts.size(), tri[1] + (uint32_t)verts.size(), tri[2] + (uint32_t)verts.size() };
						});
					// tris.insert(tris.end(), tri.begin(), tri.end());
					edges.resize(edges.size() + poly.edges.size());
					std::transform(poly.edges.begin(), poly.edges.end(), edges.begin() + edges.size() - poly.edges.size(), [&verts](const std::array<uint32_t, 2>& edge)
						{
							return std::array<uint32_t, 2> { edge[0] + (uint32_t)verts.size(), edge[1] + (uint32_t)verts.size() };
						});
					verts.insert(verts.end(), coll.verts.begin(), coll.verts.end());
				});
		std::dynamic_pointer_cast<Renderer::MutableBuffer>(vao->GetVBO())->Bind()
			.Allocate(verts.size() * sizeof(glm::vec3), verts.data());
		fb->Bind().Clear().SetViewport();

		Renderer::Capabilities()
			.Enable().Blend().DepthTest(Renderer::DepthFunc::Less);
		shaders.lineShader->Use().SetVec4("colorDefault", glm::vec4(0.1f, 0.3f, 1, 0.6f));
		vao->Bind().DrawIndexed((uint32_t*)edges.data(), edges.size() * 2, Renderer::DrawMode::Lines);

		Capabilities()
			.Enable().CullFace().Blend().DepthTest(Renderer::DepthFunc::Less)
			.Disable();
		shaders.colliderShader->Use();
		vao->Bind().DrawIndexed((uint32_t*)tris.data(), tris.size() * 3, Renderer::DrawMode::Triangles);
		// ---------- Contact Points -------------
		auto& pipeline = Engine::Get().physicsSystem.pipeline;
		verts.clear();
		for (auto& c : pipeline.collisions)
		{
			if (c.contactPoints.size() > 0)
				verts.insert(verts.end(), c.contactPoints.begin(), c.contactPoints.end());
		}
		std::dynamic_pointer_cast<Renderer::MutableBuffer>(vao->GetVBO())->Bind()
			.Allocate(verts.size() * sizeof(glm::vec3), verts.data());
		Renderer::Capabilities()
			.Disable().Blend().DepthTest();
		shaders.pointShader->Use().SetVec4("colorDefault", glm::vec4(1, 1, 1, 1));
		vao->Bind().Draw(Renderer::DrawMode::Points);
		// ---------- Collision Points -------------
		for (auto& c : pipeline.collisions)
		{
			if (!c.isColliding)
				continue;
			verts.clear();
			if (c.satRes.type == Nork::Physics::CollisionType::FaceVert)
			{
				verts.push_back(c.Collider2().verts[c.satRes.featureIdx2]);
				std::dynamic_pointer_cast<Renderer::MutableBuffer>(vao->GetVBO())->Bind()
					.Allocate(verts.size() * sizeof(glm::vec3), verts.data());
				shaders.pointShader->Use().SetVec4("colorDefault", glm::vec4(1, 1, 0, 1));
				vao->Bind().Draw(Renderer::DrawMode::Points);
			}
		}
		// ---------- Collision Dirs -------------
		for (auto& c : pipeline.collisions)
		{
			if (!c.isColliding)
				continue;
			verts.clear();
			if (c.satRes.type == Nork::Physics::CollisionType::FaceVert)
			{
				verts.push_back(c.Collider2().verts[c.satRes.featureIdx2]);
				verts.push_back(c.Collider2().verts[c.satRes.featureIdx2] - c.satRes.dir * c.satRes.depth);
				std::dynamic_pointer_cast<Renderer::MutableBuffer>(vao->GetVBO())->Bind()
					.Allocate(verts.size() * sizeof(glm::vec3), verts.data());
				shaders.lineShader->Use().SetVec4("colorDefault", glm::vec4(1, 1, 0, 1));
				vao->Bind().Draw(Renderer::DrawMode::Lines);
			}
			else if (c.satRes.type == Nork::Physics::CollisionType::EdgeEdge)
			{
				auto& edge1 = c.Collider1().edges[c.satRes.featureIdx1];
				auto& edge2 = c.Collider2().edges[c.satRes.featureIdx2];
				verts.push_back(c.Collider2().verts[edge2.first]);
				verts.push_back(c.Collider2().verts[edge2.second]);
				verts.push_back(c.Collider1().verts[edge1.first]);
				verts.push_back(c.Collider1().verts[edge1.second]);
				std::dynamic_pointer_cast<Renderer::MutableBuffer>(vao->GetVBO())->Bind()
					.Allocate(verts.size() * sizeof(glm::vec3), verts.data());
				shaders.lineShader->Use().SetVec4("colorDefault", glm::vec4(1, 1, 0, 1));
				vao->Bind().Draw(Renderer::DrawMode::Lines);
			}
		}

		// ---------- END -------------
		fb->Color()->Bind2D();
		src.Bind();
		Renderer::Capabilities()
			.Disable().DepthTest();
		shaders.textureShader->Use();
		Renderer::DrawUtils::DrawQuad();
		return false;
	}
}