#include "../Utils.h"

namespace Nork::Renderer::Utils::Draw
{
	unsigned int quadVao, quadVb, quadIb;
	void Quad()
	{
		static bool initialized = false;
		if (!initialized)
		{
			float verts[]{
				   -1.0f, -1.0f, 0.0f, 0.0f,
					1.0f, -1.0f, 1.0f, 0.0f,
					1.0f,  1.0f, 1.0f, 1.0f,
				   -1.0f,  1.0f, 0.0f, 1.0f,
			};

			unsigned int indices[]{
				0, 1, 3,
				3, 1, 2,
			};

			quadVao = VAO::Builder()
				.AddBuffer(&quadVb, GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(verts), verts)
				.SetAttribs(std::vector<int> {2, 2})
				.AddBuffer(&quadIb, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices)
				.GetVertexArrayBuffer();
			initialized = true;
		}
		glBindVertexArray(quadVao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	unsigned int cubeVao, cubeVb, cubeIb;
	void Cubemap()
	{
		static bool initialized = false;
		if (!initialized)
		{
			float verts[]{
			   -1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				1.0f,  1.0f, -1.0f,
			   -1.0f,  1.0f, -1.0f,

			   -1.0f, -1.0f,  1.0f,
				1.0f, -1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
			   -1.0f,  1.0f,  1.0f,
			};
			unsigned int indices[36 * 1] = {};
			unsigned int offset = 0, i = 0;

			{
				//CLOCK-WISE

				// Face #1
				indices[i++] = offset + 0;
				indices[i++] = offset + 1;
				indices[i++] = offset + 2;

				indices[i++] = offset + 2;
				indices[i++] = offset + 3;
				indices[i++] = offset + 0;

				// Face #2
				indices[i++] = offset + 2;
				indices[i++] = offset + 1;
				indices[i++] = offset + 5;

				indices[i++] = offset + 5;
				indices[i++] = offset + 6;
				indices[i++] = offset + 2;

				// Face #3
				indices[i++] = offset + 4;
				indices[i++] = offset + 0;
				indices[i++] = offset + 3;

				indices[i++] = offset + 3;
				indices[i++] = offset + 7;
				indices[i++] = offset + 4;

				// Face #4
				indices[i++] = offset + 1;
				indices[i++] = offset + 0;
				indices[i++] = offset + 4;

				indices[i++] = offset + 4;
				indices[i++] = offset + 5;
				indices[i++] = offset + 1;

				// Face #5
				indices[i++] = offset + 7;
				indices[i++] = offset + 3;
				indices[i++] = offset + 2;

				indices[i++] = offset + 2;
				indices[i++] = offset + 6;
				indices[i++] = offset + 7;

				// Face #6
				indices[i++] = offset + 6;
				indices[i++] = offset + 5;
				indices[i++] = offset + 4;

				indices[i++] = offset + 4;
				indices[i++] = offset + 7;
				indices[i++] = offset + 6;
			}

			cubeVao = VAO::Builder()
				.AddBuffer(&cubeVb, GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(verts), verts)
				.SetAttribs(std::vector<int> {3})
				.AddBuffer(&cubeIb, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices)
				.GetVertexArrayBuffer();
			initialized = true;
		}

		glBindVertexArray(cubeVao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}