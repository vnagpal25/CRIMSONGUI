#pragma once

#include <mitkAbstractFileIO.h>

#include <TopoDS_Shape.hxx>

namespace crimson {

/*! \brief    A class handling IO of OCCBRepData. */
class OCCBRepDataIO : public mitk::AbstractFileIO {
public:
    OCCBRepDataIO();
    virtual ~OCCBRepDataIO();
    void Write() override;

protected:
    OCCBRepDataIO(const OCCBRepDataIO&);
    std::vector<itk::SmartPointer<mitk::BaseData>> DoRead() override;
    AbstractFileIO* IOClone() const override { return new OCCBRepDataIO(*this); }

    TopoDS_Shape trySewImportedShape(const TopoDS_Shape& importedShape);
};


} // namespace crimson