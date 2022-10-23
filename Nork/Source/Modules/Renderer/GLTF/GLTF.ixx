export module Nork.Renderer:GLTF;
export import Nork.Utils;

export namespace Nork::Renderer::GLTF {
	struct Property
	{
		virtual JsonObject ToJson() const = 0;
		virtual void FromJson(const JsonObject& json) = 0;
		virtual bool Validate() const = 0;
		JsonObject extras;
	};

	struct Attribute
	{
		std::string key;
		int accessor;

		static Attribute POSITION(int accessor) { return Attribute("POSITION", accessor); }
		static Attribute NORMAL(int accessor) { return Attribute("NORMAL", accessor); }
		static Attribute TANGENT(int accessor) { return Attribute("TANGENT", accessor); }
		static Attribute TEXCOORD_0(int accessor) { return Attribute("TEXCOORD_0", accessor); }

	private:
		Attribute(const std::string& key, int accessor)
			: key(key), accessor(accessor)
		{}
	};
	struct Primitive: Property
	{
		int mode = -1;
		int indices = -1; 
		int material = -1;
		std::vector<Attribute> attributes;

		JsonObject ToJson() const override
		{
			JsonObject json;
			json.Property("mode", mode);
			if (indices != -1)
				json.Property("indices", indices);
			if (material != -1)
				json.Property("material", material);

			JsonObject attrs;
			for (auto& attr : attributes)
			{
				attrs.Property(attr.key.c_str(), attr.accessor);
			}
			return json.Property("attributes", attrs);
		}
		void FromJson(const JsonObject& json) override
		{
			mode = GL_TRIANGLES;
			json.GetIfContains<int>("mode", mode);
			json.GetIfContains<int>("indices", indices);
			json.GetIfContains<int>("material", material);

			attributes.clear();
			auto attribsJson = json.Get<JsonObject>("attributes");
			if (attribsJson.Contains("NORMAL"))
				attributes.push_back(Attribute::NORMAL(attribsJson.Get<int>("NORMAL")));
			if (attribsJson.Contains("POSITION"))
				attributes.push_back(Attribute::POSITION(attribsJson.Get<int>("POSITION")));
			if (attribsJson.Contains("TANGENT"))
				attributes.push_back(Attribute::TANGENT(attribsJson.Get<int>("TANGENT")));
			if (attribsJson.Contains("TEXCOORD_0"))
				attributes.push_back(Attribute::TEXCOORD_0(attribsJson.Get<int>("TEXCOORD_0")));
		}
		bool Validate() const override
		{
			return !attributes.empty();
		}
	};
	struct Mesh: Property
	{
		std::string name = "";
		std::vector<Primitive> primitives;

		JsonObject ToJson() const override
		{
			JsonObject json;
			if (!name.empty())
				json.Property("name", name);

			JsonArray jsonArray;
			for (auto& prim : primitives)
			{
				jsonArray.Element(prim.ToJson());
			}

			return json.Property("primitives", jsonArray);
		}
		void FromJson(const JsonObject& json) override
		{
			json.GetIfContains("name", name);

			primitives.clear();
			auto jsonArray = json.Get<JsonArray>("primitives");
			for (size_t i = 0; i < jsonArray.Size(); i++)
			{
				primitives.push_back(Primitive());
				primitives.back().FromJson(jsonArray.Get<JsonObject>(i));
			}
		}
		bool Validate() const override
		{
			return !primitives.empty();
		}
	};

	struct Buffer: Property
	{
		int byteLength = -1;
		std::string uri = "";

		JsonObject ToJson() const override
		{
			auto json = JsonObject()
				.Property("byteLength", byteLength);
			if (!uri.empty())
				json.Property("uri", uri);
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			byteLength = json.Get<int>("byteLength");
			json.GetIfContains("uri", uri);
		}
		virtual bool Validate() const override
		{
			return byteLength >= 1;
		}
	};
	struct BufferView: Property
	{
		int buffer = -1;
		int byteOffset = -1;
		int byteLength = -1;
		int byteStride = -1;
		int target = -1; // optional

