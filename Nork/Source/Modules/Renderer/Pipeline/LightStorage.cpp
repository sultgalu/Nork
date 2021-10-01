#include "pch.h"
#include "LightStorage.h"

namespace Nork::Renderer::Pipeline
{
	void LightStorage::OverwriteAll(std::span<Data::DirLight> dLights, std::span<Data::PointLight> pLights)
	{
		// NINCSENEK RENDEZVE SHADOW SZERINT!!!
		static const int dLightArrLen = 10 * 8;
		static const int dShadArrLen = 5 * 20;
		static const int pLightArrLen = 10 * 12;
		static const int pShadArrLen = 5 * 8;
		int dlIdx = 4, dShadIdx = dlIdx + dLightArrLen;
		int plIdx = dShadIdx + dShadArrLen, pShadIdx = plIdx + pLightArrLen;
		int dLightCount = 0, dShadowCount = 0;
		int pLightCount = 0, pShadowCount = 0;

		float buf[4 + dShadArrLen + dLightArrLen + pLightArrLen + pShadArrLen];

		for (size_t i = 0; i < dLights.size(); i++)
		{
			auto& dl = dLights[i];

			if (dl.shadow.fb != 0)
			{
				auto getVP = dl.GetVP();
				float* vp = (float*)&getVP;
				for (size_t i = 0; i < 16; i++)
					buf[dShadIdx++] = vp[i];

				buf[dShadIdx++] = dl.bias; buf[dShadIdx++] = dl.biasMin; buf[dShadIdx++] = dl.pcfSize; buf[dShadIdx++] = 0;

				DrawdShadow(dl.shadow, mView, getVP);

				Utils::BindTexture(dl.shadow.texture, 10 + dShadowCount);
				dShadowCount++;
			}
			buf[dlIdx++] = dl.direction.x; buf[dlIdx++] = dl.direction.y; buf[dlIdx++] = dl.direction.z; buf[dlIdx++] = 0;
			buf[dlIdx++] = dl.color.r; buf[dlIdx++] = dl.color.g; buf[dlIdx++] = dl.color.b; buf[dlIdx++] = dl.color.a;

			dLightCount++;
		}

		for (size_t i = 0; i < pLights.size(); i++)
		{
			auto& pl = pLights[i];

			if (pl.shadow.fb != 0)
			{
				buf[pShadIdx++] = pl.bias; buf[pShadIdx++] = pl.biasMin; buf[pShadIdx++] = pl.blur; buf[pShadIdx++] = pl.radius;
				buf[pShadIdx++] = pl.far;
				for (int i = 0; i < 3; i++)
					buf[pShadIdx++] = 0;

				DrawpShadow(pl.shadow, mView, tr.position, pl.far, pl.near);

				Utils::BindTextureCube(pl.shadow.texture, 15 + pShadowCount);
				pShadowCount++;
			}
			buf[plIdx++] = tr.position.x; buf[plIdx++] = tr.position.y; buf[plIdx++] = tr.position.z; buf[plIdx++] = 0;
			buf[plIdx++] = pl.color.r; buf[plIdx++] = pl.color.g; buf[plIdx++] = pl.color.b; buf[plIdx++] = pl.color.a;
			buf[plIdx++] = pl.GetLinear(); buf[plIdx++] = pl.GetQuadratic(); buf[plIdx++] = 0; buf[plIdx++] = 0;

			pLightCount++;
		}

		buf[0] = dLightCount;
		buf[1] = dShadowCount;
		buf[2] = pLightCount;
		buf[3] = pShadowCount;

		glBindBuffer(GL_UNIFORM_BUFFER, this->dLightUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(buf), buf);
		//glNamedBufferSubData(this->dLightUBO, 0, sizeof(buf), buf);
		//glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
}