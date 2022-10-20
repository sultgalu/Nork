#pragma once

#include "../Data/Lights.h"
#include "../Pipeline/Light/PointShadowMap.h"
#include "../Pipeline/Light/DirShadowMap.h"

namespace Nork::Renderer {
	struct DirLight: Data::DirLight
	{
		DirLight(std::shared_ptr<Data::DirLight*> ptrRef)
			: ptrRef(ptrRef)
		{
			direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f));
			color = { 1,1,1,1 };
			VP = glm::identity<glm::mat4>();
		}
		void Update()
		{
			**ptrRef = *this;
		}
		std::shared_ptr<Data::DirLight*> ptrRef;
	};
	struct PointLight : Data::PointLight
	{
		PointLight(std::shared_ptr<Data::PointLight*> ptrRef)
			: ptrRef(ptrRef)
		{}
		void Update()
		{
			**ptrRef = *this;
		}
		std::shared_ptr<Data::PointLight*> ptrRef;
	};
	struct DirShadow : Data::DirShadow
	{
		DirShadow(std::shared_ptr<Data::DirShadow*> ptrRef, DirShadowMap shadowMap)
			: ptrRef(ptrRef), shadowMap(shadowMap)
		{
			shadMap = shadowMap.Get()->GetBindlessHandle();
			Update();
		}
		void Update()
		{
			**ptrRef = *this;
		}
		DirShadowMap shadowMap;
		std::shared_ptr<Data::DirShadow*> ptrRef;
	};
	struct PointShadow : Data::PointShadow
	{
		PointShadow (std::shared_ptr<Data::PointShadow*> ptrRef, PointShadowMap shadowMap)
			: ptrRef(ptrRef), shadowMap(shadowMap)
		{
			shadMap = shadowMap.Get()->GetBindlessHandle();
			Update();
		}
		void Update()
		{
			**ptrRef = *this;
		}
		PointShadowMap shadowMap;
		std::shared_ptr<Data::PointShadow*> ptrRef;
	};
}