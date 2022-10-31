#include "include/ColliderEditorPanel.h"
#include "Modules/Renderer/Pipeline/Stages/BloomStage.h"
#include "Modules/Renderer/Pipeline/Stages/SkyStage.h"
#include "Modules/Renderer/Pipeline/Stages/SkyboxStage.h"
#include "Modules/Renderer/Pipeline/Stages/PostProcessStage.h"
#include "Modules/Renderer/State/Capabilities.h"
#include "Modules/Renderer/Objects/Buffer/BufferBuilder.h"

namespace Nork::Editor {
	class ColliderStage : public Renderer::Stage
	{
	public:
		struct UboData
		{
			uint32_t x = 0, y = 0;
			uint32_t id = INVALID_IDX;
		};
		ColliderStage(Components::Collider& coll, RenderingSystem& system, ViewportView::MouseState& mouseState)
			: system(system), mouseState(mouseState), coll(coll)
		{
			pointShader = system.shaders.pointShader;
			lineShader = system.shaders.lineShader;
			triShader = system.shaders.colliderShader;
			using namespace Renderer;
			vao = VertexArrayBuilder().Attributes({ 3, 1 })
				.VBO(BufferBuilder()
					.Target(BufferTarget::Vertex)
					.Data(coll.Points().data(), coll.Points().size() * sizeof(glm::vec3))
					.CreateMutable(BufferUsage::StreamDraw))
				.Create();
			UboData uboData;
			ubo = BufferBuilder()
				.Flags(ReadAccess | WriteAccess | Persistent)
				.Target(BufferTarget::SSBO)
				.Data(&uboData, sizeof(UboData))
				.Create();
			ubo->BindBase(10);
			this->uboData = (UboData*)ubo->Bind().Map(BufferAccess::ReadWrite);
		}
		struct VertexData
		{
			glm::vec3 pos;
			uint32_t id;
		};
		bool Execute(Renderer::Framebuffer& src, Renderer::Framebuffer& dst)
		{
			static GLsync sync = nullptr;

			uboData->x = mouseState.mousePosX;
			uboData->y = mouseState.mousePosY;
			Logger::Debug(uboData->id);
			std::vector<VertexData> data(coll.Points().size());
			uint32_t id = 0;
			std::transform(coll.Points().begin(), coll.Points().end(), data.begin(), [&id](const glm::vec3 pos)
				{
					return VertexData{ .pos = pos, .id = id++ };
				});
			std::dynamic_pointer_cast<Renderer::MutableBuffer>(vao->GetVBO())->Bind()
				.Allocate(data.size() * sizeof(VertexData), data.data());

			src.Bind();
			if (sync != nullptr)
			{
				glClientWaitSync(sync, 0, 100 * 1000);
				hoveredVert = uboData->id;
				uboData->id = INVALID_IDX;
			}
			RenderPoints();
			RenderEdges();
			RenderFaces();
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
			glDeleteSync(sync);
			sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			if (hoveredVert != INVALID_IDX)
			{
				pointShader->Use().SetInt("selected", 1);
				Renderer::Capabilities()
					.Disable().Blend().DepthTest();
				vao->DrawIndexed(&hoveredVert, 1, Renderer::DrawMode::Points);
				pointShader->SetInt("selected", 0);
			}

			RenderSelectedPoints();
			RenderSelectedEdges();
			RenderSelectedFaces();
			return false;
		}
	private:
		void RenderPoints()
		{
			Renderer::Capabilities()
			 	.Enable().Blend().DepthTest(Renderer::DepthFunc::Less);
			pointShader->Use();
			vao->Bind().Draw(Renderer::DrawMode::Points);
		}
		void RenderSelectedPoints()
		{
			Renderer::Capabilities()
				.Disable().Blend().DepthTest();
			pointShader->Use().SetInt("selected", 1);
			vao->DrawIndexed(selectedVerts.data(), selectedVerts.size(), Renderer::DrawMode::Points);
			pointShader->SetInt("selected", 0);
		}
		void RenderEdges()
		{
			Renderer::Capabilities()
				.Enable().Blend().DepthTest(Renderer::DepthFunc::Less);
			lineShader->Use();
			std::vector<uint32_t> v = coll.EdgeIndices();
			vao->Bind().DrawIndexed(v.data(), v.size(), Renderer::DrawMode::Lines);
		}
		void RenderSelectedEdges()
		{
			std::vector<uint32_t> edgeIndices;
			for (auto& edge : coll.Edges())
			{
				if (std::find(selectedVerts.begin(), selectedVerts.end(), edge.first) != selectedVerts.end() &&
					std::find(selectedVerts.begin(), selectedVerts.end(), edge.second) != selectedVerts.end())
				{
					edgeIndices.push_back(edge.first);
					edgeIndices.push_back(edge.second);
				}
			}
			Renderer::Capabilities()
				.Disable().Blend().DepthTest();
			lineShader->Use().SetInt("selected", 1);
			vao->Bind().DrawIndexed(edgeIndices.data(), edgeIndices.size(), Renderer::DrawMode::Lines);
			lineShader->SetInt("selected", 0);
		}
		void RenderSelectedFaces()
		{
			std::vector<uint32_t> faceIndices;
			for (auto& face : coll.Faces())
			{
				bool contained = true;
				for (auto idx : face.points)
				{
					if (std::find(selectedVerts.begin(), selectedVerts.end(), idx) == selectedVerts.end())
					{
						contained = false;
						break;
					}
				}
				if (contained)
				{
					auto tris = coll.TriangulateFace(face);
					faceIndices.insert(faceIndices.end(), tris.begin(), tris.end());
				}
			}
			Renderer::Capabilities()
				.Disable().Blend().DepthTest();
			triShader->Use().SetInt("selected", 1);
			vao->Bind().DrawIndexed(faceIndices.data(), faceIndices.size(), Renderer::DrawMode::Triangles);
			triShader->SetInt("selected", 0);
		}
		void RenderFaces()
		{
			Renderer::Capabilities()
				.Enable().CullFace().Blend().DepthTest(Renderer::DepthFunc::Less)
				.Disable();
			triShader->Use();
			std::vector<uint32_t> v = coll.TriangleIndices();
			vao->Bind().DrawIndexed(v.data(), v.size(), Renderer::DrawMode::Triangles);
		}
		void Render()
		{
			/*Renderer::Capabilities()
				.Enable().Blend(Renderer::BlendFunc::SrcAlpha_1MinuseSrcAlpha).CullFace()
				.Disable().DepthTest();
			shaders.colliderShader->Use();

			auto count = coll.Points().size();
			auto triCount = coll.TriangleCount() * 3;

			if (!colliderVao || count * sizeof(glm::vec3) > colliderVao->GetVBO()->GetSize())
			{
				CreateCollidersVao(count);
			}
			auto ptr = (glm::vec3*)colliderVao->GetVBO()->GetPersistentPtr();
			auto view = registry.view<Components::Collider, Components::Transform>();
			static std::vector<uint32_t> inds;
			inds.resize(triCount);

			int idx = 0;
			int triIdx = 0;
			int triBase = 0;
			for (auto& p : coll.Points())
			{
				ptr[idx++] = glm::vec4(p, 1.0f);
			}

			for (auto& idx : coll.TriangleIndices())
			{
				inds[triIdx++] = triBase + idx;
			}
			triBase += coll.Points().size();
			colliderVao->Bind().DrawIndexed(std::span(inds));*/
		}
	public:
		std::shared_ptr<Renderer::Shader> pointShader;
		std::shared_ptr<Renderer::Shader> lineShader;
		std::shared_ptr<Renderer::Shader> triShader;
		std::shared_ptr<Renderer::VertexArray> vao;
		std::shared_ptr<Renderer::Buffer> ubo;
		UboData* uboData;
		Components::Collider& coll;
		static constexpr uint32_t INVALID_IDX = std::numeric_limits<uint32_t>().max();
		uint32_t hoveredVert = INVALID_IDX;
		std::vector<uint32_t> selectedVerts;
		RenderingSystem& system;
		ViewportView::MouseState& mouseState;
	};

