#pragma once

#include <algorithm>
#include <itkCreateObjectFunction.h>
#include <mitkIOUtil.h>
#include <mitkBaseDataSerializer.h>
#include <string>

namespace crimson
{
/** Prefer this over itksys::SystemTools::ConvertToOutputPath so VesselTree does not pull itksys into every serializer TU (helps MSVC /MDd link consistency). */
inline std::string CrimsonPathNativeSeparators(std::string path)
{
#ifdef _WIN32
  std::replace(path.begin(), path.end(), '/', '\\');
#endif
  return path;
}
/*!
 * \brief   Simple serializer that uses mitk::IOUtil to write the data to a unique file Don't
 *  forget to call REGISTER_IOUTILDATA_SERIALIZER(YourDataType, YourExtension) in one of source
 *  files.
 *
 * \tparam  BaseDataType    Type of the base data.
 */
template <class BaseDataType> class IOUtilDataSerializer : public mitk::BaseDataSerializer
{
public:
    // TODO: check if class name is important - and if yes - GetStaticNameOfClass() should be defined differently
    typedef IOUtilDataSerializer<BaseDataType> Self;
    typedef mitk::BaseDataSerializer Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    static const char* GetStaticNameOfClass()
    {
        static std::string nameOfClass = std::string(BaseDataType::GetStaticNameOfClass()) + "Serializer";
        return nameOfClass.c_str();
    }
    const char* GetNameOfClass() const override
    {
        MITK_INFO << GetStaticNameOfClass();
        return GetStaticNameOfClass();
    }
    virtual std::vector<std::string> GetClassHierarchy() const override { return mitk::GetClassHierarchy<Self>(); }

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    // MITK BaseDataSerializer API: override Serialize() (single primary filename), not legacy SerializeAll().
    std::string Serialize() override
    {
        auto data = dynamic_cast<const BaseDataType*>(m_Data.GetPointer());
        if (data == NULL) {
            MITK_ERROR << " Object at " << static_cast<const void*>(this->m_Data) << " is not recognized. Cannot serialize as "
                       << BaseDataType::GetStaticNameOfClass() << ".";
            return {};
        }

        std::string baseFileName(this->GetUniqueFilenameInWorkingDirectory());
        baseFileName += "_" + m_FilenameHint + ".";

        std::string fullname(m_WorkingDirectory);
        fullname += "/";
        fullname += CrimsonPathNativeSeparators(baseFileName + extensions[0]);

        try {
            mitk::IOUtil::Save(data, fullname);
        } catch (std::exception& e) {
            MITK_ERROR << " Error serializing object at " << static_cast<const void*>(this->m_Data) << " to " << fullname
                       << ": " << e.what();
            return {};
        }
        return baseFileName + extensions[0];
    }

    void SetParam(const std::vector<std::string>& param) { extensions = param; }
protected:
    IOUtilDataSerializer() {}
    virtual ~IOUtilDataSerializer() {}
    std::vector<std::string> extensions;
};

/*!
 * \brief   A utility subclass of CreateObjectFunctionBase to be used with IOUtilDataSerializer.
 *
 * \tparam  T           Type of the object to be created.
 * \tparam  ParamType   Type of the parameter.
 *                      
 * \see IOUtilDataSerializer
 */
template <typename T, typename ParamType> class CreateObjectFunctionParam : public itk::CreateObjectFunctionBase
{
public:
    /** Standard class typedefs. */
    typedef CreateObjectFunctionParam Self;
    typedef itk::SmartPointer<Self> Pointer;

    /** Methods from itk:LightObject. */
    itkFactorylessNewMacro(Self);
    LightObject::Pointer CreateObject() override
    {
        auto newObject = T::New();
        newObject->SetParam(_param);
        return newObject.GetPointer();
    }

    void SetParam(ParamType param) { _param = param; }

protected:
    CreateObjectFunctionParam() {}
    ~CreateObjectFunctionParam() {}

private:
    CreateObjectFunctionParam(const Self&) = delete;
    void operator=(const Self&) = delete;

    ParamType _param;
};

/*!
 * \brief   A macro that defines a serializer factory.
 *
 * \param   classname   The name of class.
 * \param   ...         List of file extensions.
 */
#define REGISTER_IOUTILDATA_SERIALIZER(classname, ...)                                                                         \
    \
\
namespace crimson                                                                                                              \
    \
{                                                                                                                       \
        \
class classname##IOUtilDataSerializerFactory : public ::itk::ObjectFactoryBase                                                 \
        \
{                                                                                                                   \
        public:                                                                                                                \
            /* ITK typedefs */                                                                                                 \
            typedef classname##IOUtilDataSerializerFactory Self;                                                               \
            typedef itk::ObjectFactoryBase Superclass;                                                                         \
            typedef itk::SmartPointer<Self> Pointer;                                                                           \
            typedef itk::SmartPointer<const Self> ConstPointer;                                                                \
                                                                                                                               \
            /* Methods from ObjectFactoryBase */                                                                               \
            virtual const char* GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }                                     \
                                                                                                                               \
            virtual const char* GetDescription() const { return "Generated factory for " #classname; }                         \
                                                                                                                               \
            /* Method for class instantiation. */                                                                              \
            itkFactorylessNewMacro(Self);                                                                                      \
                                                                                                                               \
            /* Run-time type information (and related methods). */                                                             \
            itkTypeMacro(classname##IOUtilDataSerializerFactory, itkObjectFactoryBase);                                        \
                                                                                                                               \
        protected:                                                                                                             \
            classname##IOUtilDataSerializerFactory()                                                                           \
            {                                                                                                                  \
                auto createObjectFunction = crimson::CreateObjectFunctionParam<crimson::IOUtilDataSerializer<classname>,       \
                                                                               std::vector<std::string>>::New();               \
                createObjectFunction->SetParam(std::vector<std::string>{__VA_ARGS__});                                         \
                itk::ObjectFactoryBase::RegisterOverride(#classname "Serializer", #classname "Serializer",                     \
                                                         "Generated factory for " #classname "Serializer", 1,                  \
                                                         createObjectFunction);                                                \
            }                                                                                                                  \
                                                                                                                               \
            ~classname##IOUtilDataSerializerFactory() {}                                                                       \
                                                                                                                               \
        private:                                                                                                               \
            classname##IOUtilDataSerializerFactory(const Self&) = delete;                                                      \
            void operator=(const Self&) = delete;                                                                              \
        \
};                                                                                                                             \
                                                                                                                               \
        class classname##IOUtilDataSerializerRegistrationMethod                                                                \
        {                                                                                                                      \
        public:                                                                                                                \
            classname##IOUtilDataSerializerRegistrationMethod()                                                                \
            {                                                                                                                  \
                                                                                                                               \
                m_Factory = classname##IOUtilDataSerializerFactory::New();                                                     \
                itk::ObjectFactoryBase::RegisterFactory(m_Factory);                                                            \
            }                                                                                                                  \
                                                                                                                               \
            ~classname##IOUtilDataSerializerRegistrationMethod() { itk::ObjectFactoryBase::UnRegisterFactory(m_Factory); }     \
                                                                                                                               \
        private:                                                                                                               \
            classname##IOUtilDataSerializerFactory::Pointer m_Factory;                                                         \
        };                                                                                                                     \
    \
}                                                                                                                       \
    \
static crimson::classname##IOUtilDataSerializerRegistrationMethod somestaticinitializer_##classname;
}