		JsonObject ToJson() const override
		{
			auto json = JsonObject()
				.Property("buffer", buffer)
				.Property("byteLength", byteLength);

			if (byteStride >= 4)
				json.Property("byteStride", byteStride);
			if (byteOffset != -1)
				json.Property("byteOffset", byteOffset);
			if (target != -1)
				json.Property("target", target);
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			buffer = json.Get<int>("buffer");
			byteLength = json.Get<int>("byteLength");

			byteOffset = 0;
			json.GetIfContains("byteOffset", byteOffset);
			byteStride = 0;
			json.GetIfContains("byteStride", byteStride);
			json.GetIfContains("target", target);
		}
		virtual bool Validate() const override
		{
			return buffer >= 0
				&& byteLength > 0;
		}
	};

	struct Accessor: Property
	{
		int bufferView = -1;
		int byteOffset = 0;
		std::string type;
		int componentType;
		int count;
		glm::vec3 min = glm::vec3(std::nan(""));
		glm::vec3 max = glm::vec3(std::nan(""));

		inline static const std::string SCALAR = "SCALAR";
		inline static const std::string VEC2 = "VEC2";
		inline static const std::string VEC3 = "VEC3";
		inline static const std::string VEC4 = "VEC4";
		inline static const std::string MAT2 = "MAT2";
		inline static const std::string MAT3 = "MAT3";
		inline static const std::string MAT4 = "MAT4";

		JsonObject ToJson() const override
		{
			auto json = JsonObject()
				.Property("count", count)
				.Property("type", type)
				.Property("componentType", componentType)
				.Property("byteOffset", byteOffset);

			if (bufferView != -1)
				json.Property("bufferView", bufferView);
			if (!glm::isnan(max.x))
				json.Property("max", JsonArray().Elements(&max.x, 3));
			if (!glm::isnan(min.x))
				json.Property("min", JsonArray().Elements(&min.x, 3));
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			count = json.Get<int>("count");
			type = json.Get<std::string>("type");
			componentType = json.Get<int>("componentType");
			json.GetIfContains("byteOffset", byteOffset);
			json.GetIfContains("bufferView", bufferView);
			if (json.Contains("min"))
				json.Get<JsonArray>("min").Get(&min.x, 3);
			if (json.Contains("max"))
				json.Get<JsonArray>("max").Get(&max.x, 3);
		}
		virtual bool Validate() const override
		{
			return count > 0 && (
				type == SCALAR || type == VEC2 || type == VEC3 || type == VEC4 ||
				type == MAT2 || type == MAT3 || type == MAT4) && (
				componentType == GL_BYTE || componentType == GL_UNSIGNED_BYTE || componentType == GL_SHORT ||
				componentType == GL_UNSIGNED_SHORT || componentType == GL_UNSIGNED_INT || componentType == GL_FLOAT);
		}
	};

	struct TextureInfo: Property
	{
		int index = -1;
		int texCoord = -1;

		JsonObject ToJson() const override
		{
			auto json = JsonObject()
				.Property("index", index);
			if (texCoord != -1)
				json.Property("texCoord", texCoord);
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			index = json.Get<int>("index");
			texCoord = 0;
			json.GetIfContains("texCoord", texCoord);
		}
		virtual bool Validate() const override
		{
			return index >= 0;
		}
	};

	struct MaterialRoughnessModel: Property
	{
		glm::vec4 baseColorFactor = glm::vec4(1);
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;

		TextureInfo baseColorTexture;
		TextureInfo metallicRoughnessTexture;