	ColliderEditorPanel::ColliderEditorPanel(Entity& ent)
		: ViewportPanel(), ent(ent)
	{
		auto& coll = ent.GetComponent<Components::Physics>().LocalCollider();
		collider = Components::Collider(coll);

		camera = std::make_shared<Components::Camera>();
		GetCommonData().editorCameras.push_back(camera);

		sceneView = std::make_shared<SceneView>(1920, 1080, Renderer::TextureFormat::RGBA16F);
		sceneView->pipeline->stages.push_back(std::make_shared<ColliderStage>(collider, GetRenderer(), viewportView.mouseState));
		GetRenderer().sceneViews.insert(sceneView);

		viewportView.sceneView = sceneView;

		panelState.windowFlags = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoScrollbar;

		GetRenderer().GetGlobalShaderUniform().lineColor = glm::vec4(0.2, 0.5, 1, 0.7f);
		GetRenderer().GetGlobalShaderUniform().triColor = glm::vec4(0, 1, 0, 0.7f);
		GetRenderer().GetGlobalShaderUniform().pointColor = glm::vec4(1, 0, 0, 1.0f);
	}
	ColliderEditorPanel::~ColliderEditorPanel()
	{

	}
	void ColliderEditorPanel::Content()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Colors"))
			{
				ImGui::ColorEdit4("Point", &GetRenderer().GetGlobalShaderUniform().pointColor.r);
				ImGui::ColorEdit4("Line", &GetRenderer().GetGlobalShaderUniform().lineColor.r);
				ImGui::ColorEdit4("Face", &GetRenderer().GetGlobalShaderUniform().triColor.r);
				for (size_t i = 0; i < GetCommonData().editorCameras.size(); i++)
				{
					if (ImGui::Selectable(std::to_string(i).c_str()))
					{
						sceneView->camera = GetCommonData().editorCameras[i];
					}
				}
				ImGui::EndMenu();
			}
			ImGui::DragFloat("Cam Base Speed", &sceneView->camera->moveSpeed, 0.001f);
			std::stringstream ss;
			ss << "mouse: (" << viewportView.mouseState.mousePosX << ";"
				<< viewportView.mouseState.mousePosY << ")";
			ImGui::Text(ss.str().c_str());
			ImGui::EndMenuBar();
		}
		viewportView.Content();
		auto stage = sceneView->pipeline->Get<ColliderStage>();
		if (viewportView.mouseState.isViewportClicked)
		{
			if (stage->hoveredVert != stage->INVALID_IDX)
			{
				auto it = std::find(stage->selectedVerts.begin(), stage->selectedVerts.end(), stage->hoveredVert);
				if (it != stage->selectedVerts.end())
					stage->selectedVerts.erase(it);
				else
					stage->selectedVerts.push_back(stage->hoveredVert);
			}
		}
		glm::vec3 move = glm::vec3(0);
		if (ImGui::DragFloat3("move##collPoints", &move.x))
		{
			for (auto& idx : stage->selectedVerts)
			{
				collider.PointsMutable()[idx] += move;
			}
		}
		if (ImGui::Button("Save##Collider"))
		{
			auto& coll = ent.GetComponent<Components::Physics>().LocalCollider();
			coll = collider.ToPhysicsCollider();
		}
	}
}
