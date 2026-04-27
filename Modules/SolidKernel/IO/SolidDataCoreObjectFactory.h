#pragma once 

#include <string>

#include "mitkCoreObjectFactory.h"
#include "SolidKernelExports.h"

namespace crimson {

class SolidKernel_EXPORT SolidDataCoreObjectFactory : public mitk::CoreObjectFactoryBase
{
public:
    mitkClassMacro(SolidDataCoreObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(SolidDataCoreObjectFactory);

    ~SolidDataCoreObjectFactory();

    mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode* node) override;

    // Deprecated in MITK; signatures match modern CoreObjectFactoryBase.
    std::string GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

protected:
    SolidDataCoreObjectFactory();
};

} // namespace crimson
