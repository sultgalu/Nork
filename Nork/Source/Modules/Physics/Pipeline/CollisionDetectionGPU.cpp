#include "CollisionDetectionGPU.h"
#include "../Config.h"

namespace Nork::Physics
{
    void glFinishIfEnabled()
    {
        if constexpr (Config::glFinishCollisionDetectionGPU)
        {
            glFinish();
        }
    }

    void CollisionDetectionGPU::ZeroAtomicCounter()
    {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter);
        uint32_t d[2] = { 0, 0 };
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, 2 * sizeof(uint32_t), &d, GL_DYNAMIC_DRAW);
    }
    uint32_t CollisionDetectionGPU::GetAABBResCount()
    {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounter);
        uint32_t d;
        glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t), &d);
        return d;
    }

    CollisionDetectionGPU::CollisionDetectionGPU()
        : shapes(0), models(1), aabbs(2), verts(3), faces(4), aabbRes(5), satRes(6), centers(7), edges(8), vertsIn(9)

    {
        glGenBuffers(1, &atomicCounter);
        ZeroAtomicCounter();
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter);

        auto shaders = Get_Setup_AABB_SAT_Shaders();
        this->setupShader = shaders[0];
        this->aabbShader = shaders[1];
        this->satShader = shaders[2];
    }

    static std::vector<std::pair<std::string, float>> deltas;
    void CollisionDetectionGPU::SetColliders(std::span<Collider> colliders)
    {
        static std::vector<glm::vec4> cpuVerts;
        static std::vector<ShapeGPU> cpuShapes;
        static std::vector<Face> cpuFaces;
        static std::vector<Edge> cpuEdges;

        cpuVerts.clear();
        cpuFaces.clear();
        cpuEdges.clear();
        cpuShapes.resize(colliders.size());

        uint32_t vertCounter = 0;
        uint32_t faceCounter = 0;
        uint32_t edgeCounter = 0;
        for (size_t i = 0; i < colliders.size(); i++)
        {
            cpuShapes[i] = ShapeGPU{
                   .vertStart = vertCounter, .vertCount = (uint32_t)colliders[i].verts.size(),
                   .edgeStart = edgeCounter, .edgeCount = (uint32_t)colliders[i].edges.size(),
                   .faceStart = faceCounter, .faceCount = (uint32_t)colliders[i].faces.size(),
            };

            cpuVerts.insert(cpuVerts.end(), colliders[i].verts.begin(), colliders[i].verts.end());
            cpuFaces.insert(cpuFaces.end(), colliders[i].faces.begin(), colliders[i].faces.end());
            cpuEdges.insert(cpuEdges.end(), colliders[i].edges.begin(), colliders[i].edges.end());

            for (size_t i = faceCounter; i < cpuFaces.size(); i++)
            {
                cpuFaces[i].vertIdx += vertCounter; // by default it is local
            }

            for (size_t i = edgeCounter; i < cpuEdges.size(); i++)
            {
                cpuEdges[i][0] += vertCounter; // by default it is local
                cpuEdges[i][1] += vertCounter; // by default it is local
            }

            vertCounter += colliders[i].verts.size();
            faceCounter += colliders[i].faces.size();
            edgeCounter += colliders[i].edges.size();
        }

        aabbs.SetSize(colliders.size() * sizeof(AABBGPU));
        centers.SetSize(colliders.size() * sizeof(glm::vec4));
        aabbRes.SetSize(std::pow(colliders.size(), 2) * sizeof(glm::uvec2) / 2);
        satRes.SetSize(std::pow(colliders.size(), 2) * sizeof(CollisionResult) / 2);
        verts.SetSize(cpuVerts.size() * sizeof(glm::vec4));

        vertsIn.Data(cpuVerts);
        faces.Data(cpuFaces);
        edges.Data(cpuEdges);
        shapes.Data(cpuShapes);

        shapeCount = colliders.size();
    }
    void CollisionDetectionGPU::UpdateTransforms(std::span<glm::vec3> translate, std::span<glm::quat> quaternions)
    {
        modelCache.resize(translate.size());
        for (size_t i = 0; i < modelCache.size(); i++)
        {
            modelCache[i] = glm::translate(glm::identity<glm::mat4>(), translate[i]) * glm::mat4_cast(quaternions[i]);
        }
        SetupPhase(modelCache);
    }
    void CollisionDetectionGPU::SetupPhase(std::span<glm::mat4> models)
    {
        deltas.clear();
        Timer t;
        glFinishIfEnabled();
        deltas.push_back(std::pair("GLFinish initial", t.Reset()));

        this->models.Data(models);

        glFinishIfEnabled();
        deltas.push_back(std::pair("Upload Data", t.Reset()));
        
        ZeroAtomicCounter();
        glFinishIfEnabled();
        deltas.push_back(std::pair("Zero Atomic", t.Reset()));

        glUseProgram(setupShader);
        glDispatchCompute(shapeCount, 1, 1);
        glFinishIfEnabled();
        deltas.push_back(std::pair("SetupShader", t.Reset()));
    }
    void CollisionDetectionGPU::BroadPhase()
    {
        Timer t;

        glUseProgram(aabbShader);
        uint32_t y = shapeCount / 1024 + 1;
        glDispatchCompute(shapeCount, y, 1);
        glFinishIfEnabled();
        deltas.push_back(std::pair("AABBShader", t.Reset()));
    }
    void CollisionDetectionGPU::NarrowPhase()
    {
        Timer t;

        uint32_t aabbResCount = GetAABBResCount();
        deltas.push_back(std::pair("GetCounter", t.Reset()));

        glUseProgram(satShader);
        glDispatchCompute(aabbResCount, 1, 1);
        glFinishIfEnabled();
        deltas.push_back(std::pair("SatShader", t.Reset()));

        results.resize(aabbResCount);
        satRes.GetData(std::span(results));
        deltas.push_back(std::pair("Read results", t.Reset()));
    }
    std::vector<std::pair<std::string, float>> CollisionDetectionGPU::GetDeltas()
    {
        return deltas;
    }
}