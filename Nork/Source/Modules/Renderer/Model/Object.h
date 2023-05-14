#pragma once

#include "Material.h"
#include "Mesh.h"

namespace Nork::Renderer {
struct ToBeDrawn {
    std::shared_ptr<MeshData> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<BufferElement<glm::mat4>> modelMatrix;
    ShadingMode shadingMode;
};
}