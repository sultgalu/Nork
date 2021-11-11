#include "GPUPipeline.h"

namespace Nork::Physics
{
	

	/*GPUPipeline::GPUPipeline(GLuint setupShader, GLuint aabbShader, GLuint satShader)
		: shapes(0), models(1), aabbs(2), verts(3), faces(4), aabbRes(5), satRes(6), centers(7), edges(8), vertsIn(9)
	{
		glGenBuffers(1, &atomicCounter);
		ZeroAtomicCounters();
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter);

		this->setupShader = setupShader;
		this->aabbShader = aabbShader;
		this->satShader = satShader;
	}*/
	//void GPUPipeline::SetModels(std::span<glm::mat4> models)
	//{
	//	this->models.Data(models);
	//}
	//void GPUPipeline::SetColliders(std::span<Compute::Collider> colliders)
	//{
	//	static std::vector<glm::vec4> cpuVerts;
	//	static std::vector<Compute::Shape> cpuShapes;
	//	static std::vector<Compute::Face> cpuFaces;
	//	static std::vector<std::array<uint32_t, 2>> cpuEdges;

	//	cpuVerts.clear();
	//	cpuFaces.clear();
	//	cpuEdges.clear();
	//	cpuShapes.resize(colliders.size());

	//	uint32_t vertCounter = 0;
	//	uint32_t faceCounter = 0;
	//	uint32_t edgeCounter = 0;
	//	for (size_t i = 0; i < colliders.size(); i++)
	//	{
	//		cpuShapes[i] = Compute::Shape{
	//			   .vertStart = vertCounter, .vertCount = (uint32_t)colliders[i].verts.size(),
	//			   .edgeStart = edgeCounter, .edgeCount = (uint32_t)colliders[i].edges.size(),
	//			   .faceStart = faceCounter, .faceCount = (uint32_t)colliders[i].faces.size(),
	//		};

	//		cpuVerts.insert(cpuVerts.end(), colliders[i].verts.begin(), colliders[i].verts.end());
	//		cpuFaces.insert(cpuFaces.end(), colliders[i].faces.begin(), colliders[i].faces.end());
	//		cpuEdges.insert(cpuEdges.end(), colliders[i].edges.begin(), colliders[i].edges.end());

	//		for (size_t i = faceCounter; i < cpuFaces.size(); i++)
	//		{
	//			cpuFaces[i].vertIdx += vertCounter; // by default it is local
	//		}

	//		for (size_t i = edgeCounter; i < cpuEdges.size(); i++)
	//		{
	//			cpuEdges[i][0] += vertCounter; // by default it is local
	//			cpuEdges[i][1] += vertCounter; // by default it is local
	//		}

	//		vertCounter += colliders[i].verts.size();
	//		faceCounter += colliders[i].faces.size();
	//		edgeCounter += colliders[i].edges.size();
	//	}

	//	aabbs.SetSize(colliders.size() * sizeof(Compute::AABB));
	//	centers.SetSize(colliders.size() * sizeof(glm::vec4));
	//	aabbRes.SetSize(std::pow(colliders.size(), 2) * sizeof(glm::uvec2) / 2);
	//	satRes.SetSize(std::pow(colliders.size(), 2) * sizeof(Compute::Result) / 2);
	//	verts.SetSize(cpuVerts.size() * sizeof(glm::vec4));

	//	vertsIn.Data(cpuVerts);
	//	faces.Data(cpuFaces);
	//	edges.Data(cpuEdges);
	//	shapes.Data(cpuShapes);

	//	shapeCount = colliders.size();
	//}

	/*static std::vector<std::pair<std::string, float>> deltas;
	std::vector<std::pair<std::string, float>> GPUPipeline::GetDeltas()
	{
		return deltas;
	}
	std::vector<CollisionResult>& GPUPipeline::Execute()
	{
		Timer t;
		Timer tWhole;
		deltas.clear();
		glFinish(); 
		deltas.push_back(std::pair("GLFinish initial", t.Reset()));

		ZeroAtomicCounters();
		glFinish();
		deltas.push_back(std::pair("Zero Atomic", t.Reset()));

		glUseProgram(setupShader);
		glDispatchCompute(shapeCount, 1, 1);
		glFinish();
		deltas.push_back(std::pair("SetupShader", t.Reset()));

		glUseProgram(aabbShader);
		uint32_t y = shapeCount / 1024 + 1;
		glDispatchCompute(shapeCount, y, 1);
		glFinish();
		deltas.push_back(std::pair("AABBShader", t.Reset()));
		uint32_t aabbResCount = GetAtomicCounter();
		deltas.push_back(std::pair("GetCounter", t.Reset()));

		glUseProgram(satShader);
		glDispatchCompute(aabbResCount, 1, 1);
		glFinish();
		deltas.push_back(std::pair("SatShader", t.Reset()));

		static std::vector<CollisionResult> satResults;
		satResults.resize(aabbResCount);
		satRes.GetData(std::span(satResults));
		deltas.push_back(std::pair("Read results", t.Reset()));
		deltas.push_back(std::pair("WHOLE AABBEXEC", tWhole.Reset()));
		return satResults;
	}*/
}