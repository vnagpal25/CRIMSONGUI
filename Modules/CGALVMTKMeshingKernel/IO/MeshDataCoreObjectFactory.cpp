#include "MeshDataCoreObjectFactory.h"

#include "mitkProperties.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"

#include "MeshData.h"
#include "MeshDataMapper.h"

#include <cstdlib>

typedef std::multimap<std::string, std::string> MultimapType;

namespace crimson {

MeshDataCoreObjectFactory::MeshDataCoreObjectFactory()
: CoreObjectFactoryBase()
{
    static bool alreadyDone = false;
    if (!alreadyDone)
    {
        mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(this);
    }

}

MeshDataCoreObjectFactory::~MeshDataCoreObjectFactory()
{
}

mitk::Mapper::Pointer MeshDataCoreObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
    if (!node->GetData()) {
        return nullptr;
    }

    if (!std::getenv("CRIMSON_ENABLE_CUSTOM_MAPPERS")) {
        return nullptr;
    }

    if (auto mesh = dynamic_cast<MeshData*>(node->GetData())) {
        if (mesh->isLegacyPayloadSkipped()) {
            return nullptr;
        }

        bool renderMeshData = false;
        node->GetBoolProperty("crimson.renderMeshData", renderMeshData);
        if (!renderMeshData) {
            return nullptr;
        }

        if (id == mitk::BaseRenderer::Standard3D) {
            auto mapper = MeshDataMapper3D::New();
            mapper->SetDataNode(node);
            return mapper.GetPointer();
        }
        else if (id == mitk::BaseRenderer::Standard2D) {
            auto mapper = MeshDataMapper2D::New();
            mapper->SetDataNode(node);
            return mapper.GetPointer();
        }
        
    }
    return nullptr;
}

void MeshDataCoreObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
    if (node == nullptr) {
        return;
    }

    if (auto mesh = dynamic_cast<MeshData*>(node->GetData())) {
        MeshDataMapper2D::SetDefaultProperties(node);
        MeshDataMapper3D::SetDefaultProperties(node);
        node->AddProperty("crimson.renderMeshData", mitk::BoolProperty::New(false), nullptr, false);
        node->AddProperty("visible", mitk::BoolProperty::New(false), nullptr, false);
        if (mesh->isLegacyPayloadSkipped()) {
            node->SetBoolProperty("crimson.renderMeshData", false);
            node->SetVisibility(false);
        }
    }
}

struct RegisterMeshDataCoreObjectFactory {
    RegisterMeshDataCoreObjectFactory()
        : m_Factory(MeshDataCoreObjectFactory::New())
    {
        mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
    }

    ~RegisterMeshDataCoreObjectFactory()
    {
        mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory);
    }

    MeshDataCoreObjectFactory::Pointer m_Factory;
};

static RegisterMeshDataCoreObjectFactory registerMeshCoreObjectFactory;

} // namespace crimson
