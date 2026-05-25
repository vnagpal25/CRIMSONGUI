// Blueberry
#include "VesselDrivenResliceView.h"
#include "VesselDrivenSlicedGeometry.h"
#include "VesselPathInteractor.h"
#include "VascularModelingNodeTypes.h"

#include <vtkParametricSplineVesselPathData.h>
#include <vtkParametricSplineVesselPathVtkMapper3D.h>
#include <SolidData.h>
#include <SolidDataMapper.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkNodePredicateDataType.h>

#include <mitkSlicedGeometry3D.h>
#include <mitkImage.h>
#include <mitkProperties.h>
#include <mitkMapper.h>
#include <mitkPlanarFigure.h>
#include <mitkAnatomicalPlanes.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkVtkPropRenderer.h>
#include <mitkLogMacros.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkPropCollection.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>

// Qt
#include "QmitkRenderWindow.h"
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QToolButton>

#include <ctkSliderWidget.h>
#include <ctkPopupWidget.h>
#include <usModuleRegistry.h>

#include "ui_ReslicePropertiesWidget.h"
#include "VascularModelingUtils.h"

#include <berryIPartListener.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

#include <algorithm>
#include <vector>

namespace {

const char* yesNo(bool value)
{
    return value ? "yes" : "no";
}

const char* nodeDataClassName(mitk::DataNode* node)
{
    if (!node || !node->GetData()) {
        return "none";
    }

    return node->GetData()->GetNameOfClass();
}

void logSetupCheckpoint(const char* step)
{
    MITK_WARN << "VesselDrivenResliceView RESLICE_TRACE_V3 setup checkpoint: " << step;
}

void forceImmediateMitkRender(QmitkRenderWindow* renderWindow)
{
    if (!renderWindow || !renderWindow->GetVtkRenderWindow()) {
        return;
    }

    int* size = renderWindow->GetVtkRenderWindow()->GetSize();
    if (!size || size[0] == 0 || size[1] == 0) {
        return;
    }

    auto renderer = dynamic_cast<mitk::VtkPropRenderer*>(renderWindow->GetRenderer());
    if (!renderer) {
        return;
    }

    renderer->ForceImmediateUpdate();
}

void configureImageNodeForReslice(mitk::DataNode* imageNode, QmitkRenderWindow* renderWindow, bool showGradientMagnitude)
{
    if (!imageNode || !renderWindow || !renderWindow->GetRenderer()) {
        return;
    }

    auto renderer = renderWindow->GetRenderer();
    imageNode->SetBoolProperty("in plane resample extent by geometry", true, renderer);
    imageNode->AddProperty("reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_CUBIC), renderer, true);
    imageNode->SetBoolProperty("show gradient magnitude", false);
    imageNode->GetPropertyList(renderer)->SetBoolProperty("show gradient magnitude", showGradientMagnitude);
    imageNode->SetVisibility(true, renderer);
}

void configureOverlayNodeForReslice(mitk::DataNode* node, QmitkRenderWindow* renderWindow, bool visible)
{
    if (!node || !renderWindow || !renderWindow->GetRenderer()) {
        return;
    }

    node->SetVisibility(visible, renderWindow->GetRenderer());
}

void ensureVesselPathMapper(mitk::DataNode* node)
{
    if (!node || !dynamic_cast<crimson::vtkParametricSplineVesselPathData*>(node->GetData())) {
        return;
    }

    if (dynamic_cast<crimson::vtkParametricSplineVesselPathVtkMapper3D*>(node->GetMapper(mitk::BaseRenderer::Standard2D))) {
        return;
    }

    auto mapper = crimson::vtkParametricSplineVesselPathVtkMapper3D::New();
    mapper->SetDataNode(node);
    node->SetMapper(mitk::BaseRenderer::Standard2D, mapper.GetPointer());
}

void ensureSolidDataMapper(mitk::DataNode* node)
{
    if (!node || !dynamic_cast<crimson::SolidData*>(node->GetData())) {
        return;
    }

    if (dynamic_cast<crimson::SolidDataMapper2D*>(node->GetMapper(mitk::BaseRenderer::Standard2D))) {
        return;
    }

    auto mapper = crimson::SolidDataMapper2D::New();
    mapper->SetDataNode(node);
    node->SetMapper(mitk::BaseRenderer::Standard2D, mapper.GetPointer());
}

const char* mapperName(mitk::DataNode* node)
{
    if (!node) {
        return "none";
    }

    auto mapper = node->GetMapper(mitk::BaseRenderer::Standard2D);
    return mapper ? mapper->GetNameOfClass() : "none";
}

const char* boolPropertyState(mitk::DataNode* node, const char* propertyName, const mitk::BaseRenderer* renderer)
{
    if (!node) {
        return "no-node";
    }

    mitk::BoolProperty* property = nullptr;
    if (!node->GetProperty(property, propertyName, renderer) || !property) {
        return "missing";
    }

    return property->GetValue() ? "true" : "false";
}

const char* directBoolPropertyState(mitk::DataNode* node, const char* propertyName, const mitk::BaseRenderer* renderer)
{
    if (!node || !renderer) {
        return "no-node";
    }

    bool value = false;
    if (!node->GetPropertyList(renderer)->GetBoolProperty(propertyName, value)) {
        return "missing";
    }

    return value ? "true" : "false";
}

void logResliceRendererState(const char* label,
                             QmitkRenderWindow* renderWindow,
                             mitk::DataNode* imageNode,
                             mitk::DataNode* vesselNode,
                             const mitk::DataStorage::SetOfObjects* contourNodes,
                             mitk::DataNode* solidNode)
{
    if (!renderWindow || !renderWindow->GetRenderer() || !renderWindow->GetVtkRenderWindow()) {
        return;
    }

    auto renderer = renderWindow->GetRenderer();
    auto stepper = renderer->GetSliceNavigationController()->GetStepper();
    bool imageVisible = false;
    bool vesselVisible = false;
    bool solidVisible = false;
    imageNode && imageNode->GetVisibility(imageVisible, renderer);
    vesselNode && vesselNode->GetVisibility(vesselVisible, renderer);
    solidNode && solidNode->GetVisibility(solidVisible, renderer);

    int visibleContours = 0;
    mitk::DataNode* firstContourNode = nullptr;
    if (contourNodes) {
        for (const mitk::DataNode::Pointer& contourNode : *contourNodes) {
            if (!firstContourNode) {
                firstContourNode = contourNode.GetPointer();
            }
            bool contourVisible = false;
            contourNode->GetVisibility(contourVisible, renderer);
            visibleContours += contourVisible ? 1 : 0;
        }
    }

    int* vtkSize = renderWindow->GetVtkRenderWindow()->GetSize();
    MITK_INFO << "VesselDrivenResliceView " << label
              << " RESLICE_DIAG_V2"
              << ": rendererName=" << (renderer->GetName() ? renderer->GetName() : "null")
              << ", rendererPtr=" << renderer
              << ", vtkRenderWindowPtr=" << renderWindow->GetVtkRenderWindow()
              << ", qtSize=" << renderWindow->width() << "x" << renderWindow->height()
              << ", vtkSize=" << (vtkSize ? vtkSize[0] : 0) << "x" << (vtkSize ? vtkSize[1] : 0)
              << ", mapperId=" << renderer->GetMapperID()
              << ", slice=" << (stepper ? stepper->GetPos() : 0)
              << ", steps=" << (stepper ? stepper->GetSteps() : 0)
              << ", vtkProps=" << renderer->GetVtkRenderer()->GetViewProps()->GetNumberOfItems()
              << ", imageVisible=" << yesNo(imageVisible)
              << ", imageMapper=" << mapperName(imageNode)
              << ", showGradient=" << boolPropertyState(imageNode, "show gradient magnitude", renderer)
              << ", showGradientDirect=" << directBoolPropertyState(imageNode, "show gradient magnitude", renderer)
              << ", vesselVisible=" << yesNo(vesselVisible)
              << ", vesselMapper=" << mapperName(vesselNode)
              << ", contoursVisible=" << visibleContours << "/" << (contourNodes ? contourNodes->size() : 0)
              << ", firstContourMapper=" << mapperName(firstContourNode)
              << ", solidVisible=" << yesNo(solidVisible)
              << ", solidMapper=" << mapperName(solidNode);
}

void logFirstContourState(const char* label, const mitk::DataStorage::SetOfObjects* contourNodes)
{
    if (!contourNodes || contourNodes->empty()) {
        MITK_INFO << "VesselDrivenResliceView contour " << label << ": none";
        return;
    }

    mitk::DataNode* contourNode = contourNodes->front().GetPointer();
    auto figure = dynamic_cast<mitk::PlanarFigure*>(contourNode ? contourNode->GetData() : nullptr);
    if (!figure || !figure->GetPlaneGeometry()) {
        MITK_INFO << "VesselDrivenResliceView contour " << label << ": no planar figure geometry";
        return;
    }

    auto geometry = figure->GetPlaneGeometry();
    mitk::Point3D origin = geometry->GetOrigin();
    mitk::Vector3D spacing = geometry->GetSpacing();
    mitk::Vector3D normal = geometry->GetNormal();
    mitk::BoundingBox::BoundsArrayType bounds = geometry->GetBounds();
    float parameterValue = -1.0f;
    contourNode->GetFloatProperty("lofting.parameterValue", parameterValue);

    MITK_INFO << "VesselDrivenResliceView contour " << label
              << ": name=" << contourNode->GetName()
              << ", parameter=" << parameterValue
              << ", placed=" << (figure->IsPlaced() ? "yes" : "no")
              << ", controlPoints=" << figure->GetNumberOfControlPoints()
              << ", bounds=[" << bounds[0] << "," << bounds[1] << ","
              << bounds[2] << "," << bounds[3] << ","
              << bounds[4] << "," << bounds[5] << "]"
              << ", spacing=[" << spacing[0] << "," << spacing[1] << "," << spacing[2] << "]"
              << ", origin=[" << origin[0] << "," << origin[1] << "," << origin[2] << "]"
              << ", normal=[" << normal[0] << "," << normal[1] << "," << normal[2] << "]";
}

void logResliceWindowLayout(const char* label, QmitkRenderWindow* renderWindow)
{
    if (!renderWindow || !renderWindow->GetVtkRenderWindow()) {
        return;
    }

    int* vtkSize = renderWindow->GetVtkRenderWindow()->GetSize();
    MITK_INFO << "VesselDrivenResliceView window " << label
              << ": qtPos=" << renderWindow->x() << "," << renderWindow->y()
              << ", qtSize=" << renderWindow->width() << "x" << renderWindow->height()
              << ", vtkSize=" << (vtkSize ? vtkSize[0] : 0) << "x" << (vtkSize ? vtkSize[1] : 0)
              << ", visible=" << (renderWindow->isVisible() ? "yes" : "no")
              << ", enabled=" << (renderWindow->isEnabled() ? "yes" : "no");
}

void logResliceGeometryState(const char* label, QmitkRenderWindow* renderWindow)
{
    if (!renderWindow || !renderWindow->GetRenderer() || !renderWindow->GetRenderer()->GetVtkRenderer()) {
        return;
    }

    auto renderer = renderWindow->GetRenderer();
    auto planeGeometry = renderer->GetCurrentWorldPlaneGeometry();
    auto vtkRenderer = renderer->GetVtkRenderer();
    auto camera = vtkRenderer->GetActiveCamera();

    MITK_INFO << "VesselDrivenResliceView geometry " << label
              << ": currentWorldGeometry=" << (renderer->GetCurrentWorldGeometry() ? "yes" : "no")
              << ", currentPlane=" << (planeGeometry ? "yes" : "no")
              << ", parallelScale=" << (camera ? camera->GetParallelScale() : -1.0)
              << ", clippingRange=" << (camera ? camera->GetClippingRange()[0] : -1.0)
              << "," << (camera ? camera->GetClippingRange()[1] : -1.0);

    if (!planeGeometry) {
        return;
    }

    const mitk::BoundingBox::BoundsArrayType bounds = planeGeometry->GetBounds();
    mitk::Vector3D spacing = planeGeometry->GetSpacing();
    mitk::Point3D origin = planeGeometry->GetOrigin();
    mitk::Vector3D normal = planeGeometry->GetNormal();
    MITK_INFO << "VesselDrivenResliceView plane " << label
              << ": bounds=[" << bounds[0] << "," << bounds[1] << ","
              << bounds[2] << "," << bounds[3] << ","
              << bounds[4] << "," << bounds[5] << "]"
              << ", spacing=[" << spacing[0] << "," << spacing[1] << "," << spacing[2] << "]"
              << ", origin=[" << origin[0] << "," << origin[1] << "," << origin[2] << "]"
              << ", normal=[" << normal[0] << "," << normal[1] << "," << normal[2] << "]";
}

void logFramebufferState(const char* label, QmitkRenderWindow* renderWindow)
{
    if (!renderWindow || !renderWindow->GetVtkRenderWindow()) {
        return;
    }

    vtkRenderWindow* vtkWindow = renderWindow->GetVtkRenderWindow();
    int* size = vtkWindow->GetSize();
    if (!size || size[0] <= 0 || size[1] <= 0) {
        MITK_INFO << "VesselDrivenResliceView framebuffer " << label << ": empty window";
        return;
    }

    const int width = size[0];
    const int height = size[1];
    auto pixels = vtkSmartPointer<vtkUnsignedCharArray>::New();
    vtkWindow->GetRGBACharPixelData(0, 0, width - 1, height - 1, 0, pixels, 0);
    unsigned char* pixelData = pixels->GetPointer(0);
    if (!pixelData) {
        MITK_INFO << "VesselDrivenResliceView framebuffer " << label << ": no pixel data";
        return;
    }

    int nonBlack = 0;
    int minX = width;
    int minY = height;
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const size_t offset = (static_cast<size_t>(y) * width + x) * 4;
            const int r = pixelData[offset];
            const int g = pixelData[offset + 1];
            const int b = pixelData[offset + 2];
            if (r + g + b > 12) {
                ++nonBlack;
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }

    const double percent = 100.0 * nonBlack / static_cast<double>(width * height);
    MITK_INFO << "VesselDrivenResliceView framebuffer " << label
              << ": size=" << width << "x" << height
              << ", nonBlackPixels=" << nonBlack
              << ", nonBlackPercent=" << percent
              << ", bbox=[" << minX << "," << minY << "," << maxX << "," << maxY << "]";
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// The listener for the reslice view. 
class ResliceViewWidgetListener : public berry::IPartListener {
public:
    berryObjectMacro(ResliceViewWidgetListener);

    ResliceViewWidgetListener(VesselDrivenResliceView* resliceView)
        : _resliceView(resliceView)
    {
    }

    ~ResliceViewWidgetListener()
    {
    }

    void registerListener()
    {
        _resliceView->GetSite()->GetPage()->AddPartListener(this);
    }

    void unregisterListener()
    {
        _resliceView->GetSite()->GetPage()->RemovePartListener(this);
    }

    Events::Types GetPartEventTypes() const override
    {
        return Events::HIDDEN | Events::VISIBLE;
    }

    void PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef) override
    {
        if (partRef->GetPart(false) == _resliceView) {
            _resliceView->_updateGeometryNodeInDataStorage();
        }
    }

    void PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef) override
    {
        if (partRef->GetPart(false) == _resliceView) {
            _resliceView->_updateGeometryNodeInDataStorage();
        }
    }

private:
    VesselDrivenResliceView* _resliceView;
};


enum ObservedObject {
    ooSNC = 0,
    ooSNC_GradMag,
    ooRenderingManager,

