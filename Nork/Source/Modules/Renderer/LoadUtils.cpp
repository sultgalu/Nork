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

#define FBXSDK_SHARED
#include <fbxsdk.h>

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
        int width = 0, height = 0, channels = 0;

		unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);
		size_t size = (size_t)width * height * channels;
		if (data)
		{
			//auto d = std::vector<char>(data, data + size);
			auto image = Image{
				.width = (uint32_t)width,
				.height = (uint32_t)height,
				.channels = (uint32_t)channels,
				.format = GetFormat(channels),
				.data = std::vector<char>()
			};
			image.data.assign(data, data + size);
			std::memcpy(image.data.data(), data, size);
			stbi_image_free(data);
			return image;
		}
		else
		{
			Logger::Error("Failed to load texture data from ", path);
			return Image{
				.width = 1, .height = 1, .channels = 1, .format = TextureFormat::R8, .data = std::vector<char>(1)
			};
		}
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

	std::vector<MeshData> LoadUtils::LoadModel(std::string path)
	{
		std::vector<MeshData> result;
		FbxManager* lSdkManager = FbxManager::Create();

		FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
		bool lImportStatus = lImporter->Initialize(path.c_str(), -1, lSdkManager->GetIOSettings());

		if (!lImportStatus)
		{
			std::abort();
		}

		FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
		lImporter->Import(lScene);
		FbxGeometryConverter(lSdkManager).Triangulate(lScene, true);

		for (size_t i = 0; i < lScene->GetTextureCount(); i++)
		{
			auto obj = lScene->GetTexture(i);
			std::cout << obj->GetName() << ", " << obj->GetTextureUse() << "\n";
		}
		std::cout << "\n";
		for (size_t i = 0; i < lScene->GetNodeCount(); i++)
		{
			auto obj = lScene->GetNode(i);
			auto attr = obj->GetNodeAttribute();
			if (attr && attr->GetAttributeType() == fbxsdk::FbxNodeAttribute::eMesh)
			{
				result.push_back(MeshData());
				uint32_t indCounter = 0;

				auto mesh = obj->GetMesh();
				std::cout << obj->GetName() << "," << mesh->GetControlPointsCount() << "\n";
				auto loadTextures = [&](const char* type, TextureMapType mapType)
				{
					for (int j = 0; j < mesh->GetNode()->GetSrcObjectCount<fbxsdk::FbxSurfaceMaterial>(); j++)
					{
						fbxsdk::FbxSurfaceMaterial* material = mesh->GetNode()->GetSrcObject<fbxsdk::FbxSurfaceMaterial>(j);

						if (material)
						{
							auto prop = material->FindProperty(type);
							if (prop.IsValid())
							{
								auto texCount = prop.GetSrcObjectCount<fbxsdk::FbxTexture>();
								for (size_t k = 0; k < texCount; k++)
								{
									fbxsdk::FbxTexture* tex = prop.GetSrcObject<fbxsdk::FbxTexture>(k);
									if (tex)
									{
										result.back().textures.push_back({ mapType, ((FbxFileTexture*)tex)->GetFileName() });
										return;
									}
								}
							}
						}

					}
				};
				loadTextures(FbxSurfaceMaterial::sDiffuse, TextureMapType::Diffuse);
				loadTextures(FbxSurfaceMaterial::sNormalMap, TextureMapType::Normal);
				loadTextures(FbxSurfaceMaterial::sShininess, TextureMapType::Roughness);
				loadTextures(FbxSurfaceMaterial::sReflection, TextureMapType::Reflection);
				if (!mesh->GenerateTangentsDataForAllUVSets(true))
				{
					std::abort();
				}
				int succs[5] = { 0, 0, 0, 0, 0 };
				for (size_t j = 0; j < mesh->GetControlPointsCount(); j++)
				{
					Vertex vertex;
					auto vec4 = mesh->GetControlPointAt(j);
					vertex.position = { vec4[0], vec4[1] , vec4[2] };
					result.back().vertices.push_back(vertex);
				}
				for (size_t j = 0; j < mesh->GetPolygonVertexCount(); j++)
				{
					auto idx = mesh->GetPolygonVertices()[j];
					result.back().indices.push_back(idx);

					auto vec4 = mesh->GetElementNormal()->GetDirectArray().GetAt(j);
					result.back().vertices[idx].normal = {vec4[0], vec4[1] , vec4[2]};

					vec4 = mesh->GetElementTangent()->GetDirectArray().GetAt(j);
					result.back().vertices[idx].tangent = { vec4[0], vec4[1] , vec4[2] };

					vec4 = mesh->GetElementBinormal()->GetDirectArray().GetAt(j);
					result.back().vertices[idx].biTangent = { vec4[0], vec4[1] , vec4[2] };

					auto uvIdx = mesh->GetElementUV()->GetIndexArray().GetAt(j);
					auto vec2 = mesh->GetElementUV()->GetDirectArray().GetAt(uvIdx);
					result.back().vertices[idx].texCoords = { vec2[0], 1 - vec2[1] };
				}
			}
		}

		lImporter->Destroy();
		return result;
	}
}
