#pragma once

#include "CGALVMTKMeshingKernelExports.h"

#include <mitkAbstractFileIO.h>

namespace crimson {

/*! \brief    A class handling IO of MeshData. */
class CGALVMTKMeshingKernel_EXPORT MeshDataIO : public mitk::AbstractFileIO {
public:
    MeshDataIO();
    virtual ~MeshDataIO();

    void Write() override;

protected:
    MeshDataIO(const MeshDataIO&);
    std::vector<itk::SmartPointer<mitk::BaseData>> DoRead() override;
    AbstractFileIO* IOClone() const override { return new MeshDataIO(*this); }
};


} // namespace crimson
