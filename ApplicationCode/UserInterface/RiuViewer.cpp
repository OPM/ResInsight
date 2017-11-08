/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuViewer.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaColorTools.h"

#include "RimCase.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RivGridBoxGenerator.h"
#include "RivTernarySaturationOverlayItem.h"

#include "RiuCadNavigation.h"
#include "RiuGeoQuestNavigation.h"
#include "RiuRmsNavigation.h"
#include "RiuSimpleHistogramWidget.h"
#include "RiuViewerCommands.h"

#include "cafCategoryLegend.h"
#include "cafCeetronPlusNavigation.h"
#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafFrameAnimationControl.h"

#include "cvfCamera.h"
#include "cvfFont.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOverlayAxisCross.h"
#include "cvfOverlayScalarMapperLegend.h"
#include "cvfRenderQueueSorter.h"
#include "cvfRenderSequence.h"
#include "cvfRendering.h"
#include "cvfScene.h"

#include <QCDEStyle>
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RivWellPathPartMgr.h"
#include "RimSimWellInViewCollection.h"
#include "RimEclipseView.h"

using cvf::ManipulatorTrackball;


const double RI_MIN_NEARPLANE_DISTANCE = 0.1;

//==================================================================================================
///
/// \class RiuViewer
/// \ingroup ResInsight
///
/// 
///
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer::RiuViewer(const QGLFormat& format, QWidget* parent)
: caf::Viewer(format, parent)
{
    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_axisCross = new cvf::OverlayAxisCross(m_mainCamera.p(), standardFont);
    m_axisCross->setAxisLabels("X", "Y", "Z");
    m_axisCross->setLayout(cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_LEFT);
    m_mainRendering->addOverlayItem(m_axisCross.p());

    this->enableOverlyPainting(true);
    this->setReleaseOGLResourcesEachFrame(true);

    QColor c;
    QPalette p = QApplication::palette();
    //QColor frameAndTextColor(255, 255, 255, 255);
    QColor frameAndTextColor(0, 0, 0, 255);
    QColor progressAndHistogramColor(0,0,90,70); // Also Progressbar dark text color

    //p.setColor(QPalette::Window, QColor(144, 173, 208, 180));
    p.setColor(QPalette::Window, QColor(255, 255, 255, 50));

    p.setColor(QPalette::WindowText, frameAndTextColor);

    c = p.color(QPalette::Base );
    c.setAlpha(100);
    p.setColor(QPalette::Base, c);

    //c = p.color(QPalette::AlternateBase );
    //c.setAlpha(0);
    //p.setColor(QPalette::AlternateBase, c);

    
    //p.setColor(QPalette::Highlight, QColor(20, 20, 130, 40));
    p.setColor(QPalette::Highlight, progressAndHistogramColor);

    //p.setColor(QPalette::HighlightedText, frameAndTextColor);
    p.setColor(QPalette::HighlightedText, QColor(255, 255, 255, 255)); //Progressbar light text color

    //p.setColor(QPalette::Dark, QColor(230, 250, 255, 100));
    p.setColor(QPalette::Dark, progressAndHistogramColor);

    // Info Text
    m_infoLabel = new QLabel();
    m_infoLabel->setPalette(p);
    m_infoLabel->setFrameShape(QFrame::Box);
    m_infoLabel->setMinimumWidth(275);
    m_showInfoText = true;

    // Version info label
    m_versionInfoLabel = new QLabel();
    m_versionInfoLabel->setFrameShape(QFrame::NoFrame);
    m_versionInfoLabel->setAlignment(Qt::AlignRight);
    m_versionInfoLabel->setText(QString("%1 v%2").arg(RI_APPLICATION_NAME, RiaApplication::getVersionStringApp(false)));
    
    QPalette versionInfoPalette = p;
    QColor versionInfoLabelColor = p.color(QPalette::Window);
    versionInfoLabelColor.setAlpha(0);
    versionInfoPalette.setColor(QPalette::Window, versionInfoLabelColor);
    m_versionInfoLabel->setPalette(versionInfoPalette);

    // Animation progress bar
    m_animationProgress = new QProgressBar();
    m_animationProgress->setPalette(p);
    m_animationProgress->setFormat("Time Step: %v/%m");
    m_animationProgress->setTextVisible(true);

    m_progressBarStyle = new QCDEStyle();
    m_animationProgress->setStyle(m_progressBarStyle);
    m_showAnimProgress = false;

    // Histogram
    m_histogramWidget = new RiuSimpleHistogramWidget();
    m_histogramWidget->setPalette(p);
    m_showHistogram = false;

    m_viewerCommands = new RiuViewerCommands(this);

    if (RiaApplication::instance()->isRunningRegressionTests())
    {
        QFont regTestFont = m_infoLabel->font();
        regTestFont.setPixelSize(11);

        m_infoLabel->setFont(regTestFont);
        m_versionInfoLabel->setFont(regTestFont);
        m_animationProgress->setFont(regTestFont);
        m_histogramWidget->setFont(regTestFont);
    }

    // When a context menu is created in the viewer is, and the action triggered is displaying a dialog,
    // the context menu of QMainWindow is displayed after the action has finished
    // Setting this policy will make sure the handling is not deferred to the widget's parent,
    // which solves the problem
    setContextMenuPolicy(Qt::PreventContextMenu);

    m_gridBoxGenerator = new RivGridBoxGenerator;

    m_cursorPositionDomainCoords = cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer::~RiuViewer()
{
    if (m_rimView)
    {
        m_rimView->handleMdiWindowClosed();

        m_rimView->cameraPosition = m_mainCamera->viewMatrix();
        m_rimView->cameraPointOfInterest = pointOfInterest();
    }
    delete m_infoLabel;
    delete m_animationProgress;
    delete m_histogramWidget;
    delete m_progressBarStyle;
    delete m_gridBoxGenerator;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setDefaultView()
{
    cvf::BoundingBox bb = m_mainRendering->boundingBox();
    if (!bb.isValid())
    {
        bb.add(cvf::Vec3d(-1, -1, -1));
        bb.add(cvf::Vec3d( 1,  1,  1));
    }

    if (m_mainCamera->projection() == cvf::Camera::PERSPECTIVE)
    {
        m_mainCamera->setProjectionAsPerspective(40.0, RI_MIN_NEARPLANE_DISTANCE, 1000);
    }
    else
    {
        if (bb.isValid())
        {
            m_mainCamera->setProjectionAsOrtho(bb.extent().length(), RI_MIN_NEARPLANE_DISTANCE, 1000);
        }
    }

    m_mainCamera->fitView(bb, -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (!this->canRender()) return;

    if (event->button() == Qt::LeftButton)
    {
        QPoint diffPoint = event->pos() - m_lastMousePressPosition;
        if (diffPoint.manhattanLength() > 3)
        {
            // We are possibly in navigation mode, only clean press event will launch
            return;
        }

        if (!m_infoLabelOverlayArea.isNull())
        {
            if (m_infoLabelOverlayArea.contains(event->x(), event->y()))
            {
                m_rimView->selectOverlayInfoConfig();

                return;
            }
        }

        m_viewerCommands->handlePickAction(event->x(), event->y(), event->modifiers());

        return;

    }
    else if (event->button() == Qt::RightButton)
    {
        QPoint diffPoint = event->pos() - m_lastMousePressPosition;
        if (diffPoint.manhattanLength() > 3)
        {
            // We are possibly in navigation mode, only clean press event will launch
            return;
        }

        m_viewerCommands->displayContextMenu(event);
        return;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotEndAnimation()
{
    cvf::Rendering* firstRendering = m_mainRendering.p();
    CVF_ASSERT(firstRendering);

    if (m_rimView) m_rimView->endAnimation();
    
    caf::Viewer::slotEndAnimation();

    caf::EffectGenerator::releaseUnreferencedEffects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotSetCurrentFrame(int frameIndex)
{
    setCurrentFrame(frameIndex);

    if (m_rimView)
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        if (viewLinker)
        {
            viewLinker->updateTimeStep(m_rimView, frameIndex);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RiuViewer::pointOfInterest()
{
    return m_navigationPolicy->pointOfInterest();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setPointOfInterest(cvf::Vec3d poi)
{
    m_navigationPolicy->setPointOfInterest(poi);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setOwnerReservoirView(RimView * owner)
{
    m_rimView = owner;
    m_viewerCommands->setOwnerView(owner);

    cvf::String xLabel;
    cvf::String yLabel;
    cvf::String zLabel;

    m_rimView->axisLabels(&xLabel, &yLabel, &zLabel);
    setAxisLabels(xLabel, yLabel, zLabel);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setEnableMask(unsigned int mask)
{
    m_mainRendering->setEnableMask(mask);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::paintOverlayItems(QPainter* painter)
{
    // No support for overlay items using SW rendering yet.
    //if (!isShadersSupported())
    //{
    //    return;
    //}

    int columnWidth = 200;
    int margin = 5;
    int yPos = margin;

    bool showAnimBar = false;
    if (isAnimationActive() && frameCount() > 1) showAnimBar = true;

    //if (showAnimBar)       columnWidth = CVF_MAX(columnWidth, m_animationProgress->width());
    if (m_showInfoText) columnWidth = CVF_MAX(columnWidth, m_infoLabel->sizeHint().width());

    int columnPos = this->width() - columnWidth - margin;

    if (showAnimBar && m_showAnimProgress)
    {
        QString stepName = m_rimView->ownerCase()->timeStepName(currentFrameIndex());
        m_animationProgress->setFormat("Time Step: %v/%m " + stepName);
        m_animationProgress->setMinimum(0);
        m_animationProgress->setMaximum(static_cast<int>(frameCount()) - 1);
        m_animationProgress->setValue(currentFrameIndex());
        m_animationProgress->resize(columnWidth, m_animationProgress->sizeHint().height());

        m_animationProgress->render(painter,QPoint(columnPos, yPos));
        yPos +=  m_animationProgress->height() + margin;

    }

    if (m_showInfoText)
    {
        QPoint topLeft = QPoint(columnPos, yPos);
        m_infoLabel->resize(columnWidth, m_infoLabel->sizeHint().height());
        m_infoLabel->render(painter, topLeft);

        m_infoLabelOverlayArea.setTopLeft(topLeft);
        m_infoLabelOverlayArea.setBottom(yPos + m_infoLabel->height());
        m_infoLabelOverlayArea.setRight(columnPos + columnWidth);

        yPos +=  m_infoLabel->height() + margin;
    }
    else
    {
        m_infoLabelOverlayArea = QRect();
    }

    if (m_showHistogram)
    {
        m_histogramWidget->resize(columnWidth, 40);
        m_histogramWidget->render(painter,QPoint(columnPos, yPos));
        //yPos +=  m_histogramWidget->height() + margin;
    }

    if (m_showInfoText) // Version Label
    {
        QSize size(m_versionInfoLabel->sizeHint().width(), m_versionInfoLabel->sizeHint().height());
        QPoint pos(this->width() - size.width() - margin, this->height() - size.height() - margin);
        m_versionInfoLabel->resize(size.width(), size.height());
        m_versionInfoLabel->render(painter, pos);
    }

    if (!m_cursorPositionDomainCoords.isUndefined())
    {
        if (mainCamera())
        {
            cvf::ref<caf::DisplayCoordTransform> trans = m_rimView->displayCoordTransform();

            cvf::Vec3d displayCoord = trans->transformToDisplayCoord(m_cursorPositionDomainCoords);

            cvf::Vec3d screenCoords;
            if (mainCamera()->project(displayCoord, &screenCoords))
            {
                int translatedMousePosY = height() - screenCoords.y();
                QPoint centerPos(screenCoords.x(), translatedMousePosY);

                // Draw a cross hair marker
                int markerHalfLength = 6;

                painter->drawLine(centerPos.x(), centerPos.y() - markerHalfLength, centerPos.x(), centerPos.y() + markerHalfLength);
                painter->drawLine(centerPos.x() - markerHalfLength, centerPos.y(), centerPos.x() + markerHalfLength, centerPos.y());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setInfoText(QString text)
{
    m_infoLabel->setText(text);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::showInfoText(bool enable)
{
    m_showInfoText = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setHistogram(double min, double max, const std::vector<size_t>& histogram)
{
    m_histogramWidget->setHistogramData(min, max, histogram);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setHistogramPercentiles(double pmin, double pmax, double mean)
{
    m_histogramWidget->setPercentiles(pmin, pmax);
    m_histogramWidget->setMean(mean);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::showAnimationProgress(bool enable)
{
    m_showAnimProgress = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::showHistogram(bool enable)
{
    m_showHistogram = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePressPosition = event->pos();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::removeAllColorLegends()
{
    for (size_t i = 0; i < m_visibleLegends.size(); i++)
    {
        m_mainRendering->removeOverlayItem(m_visibleLegends[i].p());
    }

    m_visibleLegends.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::addColorLegendToBottomLeftCorner(cvf::OverlayItem* legend)
{
    cvf::Rendering* firstRendering = m_mainRendering.p();
    CVF_ASSERT(firstRendering);

    if (legend)
    {
        updateLegendTextAndTickMarkColor(legend);

        firstRendering->addOverlayItem(legend);

        m_visibleLegends.push_back(legend);
    }

    // Category count used to switch between standard height and full height of legend
    const size_t categoryThreshold = 13;

    std::vector<caf::CategoryLegend*> categoryLegends;
    std::vector<cvf::OverlayItem*> overlayItems;
    for (auto legend : m_visibleLegends)
    {
        legend->setLayout(cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_LEFT);

        caf::CategoryLegend* catLegend = dynamic_cast<caf::CategoryLegend*>(legend.p());
        if (catLegend)
        {
            if (catLegend->categoryCount() > categoryThreshold)
            {
                categoryLegends.push_back(catLegend);
            }
            else
            {
                catLegend->setSizeHint(cvf::Vec2ui(200, 200));
            }
        }
        else
        {
            overlayItems.push_back(legend.p());
        }
    }

    if (categoryLegends.size() > 0)
    {
        const int border = 3;
        const int categoryWidth = 150;

        // This value is taken from OverlayAxisCross, as the axis cross is always shown in the lower left corner
        const int axisCrossHeight = 120;

        int height = static_cast<int>(m_mainCamera->viewport()->height());
        int xPos = border;

        int yPos = axisCrossHeight + 2*border;

        for (auto catLegend : categoryLegends)
        {
            catLegend->setLayoutFixedPosition(cvf::Vec2i(xPos, yPos));
            catLegend->setSizeHint(cvf::Vec2ui(categoryWidth, height - 2*border - axisCrossHeight));

            xPos += categoryWidth + border;
        }

        for (auto item : overlayItems)
        {
            item->setLayoutFixedPosition(cvf::Vec2i(xPos, yPos));

            yPos += item->sizeHint().y() + border;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateNavigationPolicy()
{
    switch (RiaApplication::instance()->navigationPolicy())
    {
        case RiaApplication::NAVIGATION_POLICY_CAD:
            setNavigationPolicy(new RiuCadNavigation);
            break;

        case RiaApplication::NAVIGATION_POLICY_CEETRON:
            setNavigationPolicy(new caf::CeetronPlusNavigation);
            break;

        case RiaApplication::NAVIGATION_POLICY_GEOQUEST:
            setNavigationPolicy(new RiuGeoQuestNavigation);
            break;

        case RiaApplication::NAVIGATION_POLICY_RMS:
            setNavigationPolicy(new RiuRmsNavigation);
            break;

        default:
            CVF_ASSERT(0);
            break;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::navigationPolicyUpdate()
{
    caf::Viewer::navigationPolicyUpdate();

    if (m_rimView)
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        if (viewLinker)
        {
            viewLinker->updateCamera(m_rimView);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setCurrentFrame(int frameIndex)
{
    cvf::Rendering* firstRendering = m_mainRendering.p();
    CVF_ASSERT(firstRendering);

    if (m_rimView) m_rimView->setCurrentTimeStepAndUpdate(frameIndex);
    
    animationControl()->setCurrentFrameOnly(frameIndex);

    caf::Viewer::slotSetCurrentFrame(frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RiuViewer::ownerReservoirView()
{
    return m_rimView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuViewer::ownerViewWindow() const
{
    return m_rimView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::optimizeClippingPlanes()
{
    m_gridBoxGenerator->updateFromCamera(mainCamera());

    caf::Viewer::optimizeClippingPlanes();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::resizeGL(int width, int height)
{
    caf::Viewer::resizeGL(width, height);

    bool hasCategoryLegend = false;
    for (size_t i = 0; i < m_visibleLegends.size(); i++)
    {
        caf::CategoryLegend* categoryLegend = dynamic_cast<caf::CategoryLegend*>(m_visibleLegends.at(i));
        if (categoryLegend)
        {
            hasCategoryLegend = true;
        }
    }

    if (hasCategoryLegend)
    {
        m_rimView->updateCurrentTimeStepAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if (m_rimView)
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        if (viewLinker)
        {
            int translatedMousePosX = mouseEvent->pos().x();
            int translatedMousePosY = height() - mouseEvent->pos().y();

            cvf::Vec3d displayCoord(0, 0, 0);
            if (mainCamera()->unproject(cvf::Vec3d(static_cast<double>(translatedMousePosX), static_cast<double>(translatedMousePosY), 0), &displayCoord))
            {
                if (m_cursorPositionDomainCoords != cvf::Vec3d::UNDEFINED)
                {
                    // Reset the extra cursor if the view currently is receiving mouse cursor events
                    // Set undefined and redraw to remove the previously displayed cursor
                    m_cursorPositionDomainCoords = cvf::Vec3d::UNDEFINED;

                    update();
                }

                cvf::ref<caf::DisplayCoordTransform> trans = m_rimView->displayCoordTransform();
                cvf::Vec3d domainCoord = trans->transformToDomainCoord(displayCoord);

                viewLinker->updateCursorPosition(m_rimView, domainCoord);
            }
        }
    }

    caf::Viewer::mouseMoveEvent(mouseEvent);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::leaveEvent(QEvent *)
{
    if (m_rimView && m_rimView->assosiatedViewLinker())
    {
        RimViewLinker* viewLinker = m_rimView->assosiatedViewLinker();
        viewLinker->updateCursorPosition(m_rimView, cvf::Vec3d::UNDEFINED);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateGridBoxData()
{
    if (ownerReservoirView() && ownerReservoirView()->ownerCase())
    {
        RimView* rimView = ownerReservoirView();
        RimCase* rimCase = rimView->ownerCase();

        m_gridBoxGenerator->setScaleZ(rimView->scaleZ);
        m_gridBoxGenerator->setDisplayModelOffset(rimCase->displayModelOffset());
        m_gridBoxGenerator->updateFromBackgroundColor(rimView->backgroundColor);

        if (rimView->showActiveCellsOnly())
        {
            m_gridBoxGenerator->setGridBoxDomainCoordBoundingBox(rimCase->activeCellsBoundingBox());
        }
        else
        {
            m_gridBoxGenerator->setGridBoxDomainCoordBoundingBox(rimCase->allCellsBoundingBox());
        }

        m_gridBoxGenerator->createGridBoxParts();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateAnnotationItems()
{
    updateTextAndTickMarkColorForOverlayItems();

    updateWellPathTextColor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Model* RiuViewer::gridBoxModel() const
{
    return m_gridBoxGenerator->model();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateWellPathTextColor()
{
    RimView* rimView = ownerReservoirView();
    if (!rimView) return;

    cvf::Color3f color = computeContrastColor();

    RimWellPathCollection* wellPathCollection = rimView->wellPathsPartManager();
    if (wellPathCollection)
    {
        for (RimWellPath* path : wellPathCollection->wellPaths())
        {
            path->partMgr()->updateWellLabelTextColor(color);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setAxisLabels(const cvf::String& xLabel, const cvf::String& yLabel, const cvf::String& zLabel)
{
    m_axisCross->setAxisLabels(xLabel, yLabel, zLabel);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RiuViewer::lastPickPositionInDomainCoords() const
{
    CVF_ASSERT(m_viewerCommands);

    return m_viewerCommands->lastPickPositionInDomainCoords();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiuViewer::lastPickedObject() const
{
    CVF_ASSERT(m_viewerCommands);

    return m_viewerCommands->currentPickedObject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::OverlayItem* RiuViewer::pickFixedPositionedLegend(int winPosX, int winPosY)
{
    int translatedMousePosX = winPosX;
    int translatedMousePosY = height() - winPosY;

    for (auto overlayItem : m_visibleLegends)
    {
        if (overlayItem->layoutScheme() == cvf::OverlayItem::FIXED_POSITION &&
            overlayItem->pick(translatedMousePosX, translatedMousePosY, overlayItem->fixedPosition(), overlayItem->sizeHint()))
        {
            return overlayItem.p();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateParallelProjectionSettings(RiuViewer* sourceViewer)
{
    if (!sourceViewer || sourceViewer->m_navigationPolicy.isNull()) return;

    cvf::Vec3d poi = sourceViewer->m_navigationPolicy->pointOfInterest();
    this->updateParallelProjectionHeightFromMoveZoom(poi);
    this->updateParallelProjectionCameraPosFromPointOfInterestMove(poi);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setCursorPosition(const cvf::Vec3d& domainCoord)
{
    if (m_cursorPositionDomainCoords != domainCoord)
    {
        m_cursorPositionDomainCoords = domainCoord;

        update();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateLegendTextAndTickMarkColor(cvf::OverlayItem* legend)
{
    if (m_rimView.isNull()) return;

    cvf::Color3f contrastColor = computeContrastColor();

    cvf::OverlayScalarMapperLegend* scalarMapperLegend = dynamic_cast<cvf::OverlayScalarMapperLegend*>(legend);
    if (scalarMapperLegend)
    {
        scalarMapperLegend->setColor(contrastColor);
        scalarMapperLegend->setLineColor(contrastColor);
    }

    caf::CategoryLegend* categoryLegend = dynamic_cast<caf::CategoryLegend*>(legend);
    if (categoryLegend)
    {
        categoryLegend->setColor(contrastColor);
        categoryLegend->setLineColor(contrastColor);
    }

    RivTernarySaturationOverlayItem* ternaryItem = dynamic_cast<RivTernarySaturationOverlayItem*>(legend);
    if (ternaryItem)
    {
        ternaryItem->setAxisLabelsColor(contrastColor);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateTextAndTickMarkColorForOverlayItems()
{
    for (size_t i = 0; i < m_visibleLegends.size(); i++)
    {
        updateLegendTextAndTickMarkColor(m_visibleLegends.at(i));
    }

    updateAxisCrossTextColor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateAxisCrossTextColor()
{
    cvf::Color3f contrastColor = computeContrastColor();

    m_axisCross->setAxisLabelsColor(contrastColor);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiuViewer::computeContrastColor() const
{
    cvf::Color3f contrastColor = RiaColorTools::brightContrastColor();

    if (m_rimView.notNull())
    {
        contrastColor = RiaColorTools::constrastColor(m_rimView->backgroundColor);
    }
    
    return contrastColor;
}
