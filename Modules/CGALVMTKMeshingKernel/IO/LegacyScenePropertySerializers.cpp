#include <mitkEnumerationPropertySerializer.h>
#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>
#include <mitkStringProperty.h>
#include <tinyxml.h>

#if __has_include(<tinyxml2.h>)
#include <tinyxml2.h>
#define CRIMSON_HAS_TINYXML2 1
#endif

namespace
{
const char* legacyPropertyValue(TiXmlElement* element)
{
  return element ? element->Attribute("value") : nullptr;
}

#ifdef CRIMSON_HAS_TINYXML2
const char* legacyPropertyValue(tinyxml2::XMLElement* element)
{
  return element ? element->Attribute("value") : nullptr;
}

const char* legacyPropertyValue(const tinyxml2::XMLElement* element)
{
  return element ? element->Attribute("value") : nullptr;
}
#endif
}

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
  BaseProperty::Pointer Deserialize(TiXmlElement* element)                           \
  {                                                                                  \
    return DeserializeValue(legacyPropertyValue(element));                           \
  }                                                                                  \
                                                                                     \
  BaseProperty::Pointer DeserializeValue(const char* value)                           \
  {                                                                                  \
    classname::Pointer property = classname::New();                                  \
    property->SetValue(value ? value : "");                                          \
    return property.GetPointer();                                                    \
  }                                                                                  \
                                                                                     \
  /* Newer MITK SceneSerialization builds use TinyXML2 instead of TinyXML. */         \
  CRIMSON_TINYXML2_DESERIALIZE(classname)                                             \
                                                                                     \
protected:                                                                           \
  classname##Serializer() {}                                                         \
  ~classname##Serializer() override {}                                               \
};                                                                                   \
}                                                                                    \
MITK_REGISTER_SERIALIZER(classname##Serializer);

#ifdef CRIMSON_HAS_TINYXML2
#define CRIMSON_TINYXML2_DESERIALIZE(classname)                                      \
  BaseProperty::Pointer Deserialize(tinyxml2::XMLElement* element)                   \
  {                                                                                  \
    return DeserializeValue(legacyPropertyValue(element));                           \
  }                                                                                  \
                                                                                     \
  BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement* element)             \
  {                                                                                  \
    return DeserializeValue(legacyPropertyValue(element));                           \
  }
#else
#define CRIMSON_TINYXML2_DESERIALIZE(classname)
#endif

namespace mitk
{
class ShaderPropertySerializer : public BasePropertySerializer
{
public:
  mitkClassMacro(ShaderPropertySerializer, BasePropertySerializer)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  BaseProperty::Pointer Deserialize(TiXmlElement* element)
  {
    return DeserializeValue(legacyPropertyValue(element));
  }

#ifdef CRIMSON_HAS_TINYXML2
  BaseProperty::Pointer Deserialize(tinyxml2::XMLElement* element)
  {
    return DeserializeValue(legacyPropertyValue(element));
  }

  BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement* element)
  {
    return DeserializeValue(legacyPropertyValue(element));
  }
#endif

  BaseProperty::Pointer DeserializeValue(const char* value)
  {
    return StringProperty::New(value ? value : "").GetPointer();
  }

protected:
  ShaderPropertySerializer() {}
  ~ShaderPropertySerializer() override {}
};
}
MITK_REGISTER_SERIALIZER(ShaderPropertySerializer);

CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(GridRepresentationProperty)
CRIMSON_REGISTER_ENUM_PROPERTY_SERIALIZER(GridVolumeMapperProperty)
