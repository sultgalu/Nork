#pragma once

namespace Nork::Renderer::GLTF {
#define make_opt(name) inline static const std::string name = #name

static std::string PrecentDecode(const std::string& s)
{
    std::string result;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%') {
            int c;
            std::istringstream({ s[i + 1], s[i + 2] }) >> std::hex >> c;
            result += (char)c;
            i += 2;
        } else
            result += s[i];
    }
    return result;
}

struct Property {
    JsonObject extras;
    JsonObject Serialize() const
    {
        auto json = ToJson();
        if (!extras.Empty()) {
            json.Property("extras", extras);
        }
        return json;
    }
    void Deserialize(const JsonObject& json)
    {
        json.GetIfContains("extras", extras);
        FromJson(json);
    }

protected:
    virtual JsonObject ToJson() const = 0;
    virtual void FromJson(const JsonObject& json) = 0;
    virtual bool Validate() const = 0;
};

struct Attribute {
    constexpr static const char* position = "POSITION";
    constexpr static const char* normal = "NORMAL";
    constexpr static const char* tangent = "TANGENT";
    constexpr static const char* texcoord0 = "TEXCOORD_0";
    constexpr static const char* joints0 = "JOINTS_0";
    constexpr static const char* weights0 = "WEIGHTS_0";

    std::string key;
    int accessor;

    static Attribute POSITION(int accessor) { return Attribute(position, accessor); }
    static Attribute NORMAL(int accessor) { return Attribute(normal, accessor); }
    static Attribute TANGENT(int accessor) { return Attribute(tangent, accessor); }
    static Attribute TEXCOORD_0(int accessor) { return Attribute(texcoord0, accessor); }
    static Attribute JOINTS_0(int accessor) { return Attribute(joints0, accessor); }
    static Attribute WEIGHTS_0(int accessor) { return Attribute(weights0, accessor); }
private:
    Attribute(const std::string& key, int accessor)
        : key(key)
        , accessor(accessor)
    {
    }
};
struct Primitive : Property {
    int mode = -1;
    int indices = -1;
    int material = -1;
    std::vector<Attribute> attributes;

    int Accessor(const char* attribute) const
    {
        for (auto& attrib : attributes)
            if (attrib.key == attribute)
                return attrib.accessor;
        return -1;
    }
    JsonObject ToJson() const override
    {
        JsonObject json;
        json.Property("mode", mode);
        if (indices != -1)
            json.Property("indices", indices);
        if (material != -1)
            json.Property("material", material);

        JsonObject attrs;
        for (auto& attr : attributes) {
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
        if (attribsJson.Contains(Attribute::normal))
            attributes.push_back(Attribute::NORMAL(attribsJson.Get<int>(Attribute::normal)));
        if (attribsJson.Contains(Attribute::position))
            attributes.push_back(Attribute::POSITION(attribsJson.Get<int>(Attribute::position)));
        if (attribsJson.Contains(Attribute::tangent))
            attributes.push_back(Attribute::TANGENT(attribsJson.Get<int>(Attribute::tangent)));
        if (attribsJson.Contains(Attribute::texcoord0))
            attributes.push_back(Attribute::TEXCOORD_0(attribsJson.Get<int>(Attribute::texcoord0)));
        if (attribsJson.Contains(Attribute::joints0))
            attributes.push_back(Attribute::JOINTS_0(attribsJson.Get<int>(Attribute::joints0)));
        if (attribsJson.Contains(Attribute::weights0))
            attributes.push_back(Attribute::WEIGHTS_0(attribsJson.Get<int>(Attribute::weights0)));
    }
    bool Validate() const override
    {
        return !attributes.empty();
    }
};
struct Mesh : Property {
    std::string name = "";
    std::vector<Primitive> primitives;

