#pragma once

#include "../Json.h"

namespace Nork::glTF {
	struct Attribute
	{
		std::string key;
		int accessor; // int?

		static Attribute POSITION(int accessor) { return Attribute("POSITION", accessor); }
		static Attribute NORMAL(int accessor) { return Attribute("NORMAL", accessor); }
		static Attribute TANGENT(int accessor) { return Attribute("TANGENT", accessor); }
		static Attribute TEXCOORD_0(int accessor) { return Attribute("TEXCOORD_0", accessor); }
	private:
		Attribute(const std::string& key, int accessor)
			: key(key), accessor(accessor)
		{}
	};
	struct Primitive
	{
		int mode;
		std::vector<Attribute> attributes;
		int indices; // int?
		int material; // int?
	};
	struct Mesh
	{
		std::string name;
		std::vector<Primitive> primitives;
	};

	struct Buffer
	{
		int byteLength;
		std::string uri;
	};

	struct BufferView
	{
		int buffer; // int?
		int byteOffset;
		int byteLength;
		int byteStride;
		int target;
	};

	

	struct Accessor
	{
		int bufferView;
		int byteOffset;
		std::string type;
		int componentType;
		int count;

		inline static const char* SCALAR = "SCALAR";
		inline static const char* VEC2 = "VEC2";
		inline static const char* VEC3 = "VEC3";
		inline static const char* VEC4 = "VEC4";
		inline static const char* MAT2 = "MAT2";
		inline static const char* MAT3 = "MAT3";
		inline static const char* MAT4 = "MAT4";
	};

	struct Texture
	{
		int index;
		int texCoord = 0;
	};
	struct MaterialRoughnessModel
	{
		Texture baseColorTexture;
		glm::vec4 baseColorFactor;

		Texture metallicRoughnessTexture;
		float metallicFactor = 0.0f;
		float roughnessFactor = 0.0f;
	};

	struct Material
	{
		std::string name;
		MaterialRoughnessModel pbrMetallicRoughness;
		Texture normalTexture;
	};

	struct Asset
	{
		const char* version = "2.0";
		const char* generator = "Nork glTF";
	};

	struct Image
	{
		std::string uri;
	};

	struct Texture
	{
		int sampler;
		int source;
	};

	struct Node
	{
		// translation, rotation, scale
		int mesh; // could also be camera
		std::string name;
		std::vector<int> children;
	};

	struct Scene
	{
		std::string name;
		std::vector<int> nodes; // root nodes (not referred by any "children" property)
	};

	struct GLTF
	{
		Asset asset;
		std::vector<Accessor> accessors;
		std::vector<BufferView> bufferViews;
		std::vector<Buffer> buffers;
		std::vector<Image> images;
		std::vector<Mesh> meshes;
		std::vector<Node> nodes;
		std::vector<Scene> scenes;
		int scene = 0; // scene to display

