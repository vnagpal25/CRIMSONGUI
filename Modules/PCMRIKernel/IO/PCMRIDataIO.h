#pragma once

#include <mitkAbstractFileIO.h>

namespace crimson {

/*! \brief    A class handling IO of PCMRIData. */
class PCMRIDataIO : public mitk::AbstractFileIO {
public:
    PCMRIDataIO();
    virtual ~PCMRIDataIO();

    void Write() override;

protected:
    PCMRIDataIO(const PCMRIDataIO&);
    std::vector<itk::SmartPointer<mitk::BaseData>> DoRead() override;
	AbstractFileIO* IOClone() const override { return new PCMRIDataIO(*this); }

};



} // namespace crimson