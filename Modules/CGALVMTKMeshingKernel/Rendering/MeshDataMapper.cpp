#include "MeshDataMapper.h"

#include "MeshData.h"

#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

namespace crimson
{

namespace
{
const mitk::Surface* getEmptyMapperSurface()
{
    static mitk::Surface::Pointer emptySurface = [] {
        vtkNew<vtkPolyData> polyData;
        vtkNew<vtkPoints> points;
        vtkNew<vtkCellArray> emptyVerts;
        vtkNew<vtkCellArray> emptyLines;
        vtkNew<vtkCellArray> emptyPolys;
        vtkNew<vtkCellArray> emptyStrips;
        polyData->SetPoints(points.GetPointer());
        polyData->SetVerts(emptyVerts.GetPointer());
        polyData->SetLines(emptyLines.GetPointer());
        polyData->SetPolys(emptyPolys.GetPointer());
        polyData->SetStrips(emptyStrips.GetPointer());

        auto surface = mitk::Surface::New();
        surface->SetVtkPolyData(polyData.GetPointer());
        return surface;
    }();

    return emptySurface.GetPointer();
}
}

MeshDataMapper3D::MeshDataMapper3D() {}

MeshDataMapper3D::~MeshDataMapper3D() {}

const mitk::Surface* MeshDataMapper3D::GetInput()
{
    bool renderMeshData = false;
    GetDataNode()->GetBoolProperty("crimson.renderMeshData", renderMeshData);
    if (!renderMeshData) {
        return getEmptyMapperSurface();
    }

    auto mesh = dynamic_cast<MeshData*>(GetDataNode()->GetData());

    if (!mesh) {
        return getEmptyMapperSurface();
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
        return getEmptyMapperSurface();
    }

    auto brep = dynamic_cast<MeshData*>(GetDataNode()->GetData());

    if (!brep) {
        return getEmptyMapperSurface();
    }

    return brep->getSurfaceRepresentation();
}

} // namespace crimson
