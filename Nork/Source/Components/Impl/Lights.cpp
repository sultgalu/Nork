#include "../Lights.h"

namespace Nork::Components {
	inline static constexpr auto ranges = std::array<uint32_t, 14>({ 0, 7, 13, 20, 32, 50, 65, 100, 160, 200, 325, 600, 3250, std::numeric_limits<uint32_t>::max() });
	inline static constexpr auto linears = std::array<float, 14>({ 10, 0.7f, 0.35f, 0.22f, 0.14f, 0.09f, 0.07f, 0.045f, 0.027f, 0.022f, 0.014f, 0.007f, 0.0014f, std::numeric_limits<float>::max() / 2 });
	inline static constexpr auto quadratics = std::array<float, 14>({ 20, 1.8f, 0.44f, 0.20f, 0.07f, 0.032f, 0.017f, 0.0075f, 0.0028f, 0.0019f, 0.0007f, 0.0002f, 0.000007f, std::numeric_limits<float>::max() });

	void PointLight::SetIntensity(uint32_t val)
	{
		for (size_t i = 0; i < ranges.size(); i++)
		{
			if (val <= ranges[i])
			{
				if (val == ranges[i])
				{
					// light->linear = linears[i];
					// light->quadratic = quadratics[i];
				}
				else
				{
					float range = ranges[i] - ranges[i - 1];
					float portion = (val - ranges[i - 1]) / range;

					float linearRange = linears[i - 1] - linears[i];
					float quadraticRange = quadratics[i - 1] - quadratics[i];

					// light->quadratic = quadratics[i - 1] - portion * quadraticRange;
					// light->linear = linears[i - 1] - portion * linearRange;
				}
				break;
			}
		}
		intensity = val;
	}
	glm::mat4 DirLight::GetView() const
	{
		return glm::lookAt(glm::vec3(0) - Data()->direction, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));
		//return glm::lookAt(position, position + Data()->direction, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	void DirLight::RecalcVP()
	{
		auto right = position.x + rectangle.x / 2;
		auto left = position.x - rectangle.x / 2;
		auto top = position.y + rectangle.y / 2;
		auto bottom = position.y - rectangle.y / 2;
		auto near = position.z;
		auto far = position.z + rectangle.z;
		auto projection = glm::ortho(left, right, bottom, top, near, far);
		projection[1][1] *= -1; // flip for VULKAN Screen Space
		Data()->VP = projection * GetView();
	}
}