    ooLast
};

class VesselDrivenResliceViewPrivate {
public:
    VesselDrivenResliceViewPrivate()
        : sacrificialRenderWindow(nullptr)
        , renderWindow(nullptr)
        , renderWindowGradMag(nullptr)
        , settingUpRendererSlices(false)
    {
        reinitVesselDrivenGeometryTimer.setSingleShot(true);
    }

    QHash<ObservedObject, QPair<itk::Object::Pointer, unsigned long>> observerTags;

    unsigned long vesselPathObserverTag;
    unsigned long renderingManagerInitializeObserverTag;
    QVBoxLayout* mainLayout;
    QmitkRenderWindow* sacrificialRenderWindow;
    QmitkRenderWindow* renderWindow;
    QmitkRenderWindow* renderWindowGradMag;
    QSpacerItem* spacer;
    ctkSliderWidget* sliceNumberSlider;
    QDoubleSpinBox* resliceWindowSizeSpinBox;   
    QToolButton* resliceWidgetVisibilityButton;
    QTimer reinitVesselDrivenGeometryTimer;
    bool settingUpRendererSlices;
    QLabel* positionInMM;
    QScopedPointer<ResliceViewWidgetListener> resliceViewWidgetListener;

    QHash<const mitk::DataNode*, mitk::Point3D> savedSlicePositions;


