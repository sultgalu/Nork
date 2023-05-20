#include "AssetLoader.h"
#include "LoadUtils.h"
#include "GLTF/GLTFBuilder.h"
#include "MeshFactory.h"
#include "GLTF/GLTFReader.h"
#include "Resources.h"

namespace Nork::Renderer {
static GLTF::GLTF LoadGLTF(const fs::path& path)
{
	return GLTF::GLTF::FromJson(JsonObject::ParseFormatted(FileUtils::ReadAsString(path.string())));
}
static void SaveGLTF(const GLTF::GLTF& gltf, const fs::path& path)
{
	Logger::Info("Saving GLTF file to ", path);
	FileUtils::WriteString(gltf.ToJson().ToStringFormatted(), path.string());
}
static void ParseMaterial(Material& material, const GLTF::Material& mat, const GLTF::GLTF& gltf, const fs::path& gltfFolder)
{
	auto data = material.Data();
	data->baseColorFactor = mat.pbrMetallicRoughness.baseColorFactor;
	data->roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
	data->metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
	data->emissiveFactor = mat.emissiveFactor;
	if (mat.alphaMode == mat.MASK) {
		data->alphaCutoff = mat.alphaCutoff;
	}
	material.blending = mat.alphaMode == mat.BLEND;

	auto gltfFolderUri = AssetLoader::Instance().AbsolutePathToUri(gltfFolder);
	auto setTexture = [&](int idx, TextureMap type) {
		auto& filename = gltf.images[gltf.textures[idx].source].uri;
		material.SetTextureMap(AssetLoader::Instance().LoadTexture(gltfFolderUri / filename, type == TextureMap::BaseColor), type);
	};

	if (mat.pbrMetallicRoughness.baseColorTexture.Validate()) {
		setTexture(mat.pbrMetallicRoughness.baseColorTexture.index, TextureMap::BaseColor);
	}
	if (mat.pbrMetallicRoughness.metallicRoughnessTexture.Validate()) {
		setTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index, TextureMap::MetallicRoughness);
	}
	if (mat.normalTexture.Validate()) {
		setTexture(mat.normalTexture.index, TextureMap::Normal);
	}
	if (mat.occlusionTexture.Validate()) {
		setTexture(mat.occlusionTexture.index, TextureMap::Occlusion);
	}
	if (mat.emissiveTexture.Validate()) {
		setTexture(mat.emissiveTexture.index, TextureMap::Emissive);
	}
}
static std::shared_ptr<Image> LoadImage(const fs::path& path, bool sRgbSpace)
{
	std::string binExt = path.extension() == ".bin" ? ".bin_" : ".bin";
	auto binPath = path;
	binPath.replace_extension(binExt);

	std::vector<std::byte> binImageData;
	std::byte* binImageDataPtr;
	uint32_t width, height, binImageDataSize;
	if (!fs::exists(binPath)) {
		auto imageData = LoadUtils::LoadImage(path.string(), true);
		binImageData.resize(sizeof(uint32_t) * 2 + imageData.data.size());
		*(uint32_t*)&binImageData[0] = imageData.width;
		*(uint32_t*)&binImageData[sizeof(uint32_t)] = imageData.height;
		std::memcpy(binImageData.data() + sizeof(uint32_t) * 2, imageData.data.data(), imageData.data.size());
		FileUtils::WriteBinary(binImageData, binPath);
	}
	else {
		binImageData = FileUtils::ReadBinary<std::byte>(binPath);
	}
	width = *(uint32_t*)&binImageData[0];
	height = *(uint32_t*)&binImageData[sizeof(uint32_t)];
	binImageDataPtr = &binImageData[2 * sizeof(uint32_t)];
	binImageDataSize = binImageData.size() - 2 * sizeof(uint32_t);

	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	// format: is it linear or sRGB? usually the latter but should be checked
	auto texImg = std::make_shared<Image>(width, height, Vulkan::Format::rgba8Unorm,
		vk::ImageUsageFlagBits::eTransferSrc // blit (mipmap)
		| vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderSampledRead, mipLevels);

	texImg->Write(binImageDataPtr, binImageDataSize, vk::ImageLayout::eShaderReadOnlyOptimal);
	return texImg;
}

