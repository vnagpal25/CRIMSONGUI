#pragma once 

#include "mitkCoreObjectFactory.h"
#include "PCMRIKernelExports.h"

namespace crimson {

class PCMRIKernel_EXPORT PCMRIDataCoreObjectFactory : public mitk::CoreObjectFactoryBase
{
public:
    mitkClassMacro(PCMRIDataCoreObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(PCMRIDataCoreObjectFactory);

    ~PCMRIDataCoreObjectFactory();

    mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode* node) override;

    std::string GetFileExtensions() override { return {}; }
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override { return {}; }
    std::string GetSaveFileExtensions() override { return {}; }
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override { return {}; }

protected:
    PCMRIDataCoreObjectFactory();
};

} // namespace crimson