		JsonArray AccessorsJson()
		{
			JsonArray array = JsonArray();
			for (auto& element : accessors)
			{
				JsonObject obj = JsonObject()
					.Property("bufferView", element.bufferView)
					.Property("componentType", element.componentType)
					.Property("count", element.count)
					.Property("type", element.type)
					.Property("byteOffset", element.byteOffset);
				array.Element(obj);
			}
			return array;
		}
		void ParseAccessors(const JsonArray& array)
		{
			accessors.clear();
			for (auto& obj : array.Get<JsonObject>())
			{
				accessors.push_back(Accessor());
				accessors.back().bufferView = obj.Get<int>("bufferView");
				accessors.back().componentType = obj.Get<int>("componentType");
				accessors.back().count = obj.Get<int>("count");
				accessors.back().byteOffset = obj.Get<int>("byteOffset");
				accessors.back().type = obj.Get<std::string>("type");
			}
		}
		JsonArray BufferViewsJson()
		{
			JsonArray array = JsonArray();
			for (auto& element : bufferViews)
			{
				JsonObject obj = JsonObject()
					.Property("buffer", element.buffer)
					.Property("byteLength", element.byteLength)
					.Property("byteOffset", element.byteOffset)
					.Property("byteStride", element.byteStride)
					.Property("target", element.target);
				array.Element(obj);
			}
			return array;
		}
		void ParseBufferViews(const JsonArray& array)
		{
			bufferViews.clear();
			for (auto& obj : array.Get<JsonObject>())
			{
				bufferViews.push_back(BufferView());
				bufferViews.back().buffer = obj.Get<int>("buffer");
				bufferViews.back().byteLength = obj.Get<int>("byteLength");
				bufferViews.back().byteOffset = obj.Get<int>("byteOffset");
				bufferViews.back().byteStride = obj.Get<int>("byteStride");
				bufferViews.back().target = obj.Get<int>("target");
			}
		}
		JsonArray BuffersJson()
		{
			JsonArray array = JsonArray();
			for (auto& element : buffers)
			{
				JsonObject obj = JsonObject()
					.Property("byteLength", element.byteLength)
					.Property("uri", element.uri);
				array.Element(obj);
			}
			return array;
		}
		void ParseBuffers(const JsonArray& array)
		{
			buffers.clear();
			for (auto& obj : array.Get<JsonObject>())
			{
				buffers.push_back(Buffer());
				buffers.back().byteLength = obj.Get<int>("byteLength");
				buffers.back().uri = obj.Get<std::string>("uri");
			}
		}
		JsonArray ImagesJson()
		{
			JsonArray array = JsonArray();
			for (auto& element : images)
			{
				JsonObject obj = JsonObject()
					.Property("uri", element.uri);
				array.Element(obj);
			}
			return array;
		}
		void ParseImages(const JsonArray& array)
		{
			images.clear();
			for (auto& obj : array.Get<JsonObject>())
			{
				images.push_back(Image());
				images.back().uri = obj.Get<std::string>("uri");
			}
		}
		JsonObject AttributesJson(const Primitive& primitive)
		{
			JsonObject obj;
			for (auto& attr : primitive.attributes)
			{
				obj.Property(attr.key.c_str(), attr.accessor);
			}
		}
		Primitive ParseAttributes(const JsonObject& obj)
		{
			Primitive primitive;
			
			if (obj.Contains("NORMAL"))
				primitive.attributes.push_back(Attribute::NORMAL(obj.Get<int>("NORMAL")));
			if (obj.Contains("POSITION"))
				primitive.attributes.push_back(Attribute::POSITION(obj.Get<int>("POSITION")));
			if (obj.Contains("TANGENT"))
				primitive.attributes.push_back(Attribute::TANGENT(obj.Get<int>("TANGENT")));
			if (obj.Contains("TEXCOORD_0"))
				primitive.attributes.push_back(Attribute::TEXCOORD_0(obj.Get<int>("TEXCOORD_0")));

			return primitive;
		}
		JsonArray PrimitivesJson(const Mesh& mesh)
		{
			JsonArray array = JsonArray();
			for (auto& element : mesh.primitives)
			{
				JsonObject obj = JsonObject()
					.Property("mode", element.mode)
					.Property("indices", element.indices)
					.Property("material", element.material)
					.Property("attributes", AttributesJson(element));
				array.Element(obj);
			}
			return array;
		}
		Mesh ParsePrimitives(const JsonArray& array)
		{
			Mesh mesh;
			for (auto& obj : array.Get<JsonObject>())
			{
				mesh.primitives.push_back(ParseAttributes(obj.Get<JsonObject>("attributes")));
				mesh.primitives.back().indices = obj.Get<int>("indices");
				mesh.primitives.back().material = obj.Get<int>("material");
				mesh.primitives.back().mode = obj.Get<int>("mode");
			}
			return mesh;
		}
		JsonArray MeshesJson()
		{
			JsonArray array = JsonArray();
			for (auto& element : meshes)
			{
				JsonArray primitives;
				JsonObject obj = JsonObject()
					.Property("name", element.name)
					.Property("primitives", PrimitivesJson(element));
				array.Element(obj);
			}
			return array;
		}
		void ParseMeshes(const JsonArray& array)
		{
			meshes.clear();
			for (auto& obj : array.Get<JsonObject>())
			{
				meshes.push_back(ParsePrimitives(obj.Get<JsonArray>("primitives")));
				meshes.back().name = obj.Get<std::string>("name");
			}
		}
		JsonArray NodesJson()
		{
			JsonArray array = JsonArray();
			for (auto& element : nodes)
			{
				JsonObject obj = JsonObject()
					.Property("name", element.name)
					.Property("mesh", element.mesh);
				if (element.children.size() > 0)
					obj.Property("children", JsonArray().Elements(element.children));

				array.Element(obj);
			}
			return array;
		}
		void ParseNodes(const JsonArray& array)
		{
			nodes.clear();
			for (auto& obj : array.Get<JsonObject>())
			{
				nodes.push_back(Node());
				nodes.back().mesh = obj.Get<int>("mesh");
				nodes.back().name = obj.Get<std::string>("name");
				nodes.back().children = obj.Get<JsonArray>("children").Get<int>();
			}
		}
		JsonArray ScenesJson()
		{
			JsonArray array = JsonArray();
			for (auto& element : scenes)
			{
				JsonObject obj = JsonObject()
					.Property("name", element.name)
					.Property("nodes", JsonArray().Elements(element.nodes));

				array.Element(obj);
			}
			return array;
		}
		void ParseScenes(const JsonArray& array)
		{
			scenes.clear();
			for (auto& obj : array.Get<JsonObject>())
			{
				scenes.push_back(Scene());
				scenes.back().name = obj.Get<std::string>("name");
				scenes.back().nodes = obj.Get<JsonArray>("nodes").Get<int>();
			}
		}

		JsonObject ToJson()
		{
			return JsonObject()
				.Property("accessors", AccessorsJson())
				.Property("bufferViews", BufferViewsJson())
				.Property("buffers", BuffersJson())
				.Property("images", ImagesJson())
				.Property("meshes", MeshesJson())
				.Property("nodes", NodesJson())
				.Property("scenes", ScenesJson())
				.Property("scene", scene);
		}

		static GLTF FromJson(const JsonObject& json)
		{
			GLTF gltf;
			if (json.Contains("accessors"))
				gltf.ParseAccessors(json.Get<JsonArray>("accessors"));
			if (json.Contains("bufferViews"))
				gltf.ParseBufferViews(json.Get<JsonArray>("bufferViews"));
			if (json.Contains("buffers"))
				gltf.ParseBuffers(json.Get<JsonArray>("buffers"));
			if (json.Contains("images"))
				gltf.ParseImages(json.Get<JsonArray>("images"));
			if (json.Contains("meshes"))
				gltf.ParseMeshes(json.Get<JsonArray>("meshes"));
			if (json.Contains("nodes"))
				gltf.ParseNodes(json.Get<JsonArray>("nodes"));
			if (json.Contains("scenes"))
				gltf.ParseScenes(json.Get<JsonArray>("scenes"));
			gltf.scene = json.Get<int>("scene");
		}

		static bool ValidateJson(const JsonObject& json)
		{
			return
				json.Contains("accessors") &&
				json.Contains("bufferViews") &&
				json.Contains("buffers") &&
				json.Contains("images") &&
				json.Contains("meshes") &&
				json.Contains("nodes") &&
				json.Contains("scenes") &&
				json.Contains("scene");
		}
	};
}