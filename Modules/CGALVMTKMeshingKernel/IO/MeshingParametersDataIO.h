#pragma once

#include <mitkAbstractFileIO.h>

namespace crimson {

/*! \brief    A class handling IO of MeshingParametersData. */
class MeshingParametersDataIO : public mitk::AbstractFileIO {
public:
    MeshingParametersDataIO();
    virtual ~MeshingParametersDataIO();

    void Write() override;

protected:
    MeshingParametersDataIO(const MeshingParametersDataIO&);
    std::vector<itk::SmartPointer<mitk::BaseData>> DoRead() override;
    AbstractFileIO* IOClone() const override { return new MeshingParametersDataIO(*this); }
};


} // namespace crimson