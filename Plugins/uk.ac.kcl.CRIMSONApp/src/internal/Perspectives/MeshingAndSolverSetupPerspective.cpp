#include "MeshingAndSolverSetupPerspective.h"

#include <berryIViewLayout.h>
#include <berryIViewRegistry.h>
#include <berryPlatformUI.h>

namespace crimson {

namespace {

bool viewExists(const char* viewId)
{
    return berry::PlatformUI::GetWorkbench()->GetViewRegistry()->Find(QString::fromLatin1(viewId)).IsNotNull();
}

}

void MeshingAndSolverSetupPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
    QString editorArea = layout->GetEditorArea();

    layout->AddView("org.mitk.views.datamanager",
        berry::IPageLayout::LEFT, 0.2f, editorArea);

    berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
    lo->SetCloseable(false);

    layout->AddView("org.mitk.views.imagenavigator",
        berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");

    layout->AddView("org.mitk.views.vesselmeshingview",
        berry::IPageLayout::RIGHT, 0.7f, editorArea);
		
    if (viewExists("org.mitk.views.SolverSetupView")) {
        auto folder = static_cast<berry::IFolderLayout*>(layout->GetFolderForView("org.mitk.views.vesselmeshingview").GetPointer());
        folder->AddView("org.mitk.views.SolverSetupView");
    }
	
    if (viewExists("org.mitk.views.ResliceView")) {
        layout->AddView("org.mitk.views.ResliceView",
            berry::IPageLayout::TOP, 0.3f, editorArea);
    }
}

} // namespace crimson
