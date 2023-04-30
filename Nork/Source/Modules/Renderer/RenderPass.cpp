#include "RenderPass.h"
namespace Nork::Renderer {

struct LoadShaderResult {
	std::string sourceCode;
	size_t hash;
	bool hashMatched = false;
};

static fs::path BinaryShaderPath(const fs::path& sourceCodePath) {
	return fs::path(sourceCodePath).replace_extension(sourceCodePath.extension().string() + "_bin");
}

static LoadShaderResult LoadShader_(const fs::path& srcPath, std::vector<std::array<std::string, 2>> macros) {
	LoadShaderResult result;

	result.sourceCode = FileUtils::ReadAsString(srcPath.string());

	std::stringstream dataToBeHashed;
	for (auto& [n, v] : macros) {
		dataToBeHashed << n << v;
	}
	dataToBeHashed << result.sourceCode;
	result.hash = std::hash<std::string>()(dataToBeHashed.str());

	fs::path binPath = BinaryShaderPath(srcPath);
	if (fs::exists(binPath) && fs::file_size(binPath) >= sizeof(size_t)) {
		auto savedHash = FileUtils::ReadBinary<size_t>(binPath, sizeof(size_t)).front();
		if (result.hash == savedHash) {
			result.hashMatched = true;
		}
	}

	return result;
}

bool RenderPass::IsShaderSourceChanged(const fs::path& srcPath, std::vector<std::array<std::string, 2>> macros)
{
	return !LoadShader_(srcPath, macros).hashMatched;
}
std::vector<uint32_t> RenderPass::LoadShader(const fs::path& srcPath, std::vector<std::array<std::string, 2>> macros)
{
	using namespace Vulkan;
	LoadShaderResult loadShaderResult = LoadShader_(srcPath, macros);

	if (loadShaderResult.hashMatched) {
		auto binPath = BinaryShaderPath(srcPath);
		return FileUtils::ReadBinary<uint32_t>(binPath, fs::file_size(binPath) - sizeof(size_t), sizeof(size_t));
	}

	using namespace Renderer;
	ShaderType type = ShaderType::None;
	if (srcPath.extension() == ".vert")
		type = ShaderType::Vertex;
	else if (srcPath.extension() == ".frag")
		type = ShaderType::Fragment;
	else if (srcPath.extension() == ".geom")
		type = ShaderType::Geometry;
	else if (srcPath.extension() == ".comp")
		type = ShaderType::Compute;
	else if (srcPath.extension() == ".glsl")
		type = ShaderType::Mesh;
	else
		std::unreachable();

	auto data = Shaderc::Compile(loadShaderResult.sourceCode, type, macros);
	std::vector<std::byte> fileData(sizeof(size_t) + data.size() * sizeof(uint32_t));
	std::memcpy(&fileData[0], &loadShaderResult.hash, sizeof(size_t));
	std::memcpy(&fileData[sizeof(size_t)], data.data(), data.size() * sizeof(uint32_t));
	FileUtils::WriteBinary(fileData, BinaryShaderPath(srcPath));

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
			clearValue.color = vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.0f, 0.0f});
		clearValues.push_back(clearValue);
	}

	beginInfo.clearValueCount = clearValues.size();
	beginInfo.pClearValues = clearValues.data();
	cmd.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
	fb.usedByCommandBuffer = true;
}
}