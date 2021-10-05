#pragma once

namespace Nork::Components
{
	struct Transform
	{
		Transform() : position(glm::vec3(0)), scale(glm::vec3(1)), rotation(glm::vec3(0)) {}
		glm::vec3 position, scale, rotation;
	};

	struct Tag
	{
		std::string tag = "#tbd";
	};

}