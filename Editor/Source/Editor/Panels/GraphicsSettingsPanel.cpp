#include "include/GraphicsSettingsPanel.h"
#include "Modules/Renderer/MemoryAllocator.h"
#include "Modules/Renderer/RendererSettings.h"

namespace Nork::Editor {
	GraphicsSettingsPanel::GraphicsSettingsPanel()
	{
	}

	void GraphicsSettingsPanel::Content()
	{
		LiveData<Renderer::Settings>& liveSettings = Renderer::Settings::Instance();
		auto settings = *liveSettings;
		if (ImGui::TreeNodeEx("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::Checkbox("Deferred", &settings.deferred)) {
				liveSettings = settings;
			}
			if (ImGui::Checkbox("Shadows", &settings.shadows)) {
				liveSettings = settings;
			}
			auto& pp = settings.postProcess;
			auto& bloom = settings.bloom;
			if (ImGui::DragFloat("Exposure", &pp.exposure, 0.01f, 0.01f, 100, "%.2f")) {
				liveSettings = settings;
			}
			if (ImGui::Checkbox("Inline Exposure", &pp.inlineExposure)) {
				liveSettings = settings;
			}
			if (ImGui::Checkbox("Bloom Effect", &pp.bloom)) {
				liveSettings = settings;
			}
			if (ImGui::SliderScalar("Mip Levels", ImGuiDataType_::ImGuiDataType_U32, &bloom.mipLevels, &bloom.minMipLevels, &bloom.maxMipLevels())) {
				liveSettings = settings;
			}
			if (ImGui::Checkbox("Use Blitting for downsampling instead of Compute Shader", &bloom.useBlitFromDownsampling)) {
				liveSettings = settings;
			}
			if (bloom.useBlitFromDownsampling) {
				ImGui::SameLine();
				if (ImGui::Checkbox("Linear Filtering", &bloom.blitLinear)) {
					liveSettings = settings;
				}
			}
			int kernelRange = bloom.gaussianKernelSize / 2;
			if (ImGui::SliderInt("Kernel Range (Size-1)/2", &kernelRange, bloom.minKernelSize / 2, bloom.maxKernelSize / 2)) {
				bloom.gaussianKernelSize = kernelRange * 2 + 1; // always odd
				liveSettings = settings;
			}
			if (ImGui::DragFloat("Kernel Distribution", &bloom.sigma, 0.01f, 0.000001f, 100, "%.6f", ImGuiSliderFlags_::ImGuiSliderFlags_Logarithmic)) {
				liveSettings = settings;
			}
			if (!bloom.inlineKernelData) {
				if (ImGui::Checkbox("Inline Kernel Size", &bloom.inlineKernelSize)) {
					liveSettings = settings;
				}
				ImGui::SameLine();
			}
			if (ImGui::Checkbox("Inline Kernel", &bloom.inlineKernelData)) {
				bloom.inlineKernelSize = true;
				liveSettings = settings;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Inline Threshold", &bloom.inlineThreshold)) {
				liveSettings = settings;
			}
			// if (ImGui::ColorEdit4("Threshold", &bloom.threshold.r, ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_::ImGuiColorEditFlags_HDR)) {
			// 	settings.bloom = bloom;
			// }
			if (ImGui::ColorEdit3("Threshold", &bloom.threshold.r)) {
				liveSettings = settings;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				float alpha = bloom.threshold.a; // don't reset the alpha value
				bloom.threshold = bloom.thresholdDefault;
				bloom.threshold.a = alpha;
				liveSettings = settings;
			}
			if (ImGui::DragFloat("Threshold multiplier", &bloom.threshold.a, 0.001f)) {
				liveSettings = settings;
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
