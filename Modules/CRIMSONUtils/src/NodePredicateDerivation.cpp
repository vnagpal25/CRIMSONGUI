#include <NodePredicateDerivation.h>

namespace crimson
{

bool NodePredicateDerivation::CheckNode(const mitk::DataNode* node) const
{
    // mitk::WeakPointer has no operator->; Lock() yields itk::SmartPointer<T> (or nullptr).
    const auto storage = m_DataStorage.Lock();
    const auto parent = m_ParentNode.Lock();
    if (storage && parent) {
        const mitk::DataStorage::SetOfObjects::STLContainerType children =
            storage->GetDerivations(parent, nullptr, !m_SearchAllDerivations)->CastToSTLConstContainer();

        return std::find(children.begin(), children.end(), node) != children.end();
    }

    return false;
}

NodePredicateDerivation::NodePredicateDerivation(mitk::DataNode* n, bool allderiv, mitk::DataStorage* ds)
    : mitk::NodePredicateBase()
    , m_ParentNode(n)
    , m_SearchAllDerivations(allderiv)
    , m_DataStorage(ds)
{
}
}
