#include <boost/algorithm/string.hpp>

#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include "MeshDataIO.h"

#include <MeshData.h>
#include <IO/MeshingKernelIOMimeTypes.h>

#include <IO/IOUtilDataSerializer.h>

#include <FaceIdentifierBoostIO.h>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>


REGISTER_IOUTILDATA_SERIALIZER(MeshData, 
    crimson::MeshingKernelIOMimeTypes::MESHDATA_DEFAULT_EXTENSION(),
    crimson::MeshingKernelIOMimeTypes::MESHDATA_DEFAULT_EXTENSION() + ".vtu",
    crimson::MeshingKernelIOMimeTypes::MESHDATA_DEFAULT_EXTENSION() + ".vtp",
    crimson::MeshingKernelIOMimeTypes::MESHDATA_DEFAULT_EXTENSION() + ".faceinfo"
    )

namespace crimson {

static void sanitizeSurfaceForRendering(vtkPolyData* surface)
{
    if (!surface) {
        return;
    }

    surface->GetPointData()->Initialize();
    surface->GetCellData()->Initialize();
    surface->BuildCells();
    surface->BuildLinks();
}

MeshDataIO::MeshDataIO()
    : AbstractFileIO(MeshData::GetStaticNameOfClass(),
    MeshingKernelIOMimeTypes::MESHDATA_MIMETYPE(),
    " mesh data")
{
    RegisterService();
}

MeshDataIO::MeshDataIO(const MeshDataIO& other)
    : AbstractFileIO(other)
{
}

MeshDataIO::~MeshDataIO()
{
}

std::vector<itk::SmartPointer<mitk::BaseData>> MeshDataIO::DoRead()
{
    std::vector< itk::SmartPointer<mitk::BaseData> > result;

    auto mesh = MeshData::New();
    MITK_INFO << "Loading CRIMSON mesh data from " << GetLocalFileName();

    std::ifstream faceInfoFile(GetLocalFileName() + ".faceinfo");

    if (faceInfoFile.good()) {
        // Check if new version with boost serialization
        boost::archive::xml_iarchive inArchive(faceInfoFile);

        auto& dataRef = mesh->_faceIdentifierMap;
        inArchive >> BOOST_SERIALIZATION_NVP(dataRef);
    } else {
        MITK_WARN << "Failed to load the  model face information - please re-mesh for correct solver setup output";
    }

    vtkNew<vtkXMLUnstructuredGridReader> unstructuredGridReader;
    unstructuredGridReader->SetFileName((GetLocalFileName() + ".vtu").c_str());

    if (unstructuredGridReader->CanReadFile((GetLocalFileName() + ".vtu").c_str())) {
        unstructuredGridReader->Update();
        vtkNew<vtkUnstructuredGrid> unstructuredGrid;
        unstructuredGrid->DeepCopy(unstructuredGridReader->GetOutput());

        auto ug = mitk::UnstructuredGrid::New();
        ug->SetVtkUnstructuredGrid(unstructuredGrid.GetPointer());
        mesh->setUnstructuredGrid(ug, false);
        MITK_INFO << "Loaded CRIMSON mesh: " << unstructuredGrid->GetNumberOfPoints() << " points, "
                  << unstructuredGrid->GetNumberOfCells() << " cells";
    } else {
        mitkThrow() << "Failed to read mesh from " << GetLocalFileName();
    }

    vtkNew<vtkXMLPolyDataReader> surfaceReader;
    surfaceReader->SetFileName((GetLocalFileName() + ".vtp").c_str());

    if (surfaceReader->CanReadFile((GetLocalFileName() + ".vtp").c_str())) {
        surfaceReader->Update();
        vtkNew<vtkPolyData> surface;
        surface->DeepCopy(surfaceReader->GetOutput());
        sanitizeSurfaceForRendering(surface.GetPointer());

        mesh->_surfaceRepresentation = mitk::Surface::New();
        mesh->_surfaceRepresentation->SetVtkPolyData(surface.GetPointer());

        MITK_INFO << "Loaded CRIMSON mesh surface: " << surface->GetNumberOfPoints() << " points, "
                  << surface->GetNumberOfCells() << " cells";
    } else {
        MITK_WARN << "No CRIMSON mesh surface sidecar found; surface will be extracted on demand";
    }

    result.push_back(mesh.GetPointer());
    return result;
}

void MeshDataIO::Write()
{
    auto mesh = static_cast<const MeshData*>(this->GetInput());

    if (!mesh) {
        mitkThrow() << "Input  mesh data has not been set!";
    }

    // Write 'tag' file
    std::ofstream cmsFile(GetOutputLocation());
    cmsFile << "This is a stub file for CRIMSON meshes";

    // Save face information
    std::ofstream faceInfoFile(GetOutputLocation() + ".faceinfo");
    boost::archive::xml_oarchive outArchive(faceInfoFile);

    auto& dataRef = mesh->_faceIdentifierMap;
    outArchive << BOOST_SERIALIZATION_NVP(dataRef);

    // Save unstructured grid representation
    vtkNew<vtkXMLUnstructuredGridWriter> ugWriter;
    ugWriter->SetDataModeToBinary();
    ugWriter->SetInputData(mesh->getUnstructuredGridRepresentation()->GetVtkUnstructuredGrid());
    ugWriter->SetFileName((GetOutputLocation() + ".vtu").c_str());
    ugWriter->Update();

    vtkNew<vtkXMLPolyDataWriter> pdWriter;
    pdWriter->SetDataModeToBinary();
    pdWriter->SetInputData(mesh->getSurfaceRepresentation()->GetVtkPolyData());
    pdWriter->SetFileName((GetOutputLocation() + ".vtp").c_str());
    pdWriter->Update();
}

}
