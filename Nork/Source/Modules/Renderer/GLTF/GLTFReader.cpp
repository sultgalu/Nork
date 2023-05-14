#include "GLTFReader.h"
#include "../Resources.h"
#include "../AssetLoader.h"

namespace Nork::Renderer {
GLTFReader::GLTFReader(const fs::path& path)
{
    srcFolder = path.parent_path();
    dstFolder = path.stem();
    gltf = GLTF::GLTF::FromJson(JsonObject::ParseFormatted(FileUtils::ReadAsString(path.string())));
    defaultMaterial = Resources::Instance().CreateMaterial();
}
std::shared_ptr<Model> GLTFReader::Read()
{
    model = std::make_shared<Model>();

    for (auto& buf : gltf.buffers)
        buffers.push_back(FileUtils::ReadBinary<char>(AbsolutePath(buf.uri).string()));
    for (auto& img : gltf.images)
        images.push_back(AssetLoader::Instance().LoadTexture(img.uri)); // this is copied over by AssetLoader to the right folder
    for (auto& mat : gltf.materials)
        materials.push_back(CreateMaterial(mat));
    for (size_t i = 0; i < gltf.meshes.size(); i++) {
        meshDatas.push_back({});
        for (size_t j = 0; j < gltf.meshes[i].primitives.size(); j++) {
            auto mesh = CreateRendererMesh(j, i);
            meshDatas.back().push_back(mesh);
        }
    }
    for (auto nodeIdx : gltf.scenes.back().nodes)
        AddNodeRecursive(nodeIdx);

    return model;
}
void GLTFReader::AddNodeRecursive(int nodeIdx, const glm::mat4& parentTransform)
{
    const GLTF::Node& glNode = gltf.nodes[nodeIdx];
    glm::mat4 transform = parentTransform;
    if (glNode.HasTransform()) // "The global transformation matrix of a node is the product of the global transformation matrix of its parent node and its own local transformation matrix"
        transform = parentTransform * glNode.Transform();

    for (auto i : glNode.children) {
        AddNodeRecursive(i, transform);
    }

    if (glNode.mesh != -1) {
        auto& glMesh = gltf.meshes[glNode.mesh];
        model->nodes.push_back(MeshNode { .mesh = std::make_shared<Mesh>() });
        model->nodes.back().children = glNode.children;
        for (size_t i = 0; i < glMesh.primitives.size(); i++) {
            auto& mesh = meshDatas[glNode.mesh][i];
            if (mesh) {
                auto matIdx = glMesh.primitives[i].material;
                auto primitive = Primitive {
                    .meshData = mesh,
                    .material = matIdx == -1 ? defaultMaterial : materials[matIdx]
                };
                auto& mat = gltf.materials[matIdx];
                if (mat.alphaMode == mat.BLEND) {
                    primitive.shadingMode = ShadingMode::Blend;
                }
                model->nodes.back().mesh->primitives.push_back(primitive);
            }
        }
        if (transform != glm::identity<glm::mat4>()) {
            model->nodes.back().localTransform = transform;
        }
    }
}
std::shared_ptr<Material> GLTFReader::CreateMaterial(const GLTF::Material& mat)
{
    auto material = Resources::Instance().CreateMaterial();
    auto data = material->Data();
    data->baseColorFactor = mat.pbrMetallicRoughness.baseColorFactor;
    data->roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
    data->metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
    data->emissiveFactor = mat.emissiveFactor;
    if (mat.alphaMode == mat.MASK)
        data->alphaCutoff = mat.alphaCutoff;
    material->blending = mat.alphaMode == mat.BLEND;
    // material->specularExponent = mat.pbrMetallicRoughness.extras.Get<float>("specularExponent"); has official extension
    if (mat.pbrMetallicRoughness.baseColorTexture.Validate())
        material->SetTextureMap(images[gltf.textures[mat.pbrMetallicRoughness.baseColorTexture.index].source], TextureMap::BaseColor);
    if (mat.pbrMetallicRoughness.metallicRoughnessTexture.Validate())
        material->SetTextureMap(images[gltf.textures[mat.pbrMetallicRoughness.metallicRoughnessTexture.index].source], TextureMap::MetallicRoughness);
    if (mat.normalTexture.Validate())
        material->SetTextureMap(images[gltf.textures[mat.normalTexture.index].source], TextureMap::Normal);
    if (mat.occlusionTexture.Validate())
        material->SetTextureMap(images[gltf.textures[mat.occlusionTexture.index].source], TextureMap::Occlusion);
    if (mat.emissiveTexture.Validate())
        material->SetTextureMap(images[gltf.textures[mat.emissiveTexture.index].source], TextureMap::Emissive);
    return material;
}
std::shared_ptr<MeshData> GLTFReader::CreateRendererMesh(int idx, int meshIdx)
{
    using namespace Renderer;

    auto& prim = gltf.meshes[meshIdx].primitives[idx];
    if (prim.mode != GL_TRIANGLES) {
        Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", its mode is not GL_TRIANGLES, but ", prim.mode);
        return nullptr;
    }
    int posAccIdx = prim.Accessor(GLTF::Attribute::position);
    int normAccIdx = prim.Accessor(GLTF::Attribute::normal);
    int texAccIdx = prim.Accessor(GLTF::Attribute::texcoord0);
    bool hasNorm = normAccIdx != -1;
    bool hasTex = texAccIdx != -1;
    if (posAccIdx == -1 || !hasNorm) {
        Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (missing position or normal accessor)");
        return nullptr;
    }
    auto posAcc = gltf.accessors[posAccIdx];
    auto normAcc = hasNorm ? gltf.accessors[normAccIdx] : GLTF::Accessor{};
    auto texAcc = hasTex ? gltf.accessors[texAccIdx] : GLTF::Accessor{};
    auto idxAcc = gltf.accessors[prim.indices];
    if (idxAcc.type != GLTF::Accessor::SCALAR || posAcc.type != GLTF::Accessor::VEC3 || (hasNorm && normAcc.type != GLTF::Accessor::VEC3) || (hasTex && texAcc.type != GLTF::Accessor::VEC2)) {
        Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (bad type)");
        return nullptr;
    }
    if (posAcc.componentType != GL_FLOAT || (hasNorm && normAcc.componentType != GL_FLOAT) || (hasTex && texAcc.componentType != GL_FLOAT)) {
        Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (bad componentType)");
        return nullptr;
    }
    if (posAcc.count != normAcc.count || (hasNorm && normAcc.count != posAcc.count) || (hasTex && texAcc.count != posAcc.count)) {
        Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (counts not equal)");
        return nullptr;
    }
    auto posStride = glm::max<int>(gltf.bufferViews[posAcc.bufferView].byteStride, sizeof(glm::vec3)) / sizeof(float);
    auto normStride = hasNorm ? glm::max<int>(gltf.bufferViews[normAcc.bufferView].byteStride, sizeof(glm::vec3)) / sizeof(float) : 0;
    auto texStride = hasTex ? glm::max<int>(gltf.bufferViews[texAcc.bufferView].byteStride, sizeof(glm::vec2)) / sizeof(float) : 0;
    std::span<float> posData = BufferView<glm::vec3>(posAcc);
    std::span<float> normData = hasNorm ? BufferView<glm::vec3>(normAcc) : std::span<float>();
    std::span<float> texData = hasTex ? BufferView<glm::vec2>(texAcc) : std::span<float>();

