#include "ResourceManager.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/GLTF/GLTFBuilder.h"
#include "Modules/Renderer/MeshFactory.h"
#include "RenderingSystem.h"
#include "GLTFReader.h"

namespace Nork {
static Renderer::GLTF::GLTF LoadGLTF(const fs::path& path)
{
	return Renderer::GLTF::GLTF::FromJson(JsonObject::ParseFormatted(FileUtils::ReadAsString(path.string())));
}
static void SaveGLTF(const Renderer::GLTF::GLTF& gltf, const fs::path& path)
{
	Logger::Info("Saving GLTF file to ", path);
	FileUtils::WriteString(gltf.ToJson().ToStringFormatted(), path.string());
}
static void ParseMaterial(Renderer::Material& material, const Renderer::GLTF::Material& mat, const Renderer::GLTF::GLTF& gltf, const fs::path& gltfFolder)
{
	auto data = material.Data();
	data->baseColorFactor = mat.pbrMetallicRoughness.baseColorFactor;
	data->roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
	data->metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
	if (mat.alphaMode == mat.MASK)
		data->alphaCutoff = mat.alphaCutoff;

	auto gltfFolderUri = AssetLoader::Instance().AbsolutePathToUri(gltfFolder);
	auto setTexture = [&](int idx, Renderer::TextureMap type) {
		auto& filename = gltf.images[gltf.textures[idx].source].uri;
		material.SetTextureMap(AssetLoader::Instance().LoadTexture(gltfFolderUri / filename), type);
	};

	if (mat.pbrMetallicRoughness.baseColorTexture.Validate()) {
		setTexture(mat.pbrMetallicRoughness.baseColorTexture.index, Renderer::TextureMap::BaseColor);
	}
	if (mat.pbrMetallicRoughness.metallicRoughnessTexture.Validate()) {
		setTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index, Renderer::TextureMap::MetallicRoughness);
	}
	if (mat.normalTexture.Validate()) {
		setTexture(mat.normalTexture.index, Renderer::TextureMap::Normal);
	}
}

AssetLoader::AssetLoader() {
	projectAssetsRoot = fs::current_path().append("Assets");
	fs::create_directories(projectAssetsRoot);
	fs::create_directory(ModelsPath());
}
AssetLoaderProxy::AssetLoaderProxy() {
	if (!fs::exists(UriToAbsolutePath(CubeUri()))) {
		ImportModel(TemplatesPath() / "cube" / "cube.gltf", "cube");
	}
}
std::shared_ptr<Components::Model> AssetLoader::LoadModel(const fs::path& uri) {
	fs::path path = UriToAbsolutePath(uri);
	Logger::Info("Loading model ", path);
	if (!fs::exists(path)) {
		throw Exception(Exception::Code::PathDoesNotExist);
	}
	auto gltf = LoadGLTF(path);
	if (gltf.meshes.empty()) {
		throw Exception(Exception::Code::EmptyModel);
	}

	auto model = std::make_shared<Components::Model>();
	auto& mesh = gltf.meshes.front(); // gltf::Mesh = Components::Model
	for (auto& prim : mesh.primitives)
	{ // gltf::Primitive = Components::Mesh
		const auto& indsBuf = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.indices].bufferView].buffer];
		const auto& vertsBufs = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.attributes.back().accessor].bufferView].buffer]; // all of this primitive's attributes should point to the same buffer (vertices) 
		// Logger::Info("Loading mesh ", path);

		Components::Mesh mesh{ .mesh = RenderingSystem::Instance().NewMesh(
			FileUtils::ReadBinary<Renderer::Data::Vertex>((path.parent_path() / vertsBufs.uri).string()),
			FileUtils::ReadBinary<uint32_t>((path.parent_path() / indsBuf.uri).string())) };
		mesh.material = RenderingSystem::Instance().NewMaterial();
		if (prim.material != -1)
			ParseMaterial(*mesh.material, gltf.materials[prim.material], gltf, path.parent_path());
		model->meshes.push_back(mesh);
	}

	return model;
}
std::shared_ptr<Components::Model> AssetLoader::ImportModel(const fs::path& from, const fs::path& uri)
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
void AssetLoader::SaveModel(const std::shared_ptr<Components::Model>& model, const fs::path& uri)
{
	fs::path path = UriToAbsolutePath(uri);
	Logger::Info("Saving Model to ", path);

	if (!fs::exists(path.parent_path())) {
		fs::create_directories(path.parent_path()); // make sure the folder for the model's files exists
	}

	auto builder = Renderer::GLTFBuilder();
	std::vector<std::pair<Renderer::TextureMap, std::string>> imageUris;
	int matIdx = 0;
	builder.NewScene(true).NewNode(path.stem().string());
	for (auto& mesh : model->meshes)
	{
		// MeshResources::Instance().Save(mesh.mesh);
		auto tryAddTex = [&](Renderer::TextureMap type)
		{
			if (!mesh.material->HasDefault(type)) { // don't need to add if it equals the (gltf) default one 
				// save just the filename, not the whole uri, so it conforms to the standard
				imageUris.push_back({ type, AssetLoader::Instance().Uri(mesh.material->GetTextureMap(type)).filename().string()});
			}
		};
		using enum Renderer::TextureMap;
		tryAddTex(BaseColor);
		tryAddTex(MetallicRoughness);
		tryAddTex(Normal);
		builder
			.AddMesh(*mesh.mesh, path.parent_path().string(), matIdx++)
			.AddMaterial(*mesh.material, imageUris);
	}

	SaveGLTF(builder.Get(), path);
}
std::shared_ptr<Renderer::Texture> AssetLoader::LoadTexture(const fs::path& uri) {
	fs::path path = UriToAbsolutePath(uri);
	Logger::Info("Loading texture ", path);

	auto img = RenderingSystem::Instance().LoadImage(path.string());
	return Renderer::Resources::Instance().Textures().AddTexture(img);
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

AssetLoaderProxy& AssetLoader::Instance() {
	static AssetLoaderProxy instance;
	return instance;
}

std::shared_ptr<Renderer::Texture> AssetLoaderProxy::LoadTexture(const fs::path& uri) {
	if (!textures.contains(uri)) {
		textures[uri] = AssetLoader::LoadTexture(uri);
	}
	return textures[uri];
}
std::shared_ptr<Components::Model> AssetLoaderProxy::LoadModel(const fs::path& uri) {
	if (!models.contains(uri)) {
		models[uri] = AssetLoader::LoadModel(uri);
	}
	return models[uri];
}
std::shared_ptr<Components::Model> AssetLoaderProxy::ImportModel(const fs::path& from, std::string name) {
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
void AssetLoaderProxy::SaveModel(const std::shared_ptr<Components::Model>& model) {
	fs::path uri = Uri(model);
	AssetLoader::SaveModel(model, uri);
}
void AssetLoaderProxy::ReloadModel(const std::shared_ptr<Components::Model>& model) {
	auto uri = Uri(model);
	auto freshModel = AssetLoader::LoadModel(uri);
	*model = *freshModel;
}
fs::path AssetLoaderProxy::Uri(const std::shared_ptr<Components::Model>& of) {
	return Uri(models, of);
}
fs::path AssetLoaderProxy::Uri(const std::shared_ptr<Renderer::Texture>& of) {
	return Uri(textures, of);
}
std::vector<fs::path> AssetLoaderProxy::ListLoadedModels() {
	std::vector<fs::path> loaded;
	for (auto& [uri, _] : models) {
		loaded.push_back(uri);
	}
	return loaded;
}
void AssetLoaderProxy::ClearCache() {
	models.clear();
	textures.clear();
}
}