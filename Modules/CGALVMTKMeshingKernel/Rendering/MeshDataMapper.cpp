#include "MeshDataMapper.h"

#include "MeshData.h"

namespace crimson
{

MeshDataMapper3D::MeshDataMapper3D() {}

MeshDataMapper3D::~MeshDataMapper3D() {}

const mitk::Surface* MeshDataMapper3D::GetInput()
{
    bool renderMeshData = false;
    GetDataNode()->GetBoolProperty("crimson.renderMeshData", renderMeshData);
    if (!renderMeshData) {
        return nullptr;
    }

    auto mesh = dynamic_cast<MeshData*>(GetDataNode()->GetData());

    if (!mesh) {
        return nullptr;
    }

    return mesh->getSurfaceRepresentation();
}

MeshDataMapper2D::MeshDataMapper2D() {}

MeshDataMapper2D::~MeshDataMapper2D() {}

const mitk::Surface* MeshDataMapper2D::GetInput() const
{
    bool renderMeshData = false;
    GetDataNode()->GetBoolProperty("crimson.renderMeshData", renderMeshData);
    if (!renderMeshData) {
        return nullptr;
    }

    auto brep = dynamic_cast<MeshData*>(GetDataNode()->GetData());

    if (!brep) {
        return nullptr;
    }

    return brep->getSurfaceRepresentation();
}

} // namespace crimson
