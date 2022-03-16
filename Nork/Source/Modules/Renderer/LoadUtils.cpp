#include "LoadUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb/stb_image.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace Nork::Renderer
{
	static TextureFormat GetFormat(int channels)
	{
		using enum TextureFormat;
		switch (channels)
		{
		case 4: [[unlikely]]
			return RGBA8;
		case 3: [[likely]]
			return RGB8;
		case 1: [[ads]]
			return R8;
		default:
			MetaLogger().Error("Unhandled number of channels");
		}
	}
    Image LoadUtils::LoadImage(std::string_view path)
    {
		Image img{};
        int width = 0, height = 0, channels = 0;

		unsigned char* data = stbi_load(path.data(), &img.width, &img.height, &img.channels, 0);
		size_t size = (size_t)width * height * channels;
		if (data)
		{
			img.data.assign(data, data + size);
			stbi_image_free(data);
		}
		else
		{
			Logger::Error("Failed to load texture data from ", path);
		}
		img.format = GetFormat(channels);
        return img;
    }
	std::array<Image, 6> LoadUtils::LoadCubemapImages(std::string dirPath, std::string extension)
	{
		static std::string suffixes[6]{
			"right", "left","top","bottom","front","back",
		};

		if (dirPath.at(dirPath.size() - 1) != '/')
			dirPath.append("/");

		std::array<Image, 6> datas;
		int width, height, nrChannels;

		for (int i = 0; i < 6; i++)
		{
			datas[i] = LoadImage(dirPath + suffixes[i] + extension);
		}
		return datas;
	}
    std::string LoadShader(std::string_view path)
    {
        std::ifstream stream(path.data());
        std::stringstream buf;
        buf << stream.rdbuf();

		return buf.str();
    }

	std::string relativePath;

	static void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureMapType texType, std::vector<std::pair<TextureMapType, std::string>>& texs)
	{
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString src;
			mat->GetTexture(type, i, &src);

			auto endPos = relativePath.find_last_of('/');
			if(endPos == std::string::npos)
				endPos = relativePath.find_last_of('\\');
			auto texPath = relativePath.substr(0, endPos + 1) + src.C_Str();

			texs.push_back(std::pair(texType, texPath));
		}
	}
	static MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		MeshData data;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.position.x = mesh->mVertices[i].x; vertex.position.y = mesh->mVertices[i].y; vertex.position.z = mesh->mVertices[i].z;
			vertex.normal.x = mesh->mNormals[i].x; vertex.normal.y = mesh->mNormals[i].y; vertex.normal.z = mesh->mNormals[i].z;
			if (mesh->mTextureCoords[0]) // can store up to 8, but not every vertex may have texCoords.
			{
				vertex.texCoords.x = mesh->mTextureCoords[0][i].x; vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
			}
			else vertex.texCoords = glm::vec2(0.0f);
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.tangent.x = mesh->mTangents[i].x; vertex.tangent.y = mesh->mTangents[i].y; vertex.tangent.z = mesh->mTangents[i].z;
				vertex.biTangent.x = mesh->mBitangents[i].x; vertex.biTangent.y = mesh->mBitangents[i].y; vertex.biTangent.z = mesh->mBitangents[i].z;
			}
			else
			{
				vertex.tangent = glm::vec3(0.0f); vertex.biTangent = glm::vec3(0.0f);
			}
			data.vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				data.indices.push_back(face.mIndices[j]);
			}
		}

		if (mesh->mMaterialIndex > 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			LoadMaterialTextures(material, aiTextureType_DIFFUSE, TextureMapType::Diffuse, data.textures);
			LoadMaterialTextures(material, aiTextureType_HEIGHT, TextureMapType::Normal, data.textures); // map_Bump
			LoadMaterialTextures(material, aiTextureType_SHININESS, TextureMapType::Roughness, data.textures);
			LoadMaterialTextures(material, aiTextureType_AMBIENT, TextureMapType::Reflection, data.textures); // You have to change the ".mtl" file. "refl" -> "map_Ka"
		}

		return data;
	}
	static void ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshData>& meshes)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			meshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene));
		}
		
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, meshes);
		}
	}
    std::vector<MeshData> LoadUtils::LoadModel(std::string path)
    {
		relativePath = path;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate
			| aiProcess_TransformUVCoords
			| aiProcess_GenNormals
			| aiProcess_FlipUVs
			| aiProcess_CalcTangentSpace);

		std::vector<MeshData> result;

		if (!scene || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			MetaLogger().Error(importer.GetErrorString());
            return result;
		}

		ProcessNode(scene->mRootNode, scene, result);
		return result;
    }
}
