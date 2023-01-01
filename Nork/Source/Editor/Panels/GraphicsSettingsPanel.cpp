#include "include/GraphicsSettingsPanel.h"
#include "Modules/Renderer/MemoryAllocator.h"

namespace Nork::Editor {
	GraphicsSettingsPanel::GraphicsSettingsPanel()
	{
	}

	void GraphicsSettingsPanel::Content()
	{
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
	}
}
