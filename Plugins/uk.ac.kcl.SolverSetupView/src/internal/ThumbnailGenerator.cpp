#include "ThumbnailGenerator.h"

#include <unordered_map>

#include <QTimer>
#include <QElapsedTimer>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

#include <vtkNew.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageData.h>

#include <QmitkRenderWindow.h>
#include <mitkRenderWindow.h>
#include <mitkPlanarFigure.h>
#include <mitkProportionalTimeGeometry.h>
#include <HierarchyManager.h>

namespace crimson {

struct DataNodeConstPointerHasher {
    std::size_t operator()(mitk::DataNode::ConstPointer ptr) const { return std::hash<const mitk::DataNode*>()(ptr.GetPointer()); }
};

class ThumbnailGeneratorPrivate {
public:
#ifdef Q_OS_LINUX
    QmitkRenderWindow* thumbnailRenderwindow;
    bool firstRender = true;
#else 
    mitk::RenderWindow::Pointer thumbnailRenderwindow;
#endif
    QTimer thumbnailUpdateTimer;

    struct ThumbnailRequestInfo {
        QElapsedTimer requestTimer;
        mitk::TimePointType time;
        mitk::DataNode::ConstPointer imageNode;
    };

    std::unordered_map<mitk::DataNode::ConstPointer, ThumbnailRequestInfo, DataNodeConstPointerHasher> thumbnailUpdateMap;
};

ThumbnailGenerator::ThumbnailGenerator(mitk::DataStorage* dataStorage)
    : d(new ThumbnailGeneratorPrivate)
{
#ifdef Q_OS_LINUX
    d->thumbnailRenderwindow = new QmitkRenderWindow();

    d->thumbnailRenderwindow->setFixedSize(128, 128); 
    d->thumbnailRenderwindow->setWindowFlags(Qt::ToolTip | Qt::Dialog);
#else
    vtkRenderWindow* renWin = vtkRenderWindow::New(); // Non-smart pointer is intentional
    renWin->OffScreenRenderingOn();
    renWin->BordersOff();

    d->thumbnailRenderwindow = mitk::RenderWindow::New(renWin);
    d->thumbnailRenderwindow->SetSize(128, 128);
#endif

    d->thumbnailRenderwindow->GetRenderer()->SetDataStorage(dataStorage);

    d->thumbnailUpdateTimer.start(100); 
    connect(&d->thumbnailUpdateTimer, SIGNAL(timeout()), this, SLOT(_generateOneThumbnail()));

    mitk::RenderingManager::GetInstance()->RemoveRenderWindow(d->thumbnailRenderwindow->GetVtkRenderWindow());
}

ThumbnailGenerator::~ThumbnailGenerator()
{
}

void ThumbnailGenerator::requestThumbnail(mitk::DataNode::ConstPointer planarFigureNode, mitk::DataNode::ConstPointer imageNode, mitk::TimePointType time)
{
    d->thumbnailUpdateMap[planarFigureNode].requestTimer.restart();
    d->thumbnailUpdateMap[planarFigureNode].time = time;
    d->thumbnailUpdateMap[planarFigureNode].imageNode = imageNode;
}

void ThumbnailGenerator::cancelThumbnailRequest(mitk::DataNode::ConstPointer planarFigureNode)
{
    d->thumbnailUpdateMap.erase(planarFigureNode);
}

mitk::BaseRenderer* ThumbnailGenerator::getThumbnailRenderer()
{
    return d->thumbnailRenderwindow->GetRenderer();
}

void ThumbnailGenerator::_generateThumbnail(mitk::DataNode::ConstPointer planarFigureNode, mitk::DataNode::ConstPointer imageNode, mitk::TimePointType time)
{
    mitk::VtkPropRenderer* renderer = d->thumbnailRenderwindow->GetRenderer();

    mitk::PlaneGeometry::Pointer geo = static_cast<mitk::PlanarFigure*>(planarFigureNode->GetData())->GetPlaneGeometry()->Clone();
    if (imageNode && imageNode->GetData()) {
        geo->SetReferenceGeometry(imageNode->GetData()->GetGeometry());
    } else {
        geo->SetReferenceGeometry(geo);
    }
    renderer->SetCurrentWorldGeometry(geo);
    renderer->GetCameraController()->Fit();
    mitk::ProportionalTimeGeometry::Pointer thumbTimeGeometry = mitk::ProportionalTimeGeometry::New();
    thumbTimeGeometry->Initialize(geo, 1);
    thumbTimeGeometry->SetFirstTimePoint(time);
    renderer->SetWorldTimeGeometry(thumbTimeGeometry);

    mitk::RenderingManager::GetInstance()->AddRenderWindow(d->thumbnailRenderwindow->GetVtkRenderWindow());

    mitk::DataStorage::SetOfObjects::ConstPointer allNodes =
        crimson::HierarchyManager::getInstance()->getDataStorage()->GetAll();
    for (const mitk::DataNode::Pointer& node : *allNodes) {
        node->SetVisibility(false, renderer);
    }

    const_cast<mitk::DataNode*>(planarFigureNode.GetPointer())->SetVisibility(true, renderer);
    if (imageNode) {
        const_cast<mitk::DataNode*>(imageNode.GetPointer())->SetVisibility(true, renderer);
    }

#ifdef Q_OS_LINUX
    // Qt6: QDesktopWidget removed; primary screen comes from QGuiApplication.
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    d->thumbnailRenderwindow->move(rec.width() - d->thumbnailRenderwindow->width(), rec.height() - d->thumbnailRenderwindow->height());
    d->thumbnailRenderwindow->show();

    if (d->firstRender) {
        d->firstRender = false;
        renderer->ForceImmediateUpdate();
    }
#endif

    renderer->ForceImmediateUpdate();

    vtkNew<vtkWindowToImageFilter> imageFilter;
    imageFilter->SetInput(d->thumbnailRenderwindow->GetVtkRenderWindow());
    imageFilter->SetInputBufferTypeToRGB();
    imageFilter->ShouldRerenderOff();
    imageFilter->ReadFrontBufferOff();
    imageFilter->Update();

#ifdef Q_OS_LINUX
    d->thumbnailRenderwindow->hide();
#endif

    mitk::RenderingManager::GetInstance()->RemoveRenderWindow(d->thumbnailRenderwindow->GetVtkRenderWindow());

    vtkImageData* imageData = imageFilter->GetOutput();
    QImage img(imageData->GetDimensions()[0], imageData->GetDimensions()[1], QImage::Format_RGB888);
    img.fill(Qt::red);
    for (int j = 0; j < img.height(); j++) {
        for (int i = 0; i < img.width(); i++) {
            auto pixel = static_cast<unsigned char*>(imageData->GetScalarPointer(i, j, 0));
            img.setPixel(i, img.height() - j - 1, qRgb(pixel[0], pixel[1], pixel[2]));
        }
    }

    emit thumbnailGenerated(planarFigureNode, img);
}

void ThumbnailGenerator::_generateOneThumbnail()
{
    if (d->thumbnailUpdateMap.empty()) {
        return;
    }

    for (auto iter = d->thumbnailUpdateMap.begin(); iter != d->thumbnailUpdateMap.end(); ++iter) {
        if (iter->second.requestTimer.elapsed() > 300) {
            _generateThumbnail(iter->first, iter->second.imageNode, iter->second.time);
            d->thumbnailUpdateMap.erase(iter);
            return;
        }
    }
}


} // namespace crimson
