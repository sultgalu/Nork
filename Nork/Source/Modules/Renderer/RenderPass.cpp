#include "RenderPass.h"
namespace Nork::Renderer {

std::vector<uint32_t> RenderPass::LoadShader(const fs::path& srcFile, std::vector<std::array<std::string, 2>> macros)
{
	using namespace Vulkan;
	fs::path binFile = fs::path(srcFile).replace_extension(srcFile.extension().string() + "_bin");
	auto src = FileUtils::ReadAsString(srcFile.string());
	size_t srcHash = std::hash<std::string>()(src);
	if (fs::exists(binFile))
	{ // check if hash is still the same, if so return saved binary
		auto data = FileUtils::ReadBinary<uint32_t>(binFile.string());
		size_t hash = *((size_t*)&data.data()[data.size() - 2]);
		if (srcHash == hash)
		{
			data.resize(data.size() - 2);
			return data;
		}
	}

	using namespace Renderer;
	ShaderType type = ShaderType::None;
	if (srcFile.extension() == ".vert")
		type = ShaderType::Vertex;
	else if (srcFile.extension() == ".frag")
		type = ShaderType::Fragment;
	else if (srcFile.extension() == ".geom")
		type = ShaderType::Geometry;
	else if (srcFile.extension() == ".glsl")
		type = ShaderType::Mesh;
	else
		std::unreachable();

	auto data = Shaderc::Compile(src, type, macros);
	// save with hash
	data.resize(data.size() + 2);
	*((size_t*)&data.data()[data.size() - 2]) = srcHash;
	FileUtils::WriteBinary(data, binFile.string());
	// remove hash
	data.resize(data.size() - 2);
	return data;
}
void RenderPass::BeginRenderPass(vk::RenderPass renderPass, Vulkan::Framebuffer& fb, Vulkan::CommandBuffer& cmd)
{
	using namespace Vulkan;
	vk::RenderPassBeginInfo beginInfo;
	beginInfo.renderPass = renderPass;
	beginInfo.framebuffer = *fb;
	beginInfo.renderArea.offset = vk::Offset2D();
	beginInfo.renderArea.extent = vk::Extent2D(fb.Width(), fb.Height());

	std::vector<vk::ClearValue> clearValues;
	clearValues.reserve(fb.Attachments().size());
	for (auto& att : fb.Attachments())
	{
		auto format = att->Image()->Format();
		vk::ClearValue clearValue{};
		using enum vk::Format;
		if (format == eD32Sfloat || format == eD16Unorm)
			clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		// else if (format == eR8G8B8A8Srgb || format == eR8G8B8A8Unorm || format == Format::rgba16f || format == Format::rgba32f)
		// 	clearValue.color = vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.0f, 1.0f});
		// else
		// 	std::unreachable();
		else
			clearValue.color = vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.0f, 1.0f});
		clearValues.push_back(clearValue);
	}

	beginInfo.clearValueCount = clearValues.size();
	beginInfo.pClearValues = clearValues.data();
	cmd.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
	fb.usedByCommandBuffer = true;
}
}