		JsonObject ToJson() const override
		{
			auto json = JsonObject()
				.Property("baseColorFactor", JsonArray().Elements(&baseColorFactor.r, 4))
				.Property("metallicFactor", metallicFactor)
				.Property("roughnessFactor", roughnessFactor)
				.Property("extras", extras);

			if (baseColorTexture.Validate())
				json.Property("baseColorTexture", baseColorTexture.ToJson());
			if (metallicRoughnessTexture.Validate())
				json.Property("metallicRoughnessTexture", metallicRoughnessTexture.ToJson());

			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			json.GetIfContains("metallicFactor", metallicFactor);
			json.GetIfContains("roughnessFactor", roughnessFactor);
			json.GetIfContains("extras", extras);
			if (json.Contains("baseColorFactor"))
			{
				json.Get<JsonArray>("baseColorFactor").Get<float>(&baseColorFactor.r, 4);
			}
			if (json.Contains("baseColorTexture"))
			{
				baseColorTexture.FromJson(json.Get<JsonObject>("baseColorTexture"));
			}
			if (json.Contains("metallicRoughnessTexture"))
			{
				metallicRoughnessTexture.FromJson(json.Get<JsonObject>("metallicRoughnessTexture"));
			}
		}
		virtual bool Validate() const override
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (baseColorFactor[i] < 0 || baseColorFactor[i] > 1)
					return false;
			}
			if (metallicFactor < 0 || metallicFactor > 1)
				return false;
			if (roughnessFactor < 0 || roughnessFactor > 1)
				return false;

