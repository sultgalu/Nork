#include "Core/AssetLoader.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/GLTF/GLTFBuilder.h"
#include "Modules/Renderer/MeshFactory.h"
#include "RenderingSystem.h"
#include "GLTFReader.h"
#include "AssetLoader.h"

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
	if (mat.alphaMode == mat.MASK) {
		data->alphaCutoff = mat.alphaCutoff;
	}
	else if (mat.alphaMode == mat.BLEND) {
		material.shadingMode = Renderer::ShadingMode::Blend;
	}

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
	if (mat.occlusionTexture.Validate()) {
		setTexture(mat.occlusionTexture.index, Renderer::TextureMap::Occlusion);
	}
}

AssetLoader::AssetLoader() {
}
AssetLoaderProxy::AssetLoaderProxy() {
	SetProjectRoot(fs::current_path().append("Assets"));
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

	// TODO: renderer::meshes (primitives) could also be shared, but it is not possible with gltf
	std::vector<std::shared_ptr<Components::Model::Mesh>> meshes; 
	std::vector<std::shared_ptr<Renderer::Material>> materials;

	for (auto& glMat : gltf.materials) {
		auto material = RenderingSystem::Instance().NewMaterial();
		materials.push_back(material);
		ParseMaterial(*material, glMat, gltf, path.parent_path());
	}

	for (auto& glMesh : gltf.meshes) {
		auto mesh = std::make_shared<Components::Model::Mesh>();
		meshes.push_back(mesh);
		for (auto& prim : glMesh.primitives)
		{
			const auto& indsBuf = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.indices].bufferView].buffer];
			const auto& vertsBufs = gltf.buffers[gltf.bufferViews[gltf.accessors[prim.attributes.back().accessor].bufferView].buffer]; // all of this primitive's attributes should point to the same buffer (vertices) 
			Components::Model::SubMesh subMesh{ .mesh = RenderingSystem::Instance().NewMesh(
				FileUtils::ReadBinary<Renderer::Data::Vertex>((path.parent_path() / vertsBufs.uri).string()),
				FileUtils::ReadBinary<uint32_t>((path.parent_path() / indsBuf.uri).string())) };

			subMesh.material = prim.material != -1 ? materials[prim.material] : RenderingSystem::Instance().NewMaterial();
			mesh->subMeshes.push_back(subMesh);
		}
	}

	auto model = std::make_shared<Components::Model>();
	for (auto& glNode : gltf.nodes) {
		if (glNode.mesh == -1) {
			std::unreachable(); // this could happen in the future, but not now
		}
		Components::Model::MeshNode meshNode;
		meshNode.mesh = meshes[glNode.mesh];
		if (glNode.HasTransform()) {
			meshNode.localTransform = glNode.Transform();
		}
		model->nodes.push_back(meshNode);
	}
	auto& mesh = gltf.meshes.front(); // gltf::Mesh = Components::Model
	

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
	int matIdx = 0, meshIdx = 0;
	std::unordered_map<std::shared_ptr<Components::Model::Mesh>, int> meshes;
	std::unordered_map<std::shared_ptr<Renderer::Material>, int> materials;
	builder.AddScene(true); //AddNode(path.stem().string());
	for (auto& node : model->nodes)
	{
		if (!meshes.contains(node.mesh)) { // check if we already added this mesh
			meshes[node.mesh] = meshIdx++;
			builder.AddMesh();

			for (auto& primitive : node.mesh->subMeshes) {
				if (!materials.contains(primitive.material)) { // check if we already added this material
					materials[primitive.material] = matIdx++;
					auto tryAddTex = [&](Renderer::TextureMap type) {
						if (!primitive.material->HasDefault(type)) { // don't need to add if it equals the (gltf) default one 
							// save just the filename, not the whole uri, so it conforms to the standard
							imageUris.push_back({ type, AssetLoader::Instance().Uri(primitive.material->GetTextureMap(type)).filename().string() });
						}
					};
					using enum Renderer::TextureMap;
					tryAddTex(BaseColor);
					tryAddTex(MetallicRoughness);
					tryAddTex(Normal);
					tryAddTex(Occlusion);
					builder.AddMaterial(*primitive.material, imageUris);
				}
				builder.AddPrimitive(*primitive.mesh, path.parent_path().string(), materials[primitive.material]);
			}
		}
		builder.AddNode(meshes[node.mesh]);
		if (node.localTransform) {
			builder.AddTransform(*node.localTransform);
		}
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
void AssetLoaderProxy::DeleteFromCache(const std::shared_ptr<Components::Model>& model)
{
	models.erase(Uri(model));
}
void AssetLoaderProxy::ClearCache() {
	models.clear();
	textures.clear();
}
}