    void _addObserver(ObservedObject type, itk::Object::Pointer object, const itk::EventObject& event, itk::Command* command)
    {
        assert(observerTags.find(type) == observerTags.end());
        command->Register();

        observerTags[type] = qMakePair(object, object->AddObserver(event, command));
    }

    void _removeObserver(ObservedObject type)
    {
        if (observerTags.contains(type)) {
            observerTags[type].first->RemoveObserver(observerTags[type].second);
            observerTags.remove(type);
        }
    }

};


const std::string VesselDrivenResliceView::VIEW_ID = "org.mitk.views.VesselDrivenResliceView";

VesselDrivenResliceView::VesselDrivenResliceView()
    : NodeDependentView(crimson::VascularModelingNodeTypes::VesselPath(), true, QString("Vessel path"), true)
    , d(new VesselDrivenResliceViewPrivate())
{
    d->resliceViewWidgetListener.reset(new ResliceViewWidgetListener(this));
}

VesselDrivenResliceView::~VesselDrivenResliceView()
{
    d->resliceViewWidgetListener->unregisterListener();

    // Remove all observers
    for (int i = 0; i < ooLast; ++i) {
        d->_removeObserver(static_cast<ObservedObject>(i));
    }

    _removeGeometryNodeFromDataStorage();
}

void VesselDrivenResliceView::SetFocus()
{
}

void VesselDrivenResliceView::CreateQtPartControl(QWidget *parent)
{
    d->mainLayout = new QVBoxLayout(parent);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(createSelectedNodeWidget(parent));

    QToolButton* reslicePropertiesButton = new QToolButton(parent);
    reslicePropertiesButton->setIcon(QIcon(":/vesselSeg/icons/settings.png"));
    reslicePropertiesButton->setIconSize(QSize(24, 24));
    reslicePropertiesButton->setToolTip(tr("Modify the reslice window properties."));
    hLayout->addWidget(reslicePropertiesButton);

    d->resliceWidgetVisibilityButton = new QToolButton(parent);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/vesselSeg/icons/vis_off.png"), QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(QString::fromUtf8(":/vesselSeg/icons/vis_on.png"), QSize(), QIcon::Normal, QIcon::On);
    d->resliceWidgetVisibilityButton->setIcon(icon);
    d->resliceWidgetVisibilityButton->setIconSize(QSize(24, 24));
    d->resliceWidgetVisibilityButton->setToolTip(tr("Show/hide the reslice position in 3D window."));
    d->resliceWidgetVisibilityButton->setCheckable(true);
    d->resliceWidgetVisibilityButton->setChecked(true);
    hLayout->addWidget(d->resliceWidgetVisibilityButton);
    connect(d->resliceWidgetVisibilityButton, &QAbstractButton::clicked, this, &VesselDrivenResliceView::_updateGeometryNodeInDataStorage);

    d->mainLayout->addLayout(hLayout);

    ctkPopupWidget* popup = new ctkPopupWidget(reslicePropertiesButton);
    QVBoxLayout* popupLayout = new QVBoxLayout(popup);

    auto reslicePropertiesWidget = new QWidget(popup);
    Ui::ReslicePropertiesWidget ui;
    ui.setupUi(reslicePropertiesWidget);

    popupLayout->addWidget(reslicePropertiesWidget);

    popup->setAlignment(Qt::AlignHCenter | Qt::AlignBottom); // at the top left corner
    popup->setHorizontalDirection(Qt::RightToLeft); // open outside the parent
    popup->setVerticalDirection(ctkBasePopupWidget::TopToBottom); // at the left of the spinbox sharing the top border
    // Control the animation
    popup->setAnimationEffect(ctkBasePopupWidget::FadeEffect); // could also be FadeEffect
    popup->setOrientation(Qt::Horizontal); // how to animate, could be Qt::Vertical or Qt::Horizontal|Qt::Vertical
    popup->setEasingCurve(QEasingCurve::OutQuart); // how to accelerate the animation, QEasingCurve::Type
    popup->setEffectDuration(100); // how long in ms.
    // Control the behavior
    popup->setAutoShow(false); // automatically open when the mouse is over the spinbox
    popup->setAutoHide(true); // automatically hide when the mouse leaves the popup or the spinbox.

    d->resliceWindowSizeSpinBox = ui.resliceWindowSizeSpinBox;

    connect(reslicePropertiesButton, SIGNAL(clicked()), popup, SLOT(showPopup()));
    connect(ui.resliceWindowSizeSpinBox, &QAbstractSpinBox::editingFinished, this, &VesselDrivenResliceView::_setResliceWindowSize);

    hLayout = new QHBoxLayout;
    d->sliceNumberSlider = new ctkSliderWidget(parent);
    d->sliceNumberSlider->setDecimals(0);
    hLayout->addWidget(d->sliceNumberSlider);

    d->positionInMM = new QLabel(parent);
    d->positionInMM->setTextInteractionFlags(Qt::TextSelectableByMouse);
    d->positionInMM->setText("0.00 mm");
    d->positionInMM->setMinimumWidth(d->positionInMM->fontMetrics().horizontalAdvance(QStringLiteral("9999.99 mm")));
    d->positionInMM->setToolTip(tr("Distance from the beginning of vessel path to current position"));
    d->positionInMM->setAlignment(Qt::AlignRight);
    
    hLayout->addWidget(d->positionInMM);
    
    d->mainLayout->addLayout(hLayout);


    auto renderWindowsLayout = new QHBoxLayout;

    d->sacrificialRenderWindow = new QmitkRenderWindow(parent, QStringLiteral("reslicer sacrificial"));
    d->sacrificialRenderWindow->GetRenderer()->SetDataStorage(GetDataStorage());
    d->sacrificialRenderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    d->sacrificialRenderWindow->setFixedWidth(1);
    d->sacrificialRenderWindow->setFocusPolicy(Qt::NoFocus);
    renderWindowsLayout->addWidget(d->sacrificialRenderWindow);

    d->renderWindow = new QmitkRenderWindow(parent, QStringLiteral("reslicer"));
    d->renderWindow->GetRenderer()->SetDataStorage(GetDataStorage());
    d->renderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    d->renderWindow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    renderWindowsLayout->addWidget(d->renderWindow);

    d->renderWindowGradMag = new QmitkRenderWindow(parent, QStringLiteral("reslicer grad mag"));
    d->renderWindowGradMag->GetRenderer()->SetDataStorage(GetDataStorage());
    d->renderWindowGradMag->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    d->renderWindowGradMag->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    renderWindowsLayout->addWidget(d->renderWindowGradMag);

    d->mainLayout->addLayout(renderWindowsLayout, 1);
    d->mainLayout->addStretch(0);

    // Setup synchronization of renderer's stepper and the slide number
    connect(d->sliceNumberSlider, &ctkSliderWidget::valueChanged, this, &VesselDrivenResliceView::_setSliceNumber);

    auto modifiedCommand = itk::MemberCommand<VesselDrivenResliceView>::New();
    modifiedCommand->SetCallbackFunction(this, &VesselDrivenResliceView::_syncSliderWithStepper);
    modifiedCommand->SetCallbackFunction(this, &VesselDrivenResliceView::_syncSliderWithStepperC);
    d->_addObserver(ooSNC, d->renderWindow->GetRenderer()->GetSliceNavigationController()->GetStepper(), itk::ModifiedEvent(), modifiedCommand);


    modifiedCommand = itk::MemberCommand<VesselDrivenResliceView>::New();
    modifiedCommand->SetCallbackFunction(this, &VesselDrivenResliceView::_syncSliderWithStepper);
    modifiedCommand->SetCallbackFunction(this, &VesselDrivenResliceView::_syncSliderWithStepperC);
    d->_addObserver(ooSNC_GradMag, d->renderWindowGradMag->GetRenderer()->GetSliceNavigationController()->GetStepper(), itk::ModifiedEvent(), modifiedCommand);


    // Global InitializeViews() replaces renderer world geometries. Keep the
    // windows registered so QmitkRenderWindow paint/update wiring still works,
    // and restore the vessel-driven geometry whenever the global reset runs.
    auto reinitCommand = itk::SimpleMemberCommand<VesselDrivenResliceView>::New();
    reinitCommand->SetCallbackFunction(this, &VesselDrivenResliceView::_setupRendererSlices);
    d->_addObserver(ooRenderingManager, mitk::RenderingManager::GetInstance(), mitk::RenderingManagerViewsInitializedEvent(), reinitCommand);

    connect(&d->reinitVesselDrivenGeometryTimer, &QTimer::timeout, this, &VesselDrivenResliceView::_setupRendererSlices);

    d->resliceViewWidgetListener->registerListener();

    _setResliceViewEnabled(false);
    initializeCurrentNode();
    _updateGeometryNodeInDataStorage();
}

mitk::BaseRenderer* VesselDrivenResliceView::getResliceRenderer() const
{
    return d->renderWindow->GetRenderer();
}

std::vector<mitk::BaseRenderer*> VesselDrivenResliceView::getAllResliceRenderers() const
{
    std::vector<mitk::BaseRenderer*> renderers = { d->renderWindow->GetRenderer(), d->renderWindowGradMag->GetRenderer() };
    return renderers;
}

void VesselDrivenResliceView::navigateTo(const mitk::Point3D& pos)
{
    mitk::SliceNavigationController* snc = d->renderWindow->GetRenderer()->GetSliceNavigationController();
    const mitk::TimeGeometry* worldTimeGeometry = snc->GetInputWorldTimeGeometry();
    if (worldTimeGeometry == nullptr) {
        return;
    }
    auto vesselDrivenGeometry = static_cast<const crimson::VesselDrivenSlicedGeometry*>(worldTimeGeometry->GetGeometryForTimeStep(0).GetPointer());
    int sliceNo = vesselDrivenGeometry->findSliceByPoint(pos);
    snc->GetStepper()->SetPos(sliceNo);
    d->renderWindowGradMag->GetRenderer()->GetSliceNavigationController()->GetStepper()->SetPos(sliceNo);
}

void VesselDrivenResliceView::navigateTo(float parameterValue)
{
    mitk::SliceNavigationController* snc = d->renderWindow->GetRenderer()->GetSliceNavigationController();
    const mitk::TimeGeometry* worldTimeGeometry = snc->GetInputWorldTimeGeometry();
    if (worldTimeGeometry == nullptr) {
        return;
    }
    auto vesselDrivenGeometry = static_cast<const crimson::VesselDrivenSlicedGeometry*>(worldTimeGeometry->GetGeometryForTimeStep(0).GetPointer());
    int sliceNo = vesselDrivenGeometry->getSliceNumberByParameterValue(parameterValue);
    snc->GetStepper()->SetPos(sliceNo);
    d->renderWindowGradMag->GetRenderer()->GetSliceNavigationController()->GetStepper()->SetPos(sliceNo);
}

float VesselDrivenResliceView::getCurrentParameterValue() const
{
	auto geometry = dynamic_cast<const crimson::VesselDrivenSlicedGeometry*>(getResliceRenderer()->GetCurrentWorldGeometry());
    return geometry == nullptr ? 0 : geometry->getParameterValueBySliceNumber(getResliceRenderer()->GetSliceNavigationController()->GetStepper()->GetPos());
}

mitk::PlaneGeometry* VesselDrivenResliceView::getPlaneGeometry(float t) const
{
	auto geometry = dynamic_cast<const crimson::VesselDrivenSlicedGeometry*>(getResliceRenderer()->GetCurrentWorldGeometry());
    return geometry->GetPlaneGeometry(geometry->getSliceNumberByParameterValue(t));
}

void VesselDrivenResliceView::_setResliceViewEnabled(bool enabled)
{
    d->sliceNumberSlider->setEnabled(enabled);
    d->sacrificialRenderWindow->setEnabled(enabled);
    d->sacrificialRenderWindow->setVisible(enabled);
    d->renderWindow->setEnabled(enabled);
    d->renderWindow->setVisible(enabled);
    d->renderWindowGradMag->setEnabled(enabled);
    d->renderWindowGradMag->setVisible(enabled);
}

void VesselDrivenResliceView::currentNodeChanged(mitk::DataNode*)
{
    _setResliceViewEnabled(_isCurrentVesselPathValid());

    if (currentNode()) {
        MITK_WARN << "VesselDrivenResliceView RESLICE_TRACE_V3 currentNodeChanged: node=" << currentNode()->GetName()
                  << ", data=" << nodeDataClassName(currentNode())
                  << ", valid=" << yesNo(_isCurrentVesselPathValid());

        currentNode()->SetIntProperty("vesselpath.line_width", 4, d->renderWindow->GetRenderer());
        currentNode()->SetIntProperty("vesselpath.selected_line_width", 4, d->renderWindow->GetRenderer());
        currentNode()->SetIntProperty("vesselpath.editing_line_width", 6, d->renderWindow->GetRenderer());
        currentNode()->SetSelected(true, d->renderWindow->GetRenderer());
        currentNode()->SetIntProperty("vesselpath.line_width", 4, d->renderWindowGradMag->GetRenderer());
        currentNode()->SetIntProperty("vesselpath.selected_line_width", 4, d->renderWindowGradMag->GetRenderer());
        currentNode()->SetIntProperty("vesselpath.editing_line_width", 6, d->renderWindowGradMag->GetRenderer());
        currentNode()->SetSelected(true, d->renderWindowGradMag->GetRenderer());

        float resliceWindowSize = 50;
        currentNode()->GetFloatProperty("reslice.windowSize", resliceWindowSize);
        if (resliceWindowSize < 25.0f) {
            MITK_WARN << "VesselDrivenResliceView RESLICE_TRACE_V3 reslice.windowSize too small on node; raw="
                      << resliceWindowSize << ", using=50";
            resliceWindowSize = 50.0f;
            currentNode()->SetFloatProperty("reslice.windowSize", resliceWindowSize);
        }
        d->resliceWindowSizeSpinBox->blockSignals(true);
        d->resliceWindowSizeSpinBox->setValue(resliceWindowSize);
        d->resliceWindowSizeSpinBox->blockSignals(false);

        _setupRendererSlices();
    }
    else {
        d->positionInMM->setText("0.00 mm");
    }
    _updateGeometryNodeInDataStorage();
}

void VesselDrivenResliceView::currentNodeModified()
{
    _setResliceViewEnabled(_isCurrentVesselPathValid());
    auto vesselPathInteractor = dynamic_cast<crimson::VesselPathInteractor*>(currentNode()->GetDataInteractor().GetPointer());
    if (vesselPathInteractor &&
        vesselPathInteractor->getMovingControlPoint() != -1 &&
        (d->renderWindow->GetRenderer() == vesselPathInteractor->lastEventSender() ||
        d->renderWindowGradMag->GetRenderer() == vesselPathInteractor->lastEventSender())) {
        auto vesselPath = static_cast<crimson::VesselPathAbstractData*>(currentNode()->GetData());
        // Do not update the geometry when moving the point in the reslice window
        // But ensure that after the move we reset the geometry on the control point being moved
        d->savedSlicePositions[currentNode()] = vesselPath->getControlPoint(vesselPathInteractor->getMovingControlPoint());
        return;
    }

    d->reinitVesselDrivenGeometryTimer.start(200);
    _updateGeometryNodeInDataStorage();
}

void VesselDrivenResliceView::forceReinitGeometry()
{
    if (!d->reinitVesselDrivenGeometryTimer.isActive()) {
        return; // Geometry is valid
    }

    d->reinitVesselDrivenGeometryTimer.stop();
    _setupRendererSlices();
}

void VesselDrivenResliceView::_setupRendererSlices()
{
    logSetupCheckpoint("enter");
    if (d->settingUpRendererSlices) {
        logSetupCheckpoint("abort recursive setup");
        return;
    }

    if (!_isCurrentVesselPathValid()) {
        logSetupCheckpoint("abort invalid current vessel path");
        return;
    }

    d->settingUpRendererSlices = true;
    d->sliceNumberSlider->blockSignals(true);

    auto vesselPath = static_cast<crimson::VesselPathAbstractData*>(currentNode()->GetData());

    if (!vesselPath || vesselPath->controlPointsCount() == 0) {
        MITK_INFO << "VesselDrivenResliceView setup abort: vesselPath="
                  << (vesselPath ? "yes" : "no")
                  << ", controlPoints=" << (vesselPath ? vesselPath->controlPointsCount() : 0);
        d->sliceNumberSlider->blockSignals(false);
        d->settingUpRendererSlices = false;
        return;
    }

    MITK_INFO << "VesselDrivenResliceView setup vessel: node=" << currentNode()->GetName()
              << ", data=" << nodeDataClassName(currentNode())
              << ", controlPoints=" << vesselPath->controlPointsCount()
              << ", parametricLength=" << vesselPath->getParametricLength();

    float rawResliceWindowSize = 50;
    currentNode()->GetFloatProperty("reslice.windowSize", rawResliceWindowSize);
    float resliceWindowSize = rawResliceWindowSize;
    if (resliceWindowSize < 25.0f) {
        resliceWindowSize = 50.0f;
        currentNode()->SetFloatProperty("reslice.windowSize", resliceWindowSize);
    }
    MITK_WARN << "VesselDrivenResliceView RESLICE_TRACE_V3 reslice size: raw=" << rawResliceWindowSize
              << ", effective=" << resliceWindowSize;

    mitk::ScalarType paramDelta;
    mitk::Vector3D referenceImageSpacing;
    unsigned int timeSteps;

    logSetupCheckpoint("before getResliceGeometryParameters");
    std::tie(paramDelta, referenceImageSpacing, timeSteps) = crimson::VascularModelingUtils::getResliceGeometryParameters(currentNode());
    MITK_INFO << "VesselDrivenResliceView setup geometry parameters: paramDelta=" << paramDelta
              << ", referenceImageSpacing=[" << referenceImageSpacing[0] << ","
              << referenceImageSpacing[1] << "," << referenceImageSpacing[2] << "]"
              << ", timeSteps=" << timeSteps;

    logSetupCheckpoint("before image ancestor lookup");
    mitk::DataNode::Pointer imageNode = crimson::HierarchyManager::getInstance()->getAncestor(currentNode(), crimson::VascularModelingNodeTypes::Image());
    MITK_INFO << "VesselDrivenResliceView setup image ancestor: exists=" << yesNo(imageNode.IsNotNull())
              << ", name=" << (imageNode.IsNotNull() ? imageNode->GetName() : std::string("none"))
              << ", data=" << nodeDataClassName(imageNode.GetPointer());

    if (imageNode.IsNotNull()) {
        logSetupCheckpoint("before configure sacrificial image node");
        configureImageNodeForReslice(imageNode, d->sacrificialRenderWindow, false);
        logSetupCheckpoint("after configure sacrificial image node");
        logSetupCheckpoint("before configure primary image node");
        configureImageNodeForReslice(imageNode, d->renderWindow, false);
        logSetupCheckpoint("after configure primary image node");
        logSetupCheckpoint("before configure gradient image node");
        configureImageNodeForReslice(imageNode, d->renderWindowGradMag, true);
        logSetupCheckpoint("after configure gradient image node");
    }

    logSetupCheckpoint("before configure vessel overlays");
    configureOverlayNodeForReslice(currentNode(), d->sacrificialRenderWindow, true);
    configureOverlayNodeForReslice(currentNode(), d->renderWindow, true);
    configureOverlayNodeForReslice(currentNode(), d->renderWindowGradMag, true);
    logSetupCheckpoint("after configure vessel overlays");

    logSetupCheckpoint("before ensureVesselPathMapper");
    ensureVesselPathMapper(currentNode());
    logSetupCheckpoint("after ensureVesselPathMapper");

    logSetupCheckpoint("before contour lookup");
    mitk::DataStorage::SetOfObjects::ConstPointer contourNodes = crimson::VascularModelingUtils::getVesselContourNodes(currentNode());
    MITK_INFO << "VesselDrivenResliceView setup contours: exists=" << yesNo(contourNodes.IsNotNull())
              << ", count=" << (contourNodes.IsNotNull() ? contourNodes->size() : 0);
    if (contourNodes.IsNotNull()) {
        unsigned int contourIndex = 0;
        for (const mitk::DataNode::Pointer& contourNode : *contourNodes) {
            MITK_INFO << "VesselDrivenResliceView setup contour[" << contourIndex
                      << "]: name=" << (contourNode.IsNotNull() ? contourNode->GetName() : std::string("null"))
                      << ", data=" << nodeDataClassName(contourNode.GetPointer())
                      << ", mapper=" << mapperName(contourNode.GetPointer());
            configureOverlayNodeForReslice(contourNode, d->sacrificialRenderWindow, true);
            configureOverlayNodeForReslice(contourNode, d->renderWindow, true);
            configureOverlayNodeForReslice(contourNode, d->renderWindowGradMag, true);
            ++contourIndex;
        }
    }
    logSetupCheckpoint("after contour visibility setup");

    logSetupCheckpoint("before solid lookup");
    mitk::DataNode* solidNode = crimson::VascularModelingUtils::getVesselSolidModelNode(currentNode());
    MITK_INFO << "VesselDrivenResliceView setup solid: exists=" << yesNo(solidNode != nullptr)
              << ", name=" << (solidNode ? solidNode->GetName() : std::string("none"))
              << ", data=" << nodeDataClassName(solidNode)
              << ", mapperBefore=" << mapperName(solidNode);
    if (solidNode) {
        logSetupCheckpoint("before ensureSolidDataMapper");
        ensureSolidDataMapper(solidNode);
        MITK_INFO << "VesselDrivenResliceView setup solid: mapperAfterEnsure=" << mapperName(solidNode);
        logSetupCheckpoint("after ensureSolidDataMapper");
        configureOverlayNodeForReslice(solidNode, d->sacrificialRenderWindow, true);
        configureOverlayNodeForReslice(solidNode, d->renderWindow, true);
        configureOverlayNodeForReslice(solidNode, d->renderWindowGradMag, true);
        logSetupCheckpoint("after solid visibility setup");
    }

    logSetupCheckpoint("before VesselDrivenSlicedGeometry::New");
    auto vesselDrivenGeometry = crimson::VesselDrivenSlicedGeometry::New();
    logSetupCheckpoint("after VesselDrivenSlicedGeometry::New");
    logSetupCheckpoint("before InitializedVesselDrivenSlicedGeometry");
    vesselDrivenGeometry->InitializedVesselDrivenSlicedGeometry(vesselPath, paramDelta, referenceImageSpacing, resliceWindowSize);
    MITK_INFO << "VesselDrivenResliceView setup vessel geometry: slices=" << vesselDrivenGeometry->GetSlices();
    logSetupCheckpoint("after InitializedVesselDrivenSlicedGeometry");

    logSetupCheckpoint("before prebuild plane geometries");
    for (unsigned int slice = 0; slice < vesselDrivenGeometry->GetSlices(); ++slice) {
        auto plane = vesselDrivenGeometry->GetPlaneGeometry(slice);
        if (!plane && (slice == 0 || slice + 1 == vesselDrivenGeometry->GetSlices())) {
            MITK_INFO << "VesselDrivenResliceView setup plane prebuild failed: slice=" << slice;
        }
    }
    logSetupCheckpoint("after prebuild plane geometries");

    logSetupCheckpoint("before proportional time geometry");
    mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(vesselDrivenGeometry, timeSteps);
    logSetupCheckpoint("after proportional time geometry");

    mitk::Point3D savedSlicePos = d->savedSlicePositions.value(currentNode(), vesselPath->getPosition(0));
    MITK_INFO << "VesselDrivenResliceView setup saved slice pos: ["
              << savedSlicePos[0] << "," << savedSlicePos[1] << "," << savedSlicePos[2] << "]";

    logSetupCheckpoint("before sacrificial SNC setup");
    mitk::SliceNavigationController* snc = d->sacrificialRenderWindow->GetRenderer()->GetSliceNavigationController();
    logSetupCheckpoint("sacrificial SNC before SetInputWorldTimeGeometry");
    snc->SetInputWorldTimeGeometry(timeGeometry);
    logSetupCheckpoint("sacrificial SNC after SetInputWorldTimeGeometry");
    logSetupCheckpoint("sacrificial SNC before SetViewDirection");
    snc->SetViewDirection(mitk::AnatomicalPlane::Original);
    logSetupCheckpoint("sacrificial SNC after SetViewDirection");
    logSetupCheckpoint("sacrificial SNC before SetDefaultViewDirection");
    snc->SetDefaultViewDirection(mitk::AnatomicalPlane::Original);
    logSetupCheckpoint("sacrificial SNC after SetDefaultViewDirection");
    logSetupCheckpoint("sacrificial SNC before Update");
    snc->Update();
    logSetupCheckpoint("sacrificial SNC after Update");
    logSetupCheckpoint("after sacrificial SNC setup");

    logSetupCheckpoint("before primary SNC setup");
    snc = d->renderWindow->GetRenderer()->GetSliceNavigationController();
    logSetupCheckpoint("primary SNC before SetInputWorldTimeGeometry");
    snc->SetInputWorldTimeGeometry(timeGeometry);
    logSetupCheckpoint("primary SNC after SetInputWorldTimeGeometry");
    logSetupCheckpoint("primary SNC before SetViewDirection");
    snc->SetViewDirection(mitk::AnatomicalPlane::Original);
    logSetupCheckpoint("primary SNC after SetViewDirection");
    logSetupCheckpoint("primary SNC before SetDefaultViewDirection");
    snc->SetDefaultViewDirection(mitk::AnatomicalPlane::Original);
    logSetupCheckpoint("primary SNC after SetDefaultViewDirection");
    logSetupCheckpoint("primary SNC before Update");
    snc->Update();
    logSetupCheckpoint("primary SNC after Update");
    logSetupCheckpoint("after primary SNC setup");

    logSetupCheckpoint("before gradient SNC setup");
    snc = d->renderWindowGradMag->GetRenderer()->GetSliceNavigationController();
    logSetupCheckpoint("gradient SNC before SetInputWorldTimeGeometry");
    snc->SetInputWorldTimeGeometry(timeGeometry);
    logSetupCheckpoint("gradient SNC after SetInputWorldTimeGeometry");
    logSetupCheckpoint("gradient SNC before SetViewDirection");
    snc->SetViewDirection(mitk::AnatomicalPlane::Original);
    logSetupCheckpoint("gradient SNC after SetViewDirection");
    logSetupCheckpoint("gradient SNC before SetDefaultViewDirection");
    snc->SetDefaultViewDirection(mitk::AnatomicalPlane::Original);
    logSetupCheckpoint("gradient SNC after SetDefaultViewDirection");
    logSetupCheckpoint("gradient SNC before Update");
    snc->Update();
    logSetupCheckpoint("gradient SNC after Update");
    logSetupCheckpoint("after gradient SNC setup");

    logSetupCheckpoint("before navigateTo saved position");
    navigateTo(savedSlicePos);
    logSetupCheckpoint("after navigateTo saved position");

    logSetupCheckpoint("before SetWorldTimeGeometry/SetSlice");
    d->sacrificialRenderWindow->GetRenderer()->SetWorldTimeGeometry(timeGeometry);
    d->renderWindow->GetRenderer()->SetWorldTimeGeometry(timeGeometry);
    d->renderWindowGradMag->GetRenderer()->SetWorldTimeGeometry(timeGeometry);
    d->sacrificialRenderWindow->GetRenderer()->SetSlice(d->sacrificialRenderWindow->GetRenderer()->GetSliceNavigationController()->GetStepper()->GetPos());
    d->renderWindow->GetRenderer()->SetSlice(d->renderWindow->GetRenderer()->GetSliceNavigationController()->GetStepper()->GetPos());
    d->renderWindowGradMag->GetRenderer()->SetSlice(d->renderWindowGradMag->GetRenderer()->GetSliceNavigationController()->GetStepper()->GetPos());
    logSetupCheckpoint("after SetWorldTimeGeometry/SetSlice");

    logSetupCheckpoint("before camera fit");
    d->sacrificialRenderWindow->GetRenderer()->GetCameraController()->Fit();
    d->renderWindow->GetRenderer()->GetCameraController()->Fit();
    d->renderWindowGradMag->GetRenderer()->GetCameraController()->Fit();
    d->sacrificialRenderWindow->GetRenderer()->GetVtkRenderer()->ResetCameraClippingRange();
    d->renderWindow->GetRenderer()->GetVtkRenderer()->ResetCameraClippingRange();
    d->renderWindowGradMag->GetRenderer()->GetVtkRenderer()->ResetCameraClippingRange();
    logSetupCheckpoint("after camera fit");

    logSetupCheckpoint("before ForceImmediateUpdate sacrificial");
    forceImmediateMitkRender(d->sacrificialRenderWindow);
    logSetupCheckpoint("after ForceImmediateUpdate sacrificial");
    logSetupCheckpoint("before ForceImmediateUpdate primary");
    forceImmediateMitkRender(d->renderWindow);
    logSetupCheckpoint("after ForceImmediateUpdate primary");
    logSetupCheckpoint("before ForceImmediateUpdate gradient");
    forceImmediateMitkRender(d->renderWindowGradMag);
    logSetupCheckpoint("after ForceImmediateUpdate gradient");

    logSetupCheckpoint("before final diagnostic logs");
    logResliceRendererState("sacrificial", d->sacrificialRenderWindow, imageNode.GetPointer(), currentNode(), contourNodes.GetPointer(), solidNode);
    logResliceRendererState("primary", d->renderWindow, imageNode.GetPointer(), currentNode(), contourNodes.GetPointer(), solidNode);
    logResliceRendererState("gradient", d->renderWindowGradMag, imageNode.GetPointer(), currentNode(), contourNodes.GetPointer(), solidNode);
    logResliceWindowLayout("sacrificial", d->sacrificialRenderWindow);
    logResliceWindowLayout("primary", d->renderWindow);
    logResliceWindowLayout("gradient", d->renderWindowGradMag);
    logResliceGeometryState("sacrificial", d->sacrificialRenderWindow);
    logResliceGeometryState("primary", d->renderWindow);
    logResliceGeometryState("gradient", d->renderWindowGradMag);
    logFirstContourState("first", contourNodes.GetPointer());
    logFramebufferState("sacrificial", d->sacrificialRenderWindow);
    logFramebufferState("primary", d->renderWindow);
    logFramebufferState("gradient", d->renderWindowGradMag);
    d->sliceNumberSlider->blockSignals(false);
    d->settingUpRendererSlices = false;
    logSetupCheckpoint("exit");
}

void VesselDrivenResliceView::_setSliceNumber(double slice)
{
    if (d->settingUpRendererSlices) {
        MITK_WARN << "VesselDrivenResliceView RESLICE_TRACE_V3 ignoring _setSliceNumber during setup: slice=" << slice;
        return;
    }

    unsigned int sliceNumber = static_cast<unsigned int>(slice);
    d->renderWindow->GetRenderer()->GetSliceNavigationController()->GetStepper()->SetPos(sliceNumber);
    d->renderWindowGradMag->GetRenderer()->GetSliceNavigationController()->GetStepper()->SetPos(sliceNumber);
    d->positionInMM->setText(QString("%1 mm").arg(getCurrentParameterValue(), 6, 'f', 2));

    forceImmediateMitkRender(d->renderWindow);
    forceImmediateMitkRender(d->renderWindowGradMag);
}

void VesselDrivenResliceView::_setResliceWindowSize()
{
    if (currentNode()) {
        currentNode()->SetFloatProperty("reslice.windowSize", d->resliceWindowSizeSpinBox->value());
        _setupRendererSlices();
        emit geometryChanged();
    }
}

void VesselDrivenResliceView::_syncSliderWithStepperC(const itk::Object* o, const itk::EventObject&)
{
    if (d->settingUpRendererSlices) {
        MITK_WARN << "VesselDrivenResliceView RESLICE_TRACE_V3 ignoring stepper sync during setup";
        return;
    }

    static bool updating;
    if (updating) {
        return;
    }
    updating = true;

    // const_cast due to lack of itkGetConstMacro() in the mitk::Stepper
    auto stepper = const_cast<mitk::Stepper*>(static_cast<const mitk::Stepper*>(o));


    // TODO: check from which render window does this signal come from
    d->sliceNumberSlider->setRange(0, stepper->GetSteps() - 1);
    d->sliceNumberSlider->setValue(stepper->GetPos());

    auto timeGeometry = d->renderWindow->GetRenderer()->GetSliceNavigationController()->GetInputWorldTimeGeometry();
    if (timeGeometry) {
        // Avoid saving slice ID's if they come from geometry replacement upon global reinit
        auto vesselDrivenGeometry =
            dynamic_cast<const crimson::VesselDrivenSlicedGeometry*>(timeGeometry->GetGeometryForTimeStep(0).GetPointer());
        if (vesselDrivenGeometry) {
            d->savedSlicePositions[currentNode()] = vesselDrivenGeometry->getSliceCenter(stepper->GetPos());

            emit sliceChanged(vesselDrivenGeometry->getParameterValueBySliceNumber(stepper->GetPos()));
        }
    }

    updating = false;
}

void VesselDrivenResliceView::_updateGeometryNodeInDataStorage()
{
    mitk::DataNode* resliceWidgetNode = d->renderWindow->GetRenderer()->GetCurrentWorldPlaneGeometryNode();
    resliceWidgetNode->SetVisibility(_isCurrentVesselPathValid() && d->resliceWidgetVisibilityButton->isChecked() && 
        this->GetSite() && this->GetSite()->GetPage() && this->GetSite()->GetPage()->IsPartVisible(berry::IWorkbenchPart::Pointer(this)));
    if (!GetDataStorage()->Exists(resliceWidgetNode)) {
        resliceWidgetNode->SetBoolProperty("helper object", true);
        resliceWidgetNode->SetName("Vessel reslice plane");
        resliceWidgetNode->SetIntProperty("Crosshair.Gap Size", 4);
        resliceWidgetNode->SetBoolProperty("Crosshair.Ignore", true);
        GetDataStorage()->Add(resliceWidgetNode);
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void VesselDrivenResliceView::_removeGeometryNodeFromDataStorage()
{
    mitk::DataNode* resliceWidgetNode = d->renderWindow->GetRenderer()->GetCurrentWorldPlaneGeometryNode();
    if (GetDataStorage()->Exists(resliceWidgetNode)) {
        GetDataStorage()->Remove(resliceWidgetNode);
    }
}


bool VesselDrivenResliceView::_isCurrentVesselPathValid()
{
    return currentNode() != nullptr && static_cast<crimson::VesselPathAbstractData*>(currentNode()->GetData())->controlPointsCount() >= 2;
}
