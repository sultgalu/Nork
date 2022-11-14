#include "LoadUtils.h"

#define STB_IMAGE_IMPLEMENTATION
//#define STBI_NO_BMP
//#define STBI_NO_PSD
//#define STBI_NO_TGA
//#define STBI_NO_GIF
//#define STBI_NO_HDR
//#define STBI_NO_PIC
//#define STBI_NO_PNM
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include <stb/stb_image_write.h>

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
			MetaLogger().Error("Unhandled number of channels: ", channels);
		}
	}
    Image LoadUtils::LoadImage(std::string_view path)
    {
        int width = 0, height = 0, channels = 0;

		unsigned char* data = stbi_load(path.data(), &width, &height, &channels, 0);
		size_t size = (size_t)width * height * channels;
		if (data)
		{
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
	static int WriteImage_stb(const Image& image, const std::string& path, ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::JPEG:
			return stbi_write_jpg(path.c_str(), image.width, image.height, image.channels, image.data.data(), 0);
		case ImageFormat::PNG:
			return stbi_write_png(path.c_str(), image.width, image.height, image.channels, image.data.data(), 0);
		case ImageFormat::BMP:
			return stbi_write_bmp(path.c_str(), image.width, image.height, image.channels, image.data.data());
		default:
			std::unreachable();
		}
	}
	void LoadUtils::WriteImage(const Image& image, const std::string& path, ImageFormat format)
	{
		if (WriteImage_stb(image, path, format) == 0)
			Logger::Error("Failed to write image to ", path);
	}
	void LoadUtils::WriteTexture(const Renderer::Texture2D& tex, const std::string& path, Renderer::ImageFormat format)
	{
		Renderer::Image image;
		image.format = tex.GetAttributes().format;
		image.width = tex.GetWidth();
		image.height = tex.GetHeight();
		image.data = tex.Bind().GetData2D();
		image.channels = image.data.size() / (image.width * image.height);
		WriteImage(image, path, format);
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

	struct FaceElement
	{
		int vert, texCoord, norm;
	};
	struct IdxFace
	{
		std::vector<FaceElement> elements;
	};
	struct Mtl
	{
		std::string name = "";
		float Ns;
		glm::vec3 Ka;
		glm::vec3 Kd;
		glm::vec3 Ks;
		glm::vec3 Ke;
		std::unordered_map<TextureMap, std::string> textureMaps;
	};
	struct ObjObject
	{
		std::string name;
		std::vector<std::pair<Mtl, std::vector<IdxFace>>> facesByMtl;
	};

	std::string ParseMtl(std::istream& ifs, Mtl& mtl)
	{
		while (true)
		{
			auto parseVec3 = [&](glm::vec3& vec)
			{
				ifs >> vec.x;
				ifs >> vec.y;
				ifs >> vec.z;
			};

			std::string type;
			ifs >> type;
			while (ifs.peek() == ' ')
				ifs.get();

			if (type == "Ka")
				parseVec3(mtl.Ka);
			else if (type == "Kd")
				parseVec3(mtl.Kd);
			else if (type == "Ks")
				parseVec3(mtl.Ks);
			else if (type == "Ke")
				parseVec3(mtl.Ke);

			else if (type == "Ns")
				ifs >> mtl.Ns;

			else if (type == "map_Kd")
			{
				char buf[1000];
				ifs.getline(buf, sizeof(buf), '\n');
				mtl.textureMaps[TextureMap::Diffuse] = buf;
			}
			else if (type == "map_Bump")
			{
				std::string next;
				if (ifs.peek() == '-')
				{
					ifs >> next; // -bm
					ifs >> next; // num
					while (ifs.peek() == ' ')
						ifs.get();
				}

				char buf[1000];
				ifs.getline(buf, sizeof(buf), '\n');
				mtl.textureMaps[TextureMap::Normal] = buf;
			}
			else if (type == "refl")
			{
				char buf[1000];
				ifs.getline(buf, sizeof(buf), '\n');
				mtl.textureMaps[TextureMap::Reflection] = buf;
			}
			else if (type == "map_Ks")
			{
				char buf[1000];
				ifs.getline(buf, sizeof(buf), '\n');
				mtl.textureMaps[TextureMap::Roughness] = buf;
			}
			else if (type == "newmtl")
				return type;
			else if (ifs.eof())
				return "";

			else
			{
				char buf[1000];
				ifs.getline(buf, sizeof(buf));
				std::cout << "failed to parse line:\n \"" << buf << "\"\n";
			}
		}
	}

	std::unordered_map<std::string, Mtl> ParseMtl(const std::string& path)
	{
		std::ifstream ifs(path);
		if (!ifs.is_open())
		{
			Logger::Error("failed to open file ", path);
			std::abort();
		}
		std::unordered_map<std::string, Mtl> result;

		std::string str = "";

		while (true)
		{
			if (str == "newmtl")
			{
				ifs >> str;
				result[str].name = str;
				str = ParseMtl(ifs, result[str]);
				continue;
			}
			else if (ifs.eof())
			{
				break;
			}

			ifs >> str;
		}
		return result;
	}

	std::string GetVertices(std::string pf, std::ifstream& ifs, std::vector<glm::vec2>& data)
	{
		while (true)
		{
			glm::vec2 vec;
			ifs >> vec.x;
			ifs >> vec.y;
			vec.y = 1 - vec.y;
			data.push_back(vec);
			std::string prefix;
			ifs >> prefix;
			if (prefix != pf)
			{
				return prefix;
			}
		}
	}
	std::string GetVertices(std::string pf, std::ifstream& ifs, std::vector<glm::vec3>& data)
	{
		while (true)
		{
			glm::vec3 vec;
			ifs >> vec.x;
			ifs >> vec.y;
			ifs >> vec.z;
			data.push_back(vec);
			std::string prefix;
			ifs >> prefix;
			if (prefix != pf)
			{
				return prefix;
			}
		}
	}

	IdxFace GetFace(std::ifstream& ifs)
	{
		IdxFace face;
		while (true)
		{
			FaceElement vec;

			char buf[10];
			ifs.getline(buf, sizeof(buf), '/');
			vec.vert = std::stoi(buf);
			ifs.getline(buf, sizeof(buf), '/');
			if (buf[0] != '\0')
			{
				vec.texCoord = std::stoi(buf);
			}
			ifs >> vec.norm;
			face.elements.push_back(vec);
			char next = ifs.peek();
			if (next != ' ')
			{
				return face;
			}
		}
	}

	std::vector<MeshData> LoadModel2(const std::string& path)
	{
		std::ifstream obj(path);

		std::vector<glm::vec3> verts;
		std::vector<glm::vec3> norms;
		std::vector<glm::vec2> texCoords;
		// idxFaces.reserve(200 * 1000);
		// verts.reserve(200 * 1000);
		// norms.reserve(200 * 1000);
		// texCoords.reserve(200 * 1000);
		std::vector<ObjObject> objects;
		std::unordered_map<std::string, Mtl> mtls;

		std::string str = "";
		while (true)
		{
			if (str == "v")
			{
				str = GetVertices("v", obj, verts);
				continue;
			}
			else if (str == "vn")
			{
				str = GetVertices("vn", obj, norms);
				continue;
			}
			else if (str == "vt")
			{
				str = GetVertices("vt", obj, texCoords);
				continue;
			}
			else if (str == "f")
			{
				objects.back().facesByMtl.back().second.push_back(GetFace(obj));
			}
			else if (str == "o")
			{
				std::cout << "o";
				char buf[1000];
				objects.push_back(ObjObject());
				obj.getline(buf, sizeof(buf));
				objects.back().name = buf;
			}
			else if (str == "mtllib")
			{
				if (!mtls.empty())
				{
					std::abort(); // multiple mtl files referenced. should implement this.
				}
				std::string mtlPath;
				obj >> mtlPath;
				mtls = ParseMtl(path.substr(0, path.find_last_of("/\\") + 1) + mtlPath);
				std::cout << "ad";
			}
			else if (str == "usemtl")
			{
				std::string mtlName;
				obj >> mtlName;
				objects.back().facesByMtl.push_back({});
				objects.back().facesByMtl.back().first = mtls[mtlName];
			}
			else if (obj.eof())
			{
				break;
			}
			else
			{
				char buf[1000];
				obj.getline(buf, sizeof(buf));
				std::cout << "failed to parse line:\n \"" << buf << "\"\n";
			}
			str.clear();
			obj >> str;
		}
		std::cout << "verts: " << verts.size() << std::endl;
		std::cout << "norms: " << norms.size() << std::endl;
		std::cout << "texCoords: " << texCoords.size() << std::endl;
		std::cout << " " << std::endl;

		std::vector<MeshData> meshDatas;

		for (auto& o : objects)
		{
			for (auto& a : o.facesByMtl)
			{
				auto& mtl = a.first;
				auto& idxFaces = a.second;

				MeshData meshData;
				std::vector<std::vector<uint32_t>> faces;
				faces.reserve(idxFaces.size());

				for (size_t i = 0; i < idxFaces.size(); i++)
				{
					std::vector<uint32_t> face;
					for (size_t j = 0; j < idxFaces[i].elements.size(); j++)
					{
						Data::Vertex vertex;
						vertex.position = verts[idxFaces[i].elements[j].vert - 1];
						vertex.normal = norms[idxFaces[i].elements[j].norm - 1];
						vertex.texCoords = texCoords[idxFaces[i].elements[j].texCoord - 1];
						meshData.vertices.push_back(vertex);
						face.push_back(meshData.vertices.size() - 1);
					}

					auto pos1 = meshData.vertices[face[0]].position;
					auto pos2 = meshData.vertices[face[1]].position;
					auto pos3 = meshData.vertices[face[2]].position;

					auto uv1 = meshData.vertices[face[0]].texCoords;
					auto uv2 = meshData.vertices[face[1]].texCoords;
					auto uv3 = meshData.vertices[face[2]].texCoords;

					glm::vec3 edge1 = pos2 - pos1;
					glm::vec3 edge2 = pos3 - pos1;
					glm::vec2 deltaUV1 = uv2 - uv1;
					glm::vec2 deltaUV2 = uv3 - uv1;

					float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

					glm::vec3 tangent;
					tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
					tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
					tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
					for (size_t j = 0; j < face.size(); j++)
					{
						meshData.vertices[face[j]].tangent = tangent;
					}
					faces.push_back(face);
				}

				for (auto face : faces)
				{
					for (size_t i = 0; i < face.size() - 2; i++)
					{
						meshData.indices.push_back(face[0]);
						meshData.indices.push_back(face[i + 1]);
						meshData.indices.push_back(face[i + 2]);
					}
				}

				meshData.material.diffuse = mtl.Kd;
				meshData.material.specular = (mtl.Ks.x + mtl.Ks.y + mtl.Ks.z) / 3.0f;
				meshData.material.specularExponent = mtl.Ns;
				meshData.material.textureMaps = mtl.textureMaps;

				meshData.meshName = o.name;
				meshData.materialName = mtl.name;
				meshDatas.push_back(meshData);
			}
		}
		return meshDatas;
	}

	std::vector<MeshData> LoadUtils::LoadModel(const std::string& path)
	{
		return LoadModel2(path);
	}
}