    std::vector<Data::Vertex> vertices;
    vertices.reserve(posAcc.count);
    for (size_t i = 0; i < posAcc.count; i++) {
        vertices.push_back(Data::Vertex {
            .position = *((glm::vec3*)&posData[i * posStride]),
            .normal = hasNorm ? *((glm::vec3*)&normData[i * normStride]) : glm::vec3(0),
            .texCoords = hasTex ? *((glm::vec2*)&texData[i * texStride]) : glm::vec2(0.5),
        });
    }
    std::vector<uint32_t> indices;
    if (prim.indices == -1) {
        Logger::Warning("Generating Index buffer as an arithmetic series, because primitive ", idx, " of mesh ", meshIdx, ", does not provide one");
        indices.resize(vertices.size());
        uint32_t n = 0;
        std::ranges::generate(indices, [&n]() mutable { return n++; });
    } else if (idxAcc.componentType == GL_UNSIGNED_INT)
        indices = GetIndices<uint32_t>(prim);
    else if (idxAcc.componentType == GL_UNSIGNED_SHORT)
        indices = GetIndices<uint16_t>(prim);
    else if (idxAcc.componentType == GL_UNSIGNED_BYTE)
        indices = GetIndices<uint8_t>(prim);
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (hasTex) {
        SetTangent(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
    }
        else { // find any perpendicular vector and use that as tangent 
            auto& norm = vertices[indices[i]].normal;
            glm::vec3 other(norm.z, norm.x, norm.y);
            auto tangent = glm::cross(norm, other);
            vertices[indices[i]].tangent = tangent;
            vertices[indices[i + 1]].tangent = tangent;
            vertices[indices[i + 2]].tangent = tangent;
        }
    }
    return Resources::Instance().CreateMesh(vertices, indices);
}
template <class T>
std::vector<uint32_t> GLTFReader::GetIndices(const GLTF::Primitive& prim)
{
    auto idxAcc = gltf.accessors[prim.indices];
    auto idxStride = glm::max<int>(gltf.bufferViews[idxAcc.bufferView].byteStride / sizeof(T), 1);
    ;
    std::span<T> idxData = BufferView<T, T>(idxAcc);
    std::vector<uint32_t> indices(idxAcc.count);
    for (size_t i = 0; i < idxAcc.count; i++) {
        indices[i] = idxData[i * idxStride];
    }
    return indices;
}
void GLTFReader::SetTangent(Data::Vertex& v1, Data::Vertex& v2, Data::Vertex& v3)
{
    glm::vec3 edge1 = v2.position - v1.position;
    glm::vec3 edge2 = v3.position - v1.position;
    glm::vec2 deltaUV1 = v2.texCoords - v1.texCoords;
    glm::vec2 deltaUV2 = v3.texCoords - v1.texCoords;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    v1.tangent = tangent;
    v2.tangent = tangent;
    v3.tangent = tangent;
}
template <class S, class T>
std::span<T> GLTFReader::BufferView(const GLTF::Accessor& accessor)
{
    auto bufferView = gltf.bufferViews[accessor.bufferView];
    auto start = buffers[bufferView.buffer].data() + bufferView.byteOffset + accessor.byteOffset;
    auto size = accessor.count * glm::max<int>(bufferView.byteStride, sizeof(S)); // if bytestride is not 0, it should not be less that sizeof(T)
    return std::span<T>((T*)start, size / sizeof(T));
}
}