			return true;
		}
	};

	struct Material: Property
	{
		std::string name = "";
		MaterialRoughnessModel pbrMetallicRoughness;
		TextureInfo normalTexture;

		JsonObject ToJson() const override
		{
			auto json = JsonObject()
				.Property("pbrMetallicRoughness", pbrMetallicRoughness.ToJson());
			if (normalTexture.Validate())
				json.Property("normalTexture", normalTexture.ToJson());
			if (!name.empty())
				json.Property("name", name);
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			json.GetIfContains("name", name);
			if (json.Contains("normalTexture"))
			{
				normalTexture.FromJson(json.Get<JsonObject>("normalTexture"));
			}
			pbrMetallicRoughness.FromJson(json.Get<JsonObject>("pbrMetallicRoughness"));
		}
		virtual bool Validate() const override
		{
			return pbrMetallicRoughness.Validate();
		}
	};

	struct Texture: Property
	{
		int source = -1;

		JsonObject ToJson() const override
		{
			auto json = JsonObject();
			if (source != -1)
				json.Property("source", source);
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			json.GetIfContains("source", source);
		}
		virtual bool Validate() const override
		{
			return source >= 0;
		}
	};

	struct Image: Property
	{
		std::string uri = "";

		JsonObject ToJson() const override
		{
			auto json = JsonObject();
			if (!uri.empty())
				json.Property("uri", uri);
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			json.GetIfContains("uri", uri);
		}
		virtual bool Validate() const override
		{
			return true;
		}
	};

	struct Node: Property
	{
		// translation, rotation, scale
		int mesh = -1; // could also be camera
		std::string name = "";
		std::vector<int> children = {};

		JsonObject ToJson() const override
		{
			auto json = JsonObject();
			if (mesh != -1)
				json.Property("mesh", mesh);
			if (!name.empty())
				json.Property("name", name);
			if (!children.empty())
			{
				json.Property("children", JsonArray().Elements(children));
			}
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			json.GetIfContains("mesh", mesh);
			json.GetIfContains("name", name);
			if (json.Contains("children"))
			{
				auto jsonArray = json.Get<JsonArray>("children");
				children = jsonArray.Get<int>();
			}
		}
		virtual bool Validate() const override
		{
			return mesh >= -1;
		}
	};
	struct Scene: Property
	{
		std::string name = "";
		std::vector<int> nodes; // root nodes (not referred by any "children" property)

		JsonObject ToJson() const override
		{
			auto json = JsonObject();
			if (!name.empty())
				json.Property("name", name);
			if (!nodes.empty())
			{
				json.Property("nodes", JsonArray().Elements(nodes));
			}
			return json;
		}
		virtual void FromJson(const JsonObject& json) override
		{
			json.GetIfContains("name", name);
			if (json.Contains("nodes"))
			{
				auto jsonArray = json.Get<JsonArray>("nodes");
				nodes = jsonArray.Get<int>();
			}
		}
		virtual bool Validate() const override
		{
			return true;
		}
	};

	struct Asset: Property
	{
		std::string version = "2.0";
		std::string generator = "Nork glTF";

		JsonObject ToJson() const override
		{
			return JsonObject()
				.Property("version", version)
				.Property("generator", generator);
		}
		virtual void FromJson(const JsonObject& json) override
		{
			version = json.Get<std::string>("version");
			version = json.Get<std::string>("generator");
		}
		virtual bool Validate() const override
		{
			return true;
		}
	};
	
	struct GLTF
	{
		Asset asset;
		std::vector<Scene> scenes;
		std::vector<Node> nodes;
		
		std::vector<Mesh> meshes;
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Accessor> accessors;

		std::vector<Material> materials;
		std::vector<Texture> textures;
		std::vector<Image> images;

		int scene = 0; // scene to display

		template<std::derived_from<Property> T>
		JsonArray ElementsToJsonArray(const std::vector<T>& elements) const
		{
			JsonArray arr;
			for (auto& e : elements)
			{
				arr.Element(e.ToJson());
			}
			return arr;
		}
		template<std::derived_from<Property> T>
		void ParseJsonArray(std::vector<T>& elements, const JsonArray& arr)
		{
			elements.clear();
			for (size_t i = 0; i < arr.Size(); i++)
			{
				elements.push_back(T());
				elements.back().FromJson(arr.Get<JsonObject>(i));
			}
		}
		JsonObject ToJson() const
		{
			JsonObject json = JsonObject().Property("asset", asset.ToJson());

			if (!accessors.empty())
				json.Property("accessors", ElementsToJsonArray(accessors));
			if (!bufferViews.empty())
				json.Property("bufferViews", ElementsToJsonArray(bufferViews));
			if (!buffers.empty())
				json.Property("buffers", ElementsToJsonArray(buffers));
			if (!materials.empty())
				json.Property("materials", ElementsToJsonArray(materials));
			if (!images.empty())
				json.Property("images", ElementsToJsonArray(images));
			if (!textures.empty())
				json.Property("textures", ElementsToJsonArray(textures));
			if (!meshes.empty())
				json.Property("meshes", ElementsToJsonArray(meshes));
			if (!nodes.empty())
				json.Property("nodes", ElementsToJsonArray(nodes));
			if (!scenes.empty())
				json.Property("scenes", ElementsToJsonArray(scenes));

			return json.Property("scene", scene);
		}

		static GLTF FromJson(const JsonObject& json)
		{
			GLTF gltf;
			gltf.asset.FromJson(json.Get<JsonObject>("asset"));
			if (json.Contains("accessors"))
				gltf.ParseJsonArray(gltf.accessors, json.Get<JsonArray>("accessors"));
			if (json.Contains("bufferViews"))
				gltf.ParseJsonArray(gltf.bufferViews, json.Get<JsonArray>("bufferViews"));
			if (json.Contains("buffers"))
				gltf.ParseJsonArray(gltf.buffers, json.Get<JsonArray>("buffers"));
			if (json.Contains("materials"))
				gltf.ParseJsonArray(gltf.materials, json.Get<JsonArray>("materials"));
			if (json.Contains("images"))
				gltf.ParseJsonArray(gltf.images, json.Get<JsonArray>("images"));
			if (json.Contains("textures"))
				gltf.ParseJsonArray(gltf.textures, json.Get<JsonArray>("textures"));
			if (json.Contains("meshes"))
				gltf.ParseJsonArray(gltf.meshes, json.Get<JsonArray>("meshes"));
			if (json.Contains("nodes"))
				gltf.ParseJsonArray(gltf.nodes, json.Get<JsonArray>("nodes"));
			if (json.Contains("scenes"))
				gltf.ParseJsonArray(gltf.scenes, json.Get<JsonArray>("scenes"));

			gltf.scene = json.Get<int>("scene");
			return gltf;
		}
	};
}