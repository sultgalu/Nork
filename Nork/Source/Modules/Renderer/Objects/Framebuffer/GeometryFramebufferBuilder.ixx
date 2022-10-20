export module Nork.Renderer:GeometryFramebufferBuilder;

export import :GeometryFramebuffer;
export import :FramebufferBuilder;

export namespace Nork::Renderer {

	class GeometryFramebufferBuilder : FramebufferBuilder
	{
	public:
		GeometryFramebufferBuilder& Width(int width)
		{
			this->width = width;
			return *this;
		}
		GeometryFramebufferBuilder& Height(int height)
		{
			this->height = height;
			return *this;
		}
		GeometryFramebufferBuilder& Depth(TextureFormat depth)
		{
			this->depth = depth;
			return *this;
		}
		GeometryFramebufferBuilder& Position(TextureFormat position)
		{
			this->position = position;
			return *this;
		}
		GeometryFramebufferBuilder& Normal(TextureFormat normal)
		{
			this->normal = normal;
			return *this;
		}
		GeometryFramebufferBuilder& Diffuse(TextureFormat diffuse)
		{
			this->diffuse = diffuse;
			return *this;
		}
		GeometryFramebufferBuilder& Specular(TextureFormat specular)
		{
			this->specular = specular;
			return *this;
		}
		std::shared_ptr<GeometryFramebuffer> Create();
	private:
		void Validate()
		{
			if (depth == TextureFormat::None || position == TextureFormat::None || diffuse == TextureFormat::None ||
				normal == TextureFormat::None || specular == TextureFormat::None)
			{
				std::abort();
			}
		}
		void CreateAttachments();
	private:
		TextureFormat
			depth = TextureFormat::None,
			position = TextureFormat::None,
			diffuse = TextureFormat::None,
			normal = TextureFormat::None,
			specular = TextureFormat::None;
	};
}
