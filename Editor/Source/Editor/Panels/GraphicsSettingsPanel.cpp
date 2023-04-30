#include "include/GraphicsSettingsPanel.h"
#include "Modules/Renderer/MemoryAllocator.h"
#include "Modules/Renderer/RendererSettings.h"

namespace Nork::Editor {
	GraphicsSettingsPanel::GraphicsSettingsPanel()
	{
	}

	void GraphicsSettingsPanel::Content()
	{
		Renderer::Settings& settings = Renderer::Settings::Instance();
		if (ImGui::TreeNodeEx("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto bloom = settings.bloom.get();
			if (ImGui::SliderScalar("Mip Levels", ImGuiDataType_::ImGuiDataType_U32, &bloom.mipLevels, &bloom.minMipLevels, &bloom.maxMipLevels())) {
				settings.bloom = bloom;
			}
			if (ImGui::Checkbox("Use Blitting for downsampling instead of Compute Shader", &bloom.useBlitFromDownsampling)) {
				settings.bloom = bloom;
			}
			if (bloom.useBlitFromDownsampling) {
				ImGui::SameLine();
				if (ImGui::Checkbox("Linear Filtering", &bloom.blitLinear)) {
					settings.bloom = bloom;
				}
			}
			int kernelRange = bloom.gaussianKernelSize / 2;
			if (ImGui::SliderInt("Kernel Range (Size-1)/2", &kernelRange, bloom.minKernelSize / 2, bloom.maxKernelSize / 2)) {
				bloom.gaussianKernelSize = kernelRange * 2 + 1; // always odd
				settings.bloom = bloom;
			}
			if (ImGui::DragFloat("Kernel Distribution", &bloom.sigma, 0.01f, 0.000001f, 100, "%.6f", ImGuiSliderFlags_::ImGuiSliderFlags_Logarithmic)) {
				settings.bloom = bloom;
			}
			if (!bloom.inlineKernelData && ImGui::Checkbox("Build Kernel Size into Shader", &bloom.inlineKernelSize)) {
				settings.bloom = bloom;
			}
			if (ImGui::Checkbox("Build Kernel Into Shader", &bloom.inlineKernelData)) {
				bloom.inlineKernelSize = true;
				settings.bloom = bloom;
			}
			if (ImGui::Button("Refresh Shaders")) {
				Renderer::Renderer::Instance().RefreshShaders();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Memory", ImGuiTreeNodeFlags_DefaultOpen))
		{
			using namespace Renderer;
			auto& allocator = MemoryAllocator::Instance();

			static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;
			if (ImGui::BeginTable("MemTypes", 6, flags))
			{
				ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("Usage", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Bytes Allocated", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Size", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Flags", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Pools", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				// ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
				ImGui::TableHeadersRow();
				for (auto& type : allocator.types)
				{
					ImGui::TableNextRow();
					auto size = type.heap.size;
					auto sizeKbs = type.heap.size / 1024;

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%d", type.index);
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%d%%", (uint32_t)(100 * (type.allocated / (1024.f) / sizeKbs)));
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%zu", type.allocated);
					ImGui::TableSetColumnIndex(3);
					ImGui::Text("%zu", size);
					ImGui::TableSetColumnIndex(4);
					ImGui::Text("%s", vk::to_string(type.flags).c_str());
					ImGui::TableSetColumnIndex(5);
					ImGui::Text("%d", type.pools.size());
				}
				ImGui::EndTable();
			}
			if (ImGui::BeginTable("Pools", 6, flags))
			{
				ImGui::TableSetupColumn("Type Index", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("Usage", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Bytes Allocated", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Size", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Allocations", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch);
				// ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
				ImGui::TableHeadersRow();
				for (auto& type : allocator.types)
				{
					int idx = -1;
					for (auto& pool : type.pools)
					{
						idx++;
						ImGui::TableNextRow();
						auto sizeKbs = pool->size / 1024;

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%d", type.index);
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%d", idx);
						ImGui::TableSetColumnIndex(2);
						ImGui::Text("%d%%", (uint32_t)(100 * (pool->offset / (1024.f) / sizeKbs)));
						ImGui::TableSetColumnIndex(3);
						ImGui::Text("%zu", pool->offset);
						ImGui::TableSetColumnIndex(4);
						ImGui::Text("%zu", pool->size);
						ImGui::TableSetColumnIndex(5);
						ImGui::Text("%d", pool->allocations.size());
					}
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}
		ImGui::ShowStyleEditor();
	}
}