AssetLoader::AssetLoader() {
}
AssetLoaderProxy::AssetLoaderProxy() {
	SetProjectRoot(fs::current_path().append("Assets"));
}
std::shared_ptr<Model> AssetLoader::LoadModel(const fs::path& uri) {
	fs::path path = UriToAbsolutePath(uri);
	Logger::Info("Loading model ", path);
	if (!fs::exists(path)) {
		throw Exception(Exception::Code::PathDoesNotExist);
	}
	auto gltf = LoadGLTF(path);
	if (gltf.meshes.empty()) {
		throw Exception(Exception::Code::EmptyModel);
	}

	// TODO: MeshDataes (primitives) could also be shared, but it is not possible with gltf
	std::vector<std::shared_ptr<Mesh>> meshes; 
	std::vector<std::shared_ptr<Material>> materials;

	for (auto& glMat : gltf.materials) {
		auto material = Resources::Instance().CreateMaterial();
		materials.push_back(material);
		ParseMaterial(*material, glMat, gltf, path.parent_path());
	}

	for (auto& glMesh : gltf.meshes) {
		auto mesh = std::make_shared<Mesh>();
		meshes.push_back(mesh);
		for (auto& prim : glMesh.primitives)
		{
			const auto& indsBuf = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.indices].bufferView].buffer];
			const auto& vertsBufs = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.attributes.back().accessor].bufferView].buffer]; // all of this primitive's attributes should point to the same buffer (vertices) 
			Primitive primitive{ .meshData = Resources::Instance().CreateMesh(
				FileUtils::ReadBinary<Data::Vertex>((path.parent_path() / vertsBufs.uri).string()),
				FileUtils::ReadBinary<uint32_t>((path.parent_path() / indsBuf.uri).string())) };

			primitive.material = prim.material != -1 ? materials[prim.material] : Resources::Instance().CreateMaterial();
			prim.extras.GetIfContains("shadingMode", primitive.shadingMode);
			mesh->primitives.push_back(primitive);
		}
	}

	auto model = std::make_shared<Model>();
	for (auto& glNode : gltf.nodes) {
		if (glNode.mesh == -1) {
			std::unreachable(); // this could happen in the future, but not now
		}
		MeshNode meshNode;
		meshNode.mesh = meshes[glNode.mesh];
		meshNode.children = glNode.children;
		if (glNode.HasTransform()) {
			meshNode.localTransform = glNode.Transform();
		}
		model->nodes.push_back(meshNode);
	}
	auto& mesh = gltf.meshes.front(); // gltf::Mesh = Model
	
	return model;
}
std::shared_ptr<Model> AssetLoader::ImportModel(const fs::path& from, const fs::path& uri)
{
	if (from.extension() != ".gltf") {
		throw Exception(Exception::Code::NoImporterForAsset);
	}
	fs::path importPath = UriToAbsolutePath(uri);
	fs::create_directories(importPath.parent_path()); // make sure every directory in path exists
	Logger::Info("Importing model from ", from, "to ", importPath);

	if (fs::exists(importPath)) {
		Logger::Warning("Overwriting existing model ", importPath);
		// Logger::Error("Cannot import model, ", importPath, " already exists");
		// throw FileAlreadyExistsException(importPath);
	}

	auto gltfReader = GLTFReader(from);
	fs::path importFolder = importPath.parent_path();
	for (auto& img : gltfReader.gltf.images) { // copy all textures next to the imported gltf file
		auto src = gltfReader.AbsolutePath(img.uri);
		auto dst = importFolder / src.filename();
		fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
		img.uri = AbsolutePathToUri(dst).string();
	}
	// we don't copy the buffers, we need to save them in our format for faster loading next time
	auto model = gltfReader.Read();
	// here we should save the mesh data
	return model;
}
void AssetLoader::SaveModel(const std::shared_ptr<Model>& model, const fs::path& uri)
{
	fs::path path = UriToAbsolutePath(uri);
	Logger::Info("Saving Model to ", path);

	if (!fs::exists(path.parent_path())) {
		fs::create_directories(path.parent_path()); // make sure the folder for the model's files exists
	}

	auto builder = GLTFBuilder([&](auto& tex) {
		return AssetLoader::Instance().Uri(tex).filename().string();
	});
	builder.AddScene(true); //AddNode(path.stem().string());
	for (auto& node : model->nodes)
	{
		builder.AddNode(node, path.parent_path().string());
	}

	SaveGLTF(builder.Get(), path);
}
std::shared_ptr<Texture> AssetLoader::LoadTexture(const fs::path& uri, bool sRgbSpace) {
	fs::path path = UriToAbsolutePath(uri);
	Logger::Info("Loading texture ", path);

	auto img = LoadImage(path.string(), sRgbSpace);
	return Resources::Instance().Textures().AddTexture(img);
}
std::vector<fs::path> AssetLoader::ListTemplates() {
	std::vector<fs::path> templates;
	for (const auto& entry : fs::directory_iterator(TemplatesPath())) {
		if (entry.is_directory()) { // each template should be in a directory
			for (const auto& subEntry : fs::directory_iterator(entry.path())) {
				if (!subEntry.is_directory() && subEntry.path().extension() == ".gltf") {
					templates.push_back(subEntry.path());
				}
			}
		}
	}
	return templates;
}
fs::path AssetLoader::UriToAbsolutePath(const fs::path& uri) {
	if (uri.is_absolute()) {
		throw std::logic_error("Uri was absolute");
	}
	return projectAssetsRoot / uri;
}
fs::path AssetLoader::AbsolutePathToUri(const fs::path& abs) {
	if (!abs.is_absolute()) {
		throw std::logic_error("path was not absolute");
	}
	auto uri = fs::relative(abs, projectAssetsRoot);
	if (*uri.begin() == "" || *uri.begin() == "..") {
		throw std::logic_error("no uri for path, path is outside of project root");
	}
	return uri;
}
fs::path AssetLoader::ModelsPath() {
	return projectAssetsRoot / "models";
}
fs::path AssetLoader::TemplatesPath() {
	return fs::current_path() / "Assets" / "templates";
}
fs::path AssetLoader::CubeUri() {
	return AbsolutePathToUri(ModelsPath() / "cube" / "cube.gltf");
}

