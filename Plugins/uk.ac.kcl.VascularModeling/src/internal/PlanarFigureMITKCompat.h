#pragma once

// MITK (Qt6 branch): PlanarFigure::IsFinalized / SetFinalized were removed. The same UI/tool
// semantics are expressed with the "planarfigure.iseditable" property (non-editable ≈ finalized).

#include <mitkPlanarFigure.h>
#include <mitkProperties.h>

namespace crimson
{

inline void planarFigureSetFinalized(mitk::PlanarFigure* pf, bool finalized)
{
  if (pf)
    pf->SetProperty("planarfigure.iseditable", mitk::BoolProperty::New(!finalized));
}

inline bool planarFigureIsFinalized(const mitk::PlanarFigure* pf)
{
  if (!pf || !pf->IsPlaced())
    return false;
  bool editable = true;
  pf->GetPropertyList()->GetBoolProperty("planarfigure.iseditable", editable);
  return !editable;
}

} // namespace crimson