    JsonObject ToJson() const override
    {
        JsonObject json;
        if (!name.empty())
            json.Property("name", name);

        JsonArray jsonArray;
        for (auto& prim : primitives) {
            jsonArray.Element(prim.Serialize());
        }

        return json.Property("primitives", jsonArray);
    }
    void FromJson(const JsonObject& json) override
    {
        json.GetIfContains("name", name);

        primitives.clear();
        auto jsonArray = json.Get<JsonArray>("primitives");
        for (size_t i = 0; i < jsonArray.Size(); i++) {
            primitives.push_back(Primitive());
            primitives.back().Deserialize(jsonArray.Get<JsonObject>(i));
        }
    }
    bool Validate() const override
    {
        return !primitives.empty();
    }
};

struct Buffer : Property {
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
struct BufferView : Property {
    int buffer = -1;
    int byteOffset = -1;
    int byteLength = -1;
    int byteStride = 0;
    int target = -1; // optional

    static constexpr int ARRAY_BUFFER = 34962;
    static constexpr int ELEMENT_ARRAY_BUFFER = 34963;

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
        json.GetIfContains("byteStride", byteStride);
        json.GetIfContains("target", target);
    }
    virtual bool Validate() const override
    {
        return buffer >= 0
            && byteLength > 0;
    }
};

struct Accessor : Property {
    int bufferView = -1;
    int byteOffset = 0;
    std::string type;
    int componentType;
    int count;
    // std::optional<glm::vec3> min;
    // std::optional<glm::vec3> max;
    // min/max component count can range from 1 to 4

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
                        .Property("componentType", componentType);
        if (byteOffset != 0)
            json.Property("byteOffset", byteOffset);
        if (bufferView != -1)
            json.Property("bufferView", bufferView);
        // if (max)
        // 	json.Property("max", JsonArray().Elements(*max));
        // if (min)
        // 	json.Property("min", JsonArray().Elements(*min));
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        count = json.Get<int>("count");
        type = json.Get<std::string>("type");
        componentType = json.Get<int>("componentType");
        json.GetIfContains("byteOffset", byteOffset);
        json.GetIfContains("bufferView", bufferView);
        // if (json.Contains("min"))
        // 	json.Get<JsonArray>("min").Get(min.emplace());
        // if (json.Contains("max"))
        // 	json.Get<JsonArray>("max").Get(max.emplace());
    }
    virtual bool Validate() const override
    {
        return count > 0 && (type == SCALAR || type == VEC2 || type == VEC3 || type == VEC4 || type == MAT2 || type == MAT3 || type == MAT4) && (componentType == GL_BYTE || componentType == GL_UNSIGNED_BYTE || componentType == GL_SHORT || componentType == GL_UNSIGNED_SHORT || componentType == GL_UNSIGNED_INT || componentType == GL_FLOAT);
    }
};

struct TextureInfo : Property {
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

struct MaterialRoughnessModel : Property {
    glm::vec4 baseColorFactor = glm::vec4(1);
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;

    TextureInfo baseColorTexture;
    TextureInfo metallicRoughnessTexture;

