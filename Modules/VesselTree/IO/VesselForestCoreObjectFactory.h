#pragma once

#include <string>

#include "mitkCoreObjectFactory.h"
#include "VesselTreeExports.h"

namespace crimson {

class VesselTree_EXPORT VesselForestCoreObjectFactory : public mitk::CoreObjectFactoryBase
{
public:
    mitkClassMacro(VesselForestCoreObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(VesselForestCoreObjectFactory);

    ~VesselForestCoreObjectFactory();

    virtual mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);

    // Deprecated in MITK; signatures use std::string since modern CoreObjectFactoryBase.
    std::string GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

protected:
    VesselForestCoreObjectFactory();
};

}