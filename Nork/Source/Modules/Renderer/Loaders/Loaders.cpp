#include "Loaders.h"
#include "../Utils.h"

namespace Nork::Renderer::Loaders
{
	Utils::Texture::Format GetFormat(int channels)
	{
		using enum Utils::Texture::Format;
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
    TextureData LoadImage(std::string_view path)
    {
        TextureData data{};
        int width = 0, height = 0, channels = 0;
        data.data = Utils::Texture::LoadImageData(path, width, height, channels);
        data.height = (int16_t)height;
        data.width = (int16_t)width;
        data.channels = (int8_t)channels;
		data.format = GetFormat(channels);
        return data;
    }
	std::array<TextureData, 6> LoadCubemapImages(std::string dirPath, std::string extension)
	{
		static std::string suffixes[6]{
			"right", "left","top","bottom","front","back",
		};

		if (dirPath.at(dirPath.size() - 1) != '/')
			dirPath.append("/");

		std::array<TextureData, 6> datas;
		int width, height, nrChannels;

		for (int i = 0; i < 6; i++)
		{
			auto data = Utils::Texture::LoadImageData((dirPath + suffixes[i] + extension).c_str(), width, height, nrChannels);
			
			if (data.size() > 0)
			{
				Logger::Debug("Loading Cubemap face #", i);
				datas[i].data = data;
				datas[i].width = (uint16_t)width;
				datas[i].height = (uint16_t)height;
				datas[i].channels = (uint8_t)nrChannels;
				datas[i].format = GetFormat(nrChannels);
			}
			else
			{
				std::cout << "ERR::FAILED TO LOAD TEXTURE" << std::endl;
				std::abort();
			}
		}
		return datas;
	}
    ShaderData LoadShader(std::string_view path)
    {
        std::ifstream stream(path.data());
        std::stringstream buf;
        buf << stream.rdbuf();

        auto data = ShaderData{ .source = std::string(buf.str()) };
        stream.close();
        return data;
    }

	std::string relativePath;

	void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureUse texType, std::vector<std::pair<TextureUse, TextureData>>& texs)
	{
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString src;
			mat->GetTexture(type, i, &src);

			auto endPos = relativePath.find_last_of('/');
			if(endPos == std::string::npos)
				endPos = relativePath.find_last_of('\\');
			auto texPath = relativePath.substr(0, endPos + 1) + src.C_Str();

			texs.push_back(std::pair(texType, LoadImage(texPath)));
		}
	}
	MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		MeshData data;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.Position.x = mesh->mVertices[i].x; vertex.Position.y = mesh->mVertices[i].y; vertex.Position.z = mesh->mVertices[i].z;
			vertex.Normal.x = mesh->mNormals[i].x; vertex.Normal.y = mesh->mNormals[i].y; vertex.Normal.z = mesh->mNormals[i].z;
			if (mesh->mTextureCoords[0]) // can store up to 8, but not every vertex may have texCoords.
			{
				vertex.TexCoords.x = mesh->mTextureCoords[0][i].x; vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
			}
			else vertex.TexCoords = glm::vec2(0.0f);
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

			LoadMaterialTextures(material, aiTextureType_DIFFUSE, TextureUse::Diffuse, data.textures);
			LoadMaterialTextures(material, aiTextureType_HEIGHT, TextureUse::Normal, data.textures); // map_Bump
			LoadMaterialTextures(material, aiTextureType_SHININESS, TextureUse::Roughness, data.textures);
			LoadMaterialTextures(material, aiTextureType_AMBIENT, TextureUse::Reflection, data.textures); // You have to change the ".mtl" file. "refl" -> "map_Ka"
		}

		return data;
	}
	void ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshData>& meshes)
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
    std::vector<MeshData> LoadModel(std::string path)
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
