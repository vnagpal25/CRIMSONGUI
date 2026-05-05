#include <mitkEnumerationPropertySerializer.h>
#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>
#include <mitkProperties.h>

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

namespace mitk
{
class ShaderPropertySerializer : public BasePropertySerializer
{
public:
  mitkClassMacro(ShaderPropertySerializer, BasePropertySerializer)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  BaseProperty::Pointer Deserialize(TiXmlElement* element) override
  {
    return StringProperty::New(element && element->Attribute("value") ? element->Attribute("value") : "").GetPointer();
  }

protected:
  ShaderPropertySerializer() {}
  ~ShaderPropertySerializer() override {}
};
}
MITK_REGISTER_SERIALIZER(ShaderPropertySerializer);

CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(GridRepresentationProperty)
CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(GridVolumeMapperProperty)