void AssetLoader::SetProjectRoot(const fs::path& newRoot)
{
	projectAssetsRoot = newRoot;
	fs::create_directory(ModelsPath());

}
void AssetLoaderProxy::SetProjectRoot(const fs::path& newRoot)
{
	AssetLoader::SetProjectRoot(newRoot);
	ClearCache();
	if (!fs::exists(UriToAbsolutePath(CubeUri()))) {
		ImportModel(TemplatesPath() / "cube" / "cube.gltf", "cube");
	}
}

AssetLoaderProxy& AssetLoader::Instance() {
	static AssetLoaderProxy instance;
	return instance;
}

std::shared_ptr<Texture> AssetLoaderProxy::LoadTexture(const fs::path& uri, bool sRgbSpace) {
	if (!textures.contains(uri)) {
		textures[uri] = AssetLoader::LoadTexture(uri, sRgbSpace);
	}
	return textures[uri];
}
std::shared_ptr<Model> AssetLoaderProxy::LoadModel(const fs::path& uri) {
	if (!models.contains(uri)) {
		models[uri] = AssetLoader::LoadModel(uri);
	}
	return models[uri];
}
std::shared_ptr<Model> AssetLoaderProxy::ImportModel(const fs::path& from, std::string name) {
	if (name == "") {
		name = from.stem().string();
	}
	fs::path importPath = ModelsPath() / name / from.filename();
	auto uri = AbsolutePathToUri(importPath);

	if (models.contains(uri)) {
		throw Exception(Exception::Code::UriAlreadyExists);
	}
	models[uri] = AssetLoader::ImportModel(from, uri);
	return models[uri];
}
void AssetLoaderProxy::SaveModel(const std::shared_ptr<Model>& model) {
	fs::path uri = Uri(model);
	AssetLoader::SaveModel(model, uri);
}
void AssetLoaderProxy::ReloadModel(const std::shared_ptr<Model>& model) {
	auto uri = Uri(model);
	auto freshModel = AssetLoader::LoadModel(uri);
	*model = *freshModel;
}
fs::path AssetLoaderProxy::Uri(const std::shared_ptr<Model>& of) {
	return Uri(models, of);
}
fs::path AssetLoaderProxy::Uri(const std::shared_ptr<Texture>& of) {
	return Uri(textures, of);
}
std::vector<fs::path> AssetLoaderProxy::ListLoadedModels() {
	std::vector<fs::path> loaded;
	for (auto& [uri, _] : models) {
		loaded.push_back(uri);
	}
	return loaded;
}
void AssetLoaderProxy::DeleteFromCache(const std::shared_ptr<Model>& model)
{
	models.erase(Uri(model));
}
void AssetLoaderProxy::ClearCache() {
	models.clear();
	textures.clear();
}
}