    JsonObject ToJson() const override
    {
        auto json = JsonObject();
        if (baseColorFactor != glm::vec4(1)) {
            json.Property("baseColorFactor", JsonArray().Elements(&baseColorFactor.r, 4));
        }
        if (metallicFactor != 1.0f) {
            json.Property("metallicFactor", metallicFactor);
        }
        if (roughnessFactor != 1.0f) {
            json.Property("roughnessFactor", roughnessFactor);
        }
        if (baseColorTexture.Validate()) {
            json.Property("baseColorTexture", baseColorTexture.Serialize());
        }
        if (metallicRoughnessTexture.Validate()) {
            json.Property("metallicRoughnessTexture", metallicRoughnessTexture.Serialize());
        }
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.GetIfContains("metallicFactor", metallicFactor);
        json.GetIfContains("roughnessFactor", roughnessFactor);
        if (json.Contains("baseColorFactor")) {
            json.Get<JsonArray>("baseColorFactor").Get<float>(&baseColorFactor.r, 4);
        }
        if (json.Contains("baseColorTexture")) {
            baseColorTexture.Deserialize(json.Get<JsonObject>("baseColorTexture"));
        }
        if (json.Contains("metallicRoughnessTexture")) {
            metallicRoughnessTexture.Deserialize(json.Get<JsonObject>("metallicRoughnessTexture"));
        }
    }
    virtual bool Validate() const override
    {
        for (size_t i = 0; i < 4; i++) {
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

struct Material : Property {
    std::string name = "";
    MaterialRoughnessModel pbrMetallicRoughness;
    TextureInfo normalTexture;
    TextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
    glm::vec3 emissiveFactor = glm::vec3(0);
    std::string alphaMode = OPAQUE;
    float alphaCutoff = 0.5f;
    inline static const std::string OPAQUE = "OPAQUE";
    inline static const std::string MASK = "MASK";
    inline static const std::string BLEND = "BLEND";

    JsonObject ToJson() const override
    {
        auto json = JsonObject()
                        .Property("pbrMetallicRoughness", pbrMetallicRoughness.Serialize());
        if (normalTexture.Validate())
            json.Property("normalTexture", normalTexture.Serialize());
        if (occlusionTexture.Validate())
            json.Property("occlusionTexture", occlusionTexture.Serialize());
        if (emissiveTexture.Validate())
            json.Property("emissiveTexture", emissiveTexture.Serialize());
        if (emissiveFactor != glm::vec3(0))
            json.Property("emissiveFactor", JsonArray().Elements(emissiveFactor));
        if (!name.empty())
            json.Property("name", name);
        if (alphaMode != OPAQUE) {
            json.Property("alphaMode", alphaMode);
            if (alphaCutoff != 0.5f)
                json.Property("alphaCutoff", alphaCutoff);
        }
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.GetIfContains("name", name);
        json.GetIfContains("alphaMode", alphaMode);
        json.GetIfContains("alphaCutoff", alphaCutoff);
        if (json.Contains("normalTexture")) {
            normalTexture.Deserialize(json.Get<JsonObject>("normalTexture"));
        }
        if (json.Contains("occlusionTexture")) {
            occlusionTexture.Deserialize(json.Get<JsonObject>("occlusionTexture"));
        }
        if (json.Contains("emissiveTexture")) {
            emissiveTexture.Deserialize(json.Get<JsonObject>("emissiveTexture"));
        }
        if (json.Contains("emissiveFactor")) {
            json.Get<JsonArray>("emissiveFactor").Get(emissiveFactor);
        }
        pbrMetallicRoughness.Deserialize(json.Get<JsonObject>("pbrMetallicRoughness"));
    }
    virtual bool Validate() const override
    {
        return pbrMetallicRoughness.Validate();
    }
};

struct Texture : Property {
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

struct Image : Property {
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
        if (uri.contains('%'))
            uri = PrecentDecode(uri);
    }
    virtual bool Validate() const override
    {
        return true;
    }
};

struct Node : Property {
    int mesh = -1; // could also be camera
    std::string name = "";
    std::vector<int> children = {};

    std::optional<glm::vec3> translation;
    std::optional<glm::quat> rotation;
    std::optional<glm::vec3> scale;

    std::optional<glm::mat4> matrix; // if present, TRS should not be

    bool HasTransform() const
    {
        return matrix || translation || rotation || scale;
    }
    glm::mat4 Transform() const
    {
        if (matrix)
            return *matrix;
        auto tr = glm::identity<glm::mat4>();
        if (scale)
            tr = glm::scale(tr, *scale);
        if (rotation)
            tr = glm::mat4_cast(*rotation) * tr;
        if (translation)
            tr = glm::translate(glm::identity<glm::mat4>(), *translation) * tr;
        return tr;
    }
    JsonObject ToJson() const override
    {
        auto json = JsonObject();
        if (mesh != -1)
            json.Property("mesh", mesh);
        if (!name.empty())
            json.Property("name", name);
        if (translation.has_value())
            json.Property("translation", JsonArray().Elements(*translation));
        if (rotation.has_value())
            json.Property("rotation", JsonArray().Elements(*rotation));
        if (scale.has_value())
            json.Property("scale", JsonArray().Elements(*scale));
        if (matrix.has_value())
            json.Property("matrix", JsonArray().Elements(*matrix));
        if (!children.empty())
            json.Property("children", JsonArray().Elements(children));
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.GetIfContains("mesh", mesh);
        json.GetIfContains("name", name);
        auto array = JsonArray();
        if (json.GetIfContains("translation", array))
            array.Get(translation.emplace());
        if (json.GetIfContains("rotation", array))
            array.Get(rotation.emplace());
        if (json.GetIfContains("scale", array))
            array.Get(scale.emplace());
        if (json.GetIfContains("matrix", array))
            array.Get(matrix.emplace());
        if (json.GetIfContains("children", array))
            children = array.Get<int>();
    }
    virtual bool Validate() const override
    {
        if (matrix && (translation || rotation || scale))
            return false;
        return mesh >= -1;
    }
};

struct AnimationSampler : Property {
    int input = -1; // required
    int output = -1; // required
    enum Interpolation {
        Linear, Step, CubicSpline
    };
    Interpolation interpolation = Linear;

private:
    make_opt(LINEAR); make_opt(STEP); make_opt(CUBICSPLINE);
public:
    JsonObject ToJson() const override
    {
        auto json = JsonObject()
            .Property("input", input)
            .Property("output", output);
        if (interpolation != Linear) {
            json.Property("interpolation", interpolation == Step ? STEP : CUBICSPLINE);
        }
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.Get("input", input)
            .Get("output", output);
        std::string interpol;
        if (json.GetIfContains("interpolation", interpol)) {
            if (interpol == STEP) interpolation = Step;
            else if (interpol == CUBICSPLINE) interpolation = CubicSpline;
        }
    }
    virtual bool Validate() const override
    {
        return input != -1 && output != -1;
    }
};
struct AnimationChannelTarget: Property {
    int node = -1;
    enum Path {
        None, Translation, Rotation, Scale, Weights
    };
    Path path = None; // required

private:
    make_opt(translation); make_opt(rotation); make_opt(scale); make_opt(weights);
public:
    JsonObject ToJson() const override
    {
        auto json = JsonObject();
        if (node != -1) {
            json.Property("node", node);
        }
        if (path == Translation) json.Property("path", translation);
        if (path == Rotation) json.Property("path", rotation);
        if (path == Scale) json.Property("path", scale);
        if (path == Weights) json.Property("path", weights);
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.GetIfContains("node", node);
        auto p = json.Get<std::string>("path");
        if (p == translation) path = Translation;
        if (p == rotation) path = Rotation;
        if (p == scale) path = Scale;
        if (p == weights) path = Weights;
    }
    virtual bool Validate() const override
    {
        return path != None;
    }
};
struct AnimationChannel: Property {
    int sampler = -1; // required
    AnimationChannelTarget target; // required
    JsonObject ToJson() const override
    {
        auto json = JsonObject()
            .Property("sampler", sampler)
            .Property("target", target.Serialize());
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.Get("sampler", sampler);
        target.Deserialize(json.Get<JsonObject>("target"));
    }
    virtual bool Validate() const override
    {
        return sampler != -1 && target.Validate();
    }
};
struct Animation: Property {
    std::vector<AnimationChannel> channels; // required
    std::vector<AnimationSampler> samplers; // required
    std::string name;
    JsonObject ToJson() const override
    {
        JsonArray channelsArr;
        for (auto& channel : channels) {
            channelsArr.Element(channel.Serialize());
        }
        JsonArray samplersArr;
        for (auto& sampler : samplers) {
            samplersArr.Element(sampler.Serialize());
        }
        auto json = JsonObject()
            .Property("channels", channelsArr)
            .Property("samplers", samplersArr);
        if (!name.empty()) {
            json.Property("name", name);
        }
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        channels.clear(); samplers.clear();
        for (auto& chan : json.Get<JsonArray>("channels").Get<JsonObject>()) {
            channels.emplace_back().Deserialize(chan);
        }
        for (auto& samp : json.Get<JsonArray>("samplers").Get<JsonObject>()) {
            samplers.emplace_back().Deserialize(samp);
        }
        json.GetIfContains("name", name);
    }
    virtual bool Validate() const override
    {
        if (samplers.empty() || channels.empty())
            return false;
        for (auto& c : channels) {
            if (!c.Validate())
                return false;
        }
        for (auto& s : samplers) {
            if (!s.Validate())
                return false;
        }
    }
};
struct Skin : Property {
    int inverseBindMatrices = -1;
    int skeleton = -1;
    std::vector<uint16_t> joints; // required
    std::string name;
public:
    JsonObject ToJson() const override
    {
        auto json = JsonObject()
            .Property("joints", JsonArray().Elements(joints));
        if (inverseBindMatrices != -1) {
            json.Property("inverseBindMatrices", inverseBindMatrices);
        }
        if (skeleton != -1) {
            json.Property("skeleton", skeleton);
        }
        if (!name.empty()) {
            json.Property("name", name);
        }
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.Get<JsonArray>("joints").Get(joints);
        json.GetIfContains("inverseBindMatrices", inverseBindMatrices);
        json.GetIfContains("skeleton", skeleton);
        json.GetIfContains("name", name);
    }
    virtual bool Validate() const override
    {
        return !joints.empty();
    }
};

struct Scene : Property {
    std::string name = "";
    std::vector<int> nodes; // root nodes (not referred by any "children" property)

    JsonObject ToJson() const override
    {
        auto json = JsonObject();
        if (!name.empty())
            json.Property("name", name);
        if (!nodes.empty()) {
            json.Property("nodes", JsonArray().Elements(nodes));
        }
        return json;
    }
    virtual void FromJson(const JsonObject& json) override
    {
        json.GetIfContains("name", name);
        if (json.Contains("nodes")) {
            auto jsonArray = json.Get<JsonArray>("nodes");
            nodes = jsonArray.Get<int>();
        }
    }
    virtual bool Validate() const override
    {
        return true;
    }
};

struct Asset : Property {
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
        generator = json.Get<std::string>("generator");
    }
    virtual bool Validate() const override
    {
        return true;
    }
};

struct GLTF {
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

    std::vector<Animation> animations;
    std::vector<Skin> skins;

    int scene = 0; // scene to display

    template <std::derived_from<Property> T>
    JsonArray ElementsToJsonArray(const std::vector<T>& elements) const
    {
        JsonArray arr;
        for (auto& e : elements) {
            arr.Element(e.Serialize());
        }
        return arr;
    }
    template <std::derived_from<Property> T>
    void ParseJsonArray(std::vector<T>& elements, const JsonArray& arr)
    {
        elements.clear();
        for (size_t i = 0; i < arr.Size(); i++) {
            elements.push_back(T());
            elements.back().Deserialize(arr.Get<JsonObject>(i));
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
        if (!animations.empty())
            json.Property("animations", ElementsToJsonArray(animations));
        if (!skins.empty())
            json.Property("skins", ElementsToJsonArray(skins));

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
        if (json.Contains("animations"))
            gltf.ParseJsonArray(gltf.animations, json.Get<JsonArray>("animations"));
        if (json.Contains("skins"))
            gltf.ParseJsonArray(gltf.skins, json.Get<JsonArray>("skins"));

        gltf.scene = json.Get<int>("scene");
        return gltf;
    }
};

#undef make_opt
}