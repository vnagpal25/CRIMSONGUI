#include <mitkEnumerationPropertySerializer.h>
#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>
#include <mitkShaderProperty.h>

#define CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(classname)                         \
namespace mitk                                                                       \
{                                                                                    \
class classname##Serializer : public EnumerationPropertySerializer                   \
{                                                                                    \
public:                                                                              \
  mitkClassMacro(classname##Serializer, EnumerationPropertySerializer)               \
  itkFactorylessNewMacro(Self)                                                       \
  itkCloneMacro(Self)                                                                \
                                                                                     \
  BaseProperty::Pointer Deserialize(TiXmlElement* element) override                  \
  {                                                                                  \
    if (!element)                                                                    \
      return nullptr;                                                                \
                                                                                     \
    const char* value = element->Attribute("value");                                 \
    classname::Pointer property = classname::New();                                  \
    property->SetValue(value ? value : "");                                          \
    return property.GetPointer();                                                    \
  }                                                                                  \
                                                                                     \
protected:                                                                           \
  classname##Serializer() {}                                                         \
  ~classname##Serializer() override {}                                               \
};                                                                                   \
}                                                                                    \
MITK_REGISTER_SERIALIZER(classname##Serializer);

CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(ShaderProperty)
CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(GridRepresentationProperty)
CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(GridVolumeMapperProperty)
