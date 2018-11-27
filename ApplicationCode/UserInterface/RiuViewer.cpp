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
#include "RiaPreferences.h"
#include "RiaRegressionTestRunner.h"

#include "RimCase.h"
#include "RimProject.h"
#include "RimGridView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RivGridBoxGenerator.h"
#include "RivTernarySaturationOverlayItem.h"

#include "RiuCadNavigation.h"
#include "RiuGeoQuestNavigation.h"
#include "RiuRmsNavigation.h"
#include "RiuSimpleHistogramWidget.h"
#include "RiuViewerCommands.h"

#include "cafPdmUiSelection3dEditorVisualizer.h"

#include "cafTitledOverlayFrame.h"
#include "cafCategoryLegend.h"
#include "cafOverlayScalarMapperLegend.h"
#include "cafCeetronPlusNavigation.h"
#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafFrameAnimationControl.h"
#include "cafOverlayScaleLegend.h"

#include "cvfCamera.h"
#include "cvfFont.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOverlayAxisCross.h"
#include "cvfOverlayItem.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfRenderQueueSorter.h"
#include "cvfRenderSequence.h"
#include "cvfRendering.h"
#include "cvfScene.h"

#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include "WindowEdgeAxesOverlayItem/RivWindowEdgeAxesOverlayItem.h"
#include <algorithm>


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
: caf::Viewer(format, parent), m_isNavigationRotationEnabled(true)
{
    cvf::Font* standardFont = RiaApplication::instance()->standardFont();
    m_axisCross = new cvf::OverlayAxisCross(m_mainCamera.p(), standardFont);
    m_axisCross->setAxisLabels("X", "Y", "Z");
    m_axisCross->setLayout(cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_RIGHT);
    m_mainRendering->addOverlayItem(m_axisCross.p());
    m_showAxisCross = true;

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
    m_animationProgress->setAlignment(Qt::AlignCenter);

    m_showAnimProgress = false;

    // Histogram
    m_histogramWidget = new RiuSimpleHistogramWidget();
    m_histogramWidget->setPalette(p);
    m_showHistogram = false;

    m_viewerCommands = new RiuViewerCommands(this);

    if (RiaRegressionTestRunner::instance()->isRunningRegressionTests())
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
    m_windowEdgeAxisOverlay = new RivWindowEdgeAxesOverlayItem(standardFont);
    m_showWindowEdgeAxes = false;

    m_selectionVisualizerManager = new caf::PdmUiSelection3dEditorVisualizer(this);

    m_scaleLegend = new caf::OverlayScaleLegend(standardFont);
    m_scaleLegend->setOrientation(caf::OverlayScaleLegend::HORIZONTAL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer::~RiuViewer()
{
    if (m_rimView)
    {
        m_rimView->handleMdiWindowClosed();

        m_rimView->setCameraPosition(m_mainCamera->viewMatrix());
        m_rimView->setCameraPointOfInterest( pointOfInterest());
    }

    delete m_infoLabel;
    delete m_animationProgress;
    delete m_histogramWidget;
    delete m_gridBoxGenerator;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::clearRimView()
{
    m_rimView = nullptr;
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
            viewLinker->updateTimeStep(dynamic_cast<RimGridView*>(m_rimView.p()), frameIndex);
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
void RiuViewer::setOwnerReservoirView(RiuViewerToViewInterface * owner)
{
    m_rimView = owner;

    m_viewerCommands->setOwnerView(dynamic_cast<Rim3dView*>(owner));
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
    int columnWidth = 200;

    int edgeAxisFrameBorderWidth  = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderWidth(): 0;
    int edgeAxisFrameBorderHeight = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderHeight(): 0;

    int margin = 5;
    int yPos = margin + edgeAxisFrameBorderHeight;

    bool showAnimBar = false;
    if (isAnimationActive() && frameCount() > 1) showAnimBar = true;

    if (m_showInfoText) columnWidth = CVF_MAX(columnWidth, m_infoLabel->sizeHint().width());

    int columnPos = this->width() - columnWidth - margin - edgeAxisFrameBorderWidth;

    if (showAnimBar && m_showAnimProgress)
    {
        QString stepName = m_rimView->timeStepName(currentFrameIndex());
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
        QPoint pos(this->width() - size.width() - margin - edgeAxisFrameBorderWidth, this->height() - size.height() - margin - edgeAxisFrameBorderHeight);
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
void RiuViewer::showGridBox(bool enable)
{
    this->removeStaticModel(m_gridBoxGenerator->model());

    if (enable)
    {
        this->addStaticModelOnce(m_gridBoxGenerator->model());
    }
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
void RiuViewer::addColorLegendToBottomLeftCorner(caf::TitledOverlayFrame* addedLegend)
{
    RiaApplication* app = RiaApplication::instance();
    CVF_ASSERT(app);
    RiaPreferences* preferences = app->preferences();
    cvf::Rendering* firstRendering = m_mainRendering.p();
    CVF_ASSERT(preferences);
    CVF_ASSERT(firstRendering);

    if (addedLegend)
    {
        cvf::Color4f backgroundColor = mainCamera()->viewport()->clearColor();
        backgroundColor.a() = 0.8f;
        cvf::Color3f frameColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b());
        updateLegendTextAndTickMarkColor(addedLegend);
        
        firstRendering->addOverlayItem(addedLegend);
        addedLegend->enableBackground(preferences->showLegendBackground());
        addedLegend->setBackgroundColor(backgroundColor);
        addedLegend->setBackgroundFrameColor(cvf::Color4f(RiaColorTools::computeOffsetColor(frameColor, 0.3f), 0.9f));

        m_visibleLegends.push_back(addedLegend);
    }

    updateLegendLayout();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateLegendLayout()
{
    int viewPortHeight = static_cast<int>(m_mainCamera->viewport()->height());

    const float maxFreeLegendHeight = 0.7f*viewPortHeight;
    const int border = 3;
    int edgeAxisBorderWidth  = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderWidth(): 0;
    int edgeAxisBorderHeight = m_showWindowEdgeAxes ? m_windowEdgeAxisOverlay->frameBorderHeight(): 0;

    int xPos = border + edgeAxisBorderWidth;
    int yPos = border + edgeAxisBorderHeight;

    std::vector<caf::TitledOverlayFrame*> standardHeightLegends;
    
    // Place the legends needing the full height, and sort out the standard height legends

    for ( cvf::ref<caf::TitledOverlayFrame> legend : m_visibleLegends )
    {
        cvf::Vec2ui prefSize = legend->preferredSize();
        if (prefSize.y() > maxFreeLegendHeight)
        {
            int legendWidth = prefSize.x();
            legend->setLayoutFixedPosition(cvf::Vec2i(xPos, yPos));
            legend->setRenderSize(cvf::Vec2ui(legendWidth, viewPortHeight - 2 * border - 2 * edgeAxisBorderHeight));
            xPos += legendWidth + border;
        }
        else
        {
            standardHeightLegends.push_back(legend.p());
        }
    }

    // Place the rest of the legends in columns that fits within the screen height
    
    int maxColumnWidht = 0;
    std::vector<caf::TitledOverlayFrame*> columnLegends;

    for ( caf::TitledOverlayFrame* legend : standardHeightLegends )
    {
        cvf::Vec2ui prefSize = legend->preferredSize();

        // Check if we need a new column
        if ((yPos + (int)prefSize.y() + border) > viewPortHeight)
        {
            xPos += border + maxColumnWidht;
            yPos = border + edgeAxisBorderHeight;

            // Set same width to all legends in the column
            for (caf::TitledOverlayFrame* columnLegend : columnLegends )
            {
                columnLegend->setRenderSize(cvf::Vec2ui(maxColumnWidht, columnLegend->renderSize().y())); 
            }
            maxColumnWidht = 0;
            columnLegends.clear();
        }

        legend->setLayoutFixedPosition(cvf::Vec2i(xPos, yPos));
        legend->setRenderSize(cvf::Vec2ui(prefSize.x(),  prefSize.y()));
        columnLegends.push_back(legend);

        yPos += legend->renderSize().y() + border;
        maxColumnWidht = std::max(maxColumnWidht, (int)prefSize.x());
    }

    // Set same width to all legends in the last column

    for (caf::TitledOverlayFrame* legend : columnLegends )
    {
        legend->setRenderSize(cvf::Vec2ui(maxColumnWidht, legend->renderSize().y())); 
    }

    int margin = 5;
    auto scaleLegendSize = m_scaleLegend->renderSize();
    auto otherItemsHeight = m_versionInfoLabel->size().height();
    m_scaleLegend->setLayoutFixedPosition({ width() - (int)scaleLegendSize.x() - margin - edgeAxisBorderWidth,
                                           margin + edgeAxisBorderHeight + margin + otherItemsHeight});
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::enableNavigationRotation(bool enable)
{
    auto tbNavPol = dynamic_cast<caf::TrackBallBasedNavigation*>(m_navigationPolicy.p());
    m_isNavigationRotationEnabled = enable;

    if (tbNavPol) tbNavPol->enableRotation(m_isNavigationRotationEnabled);
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

    enableNavigationRotation(m_isNavigationRotationEnabled);
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
            viewLinker->updateCamera(dynamic_cast<RimGridView*>(m_rimView.p()));
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
void RiuViewer::showAxisCross(bool enable)
{
    m_mainRendering->removeOverlayItem(m_axisCross.p());

    if (enable)
    {
        m_mainRendering->addOverlayItem(m_axisCross.p());
    }
    m_showAxisCross = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewerToViewInterface* RiuViewer::ownerReservoirView()
{
    return m_rimView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuViewer::ownerViewWindow() const
{
    return dynamic_cast<RimViewWindow*>( m_rimView.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::optimizeClippingPlanes()
{
    if (m_showWindowEdgeAxes)
    {
        m_windowEdgeAxisOverlay->setDisplayCoordTransform(m_rimView->displayCoordTransform().p());
        m_windowEdgeAxisOverlay->updateFromCamera(this->mainCamera());
    }

    m_gridBoxGenerator->updateFromCamera(mainCamera());

    m_scaleLegend->setDisplayCoordTransform(m_rimView->displayCoordTransform().p());
    m_scaleLegend->updateFromCamera(mainCamera());

    caf::Viewer::optimizeClippingPlanes();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::resizeGL(int width, int height)
{
    caf::Viewer::resizeGL(width, height);
    this->updateLegendLayout();
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

                viewLinker->updateCursorPosition(dynamic_cast<RimGridView*>(m_rimView.p()) , domainCoord);
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
        viewLinker->updateCursorPosition(dynamic_cast<RimGridView*>(m_rimView.p()), cvf::Vec3d::UNDEFINED);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateGridBoxData(double scaleZ, 
                                  const cvf::Vec3d& displayModelOffset,
                                  const cvf::Color3f&  backgroundColor,
                                  const cvf::BoundingBox& domainCoordBoundingBox)
{
    m_gridBoxGenerator->setScaleZ(scaleZ);
    m_gridBoxGenerator->setDisplayModelOffset(displayModelOffset);
    m_gridBoxGenerator->updateFromBackgroundColor(backgroundColor);
    m_gridBoxGenerator->setGridBoxDomainCoordBoundingBox(domainCoordBoundingBox);

    m_gridBoxGenerator->createGridBoxParts();

    m_selectionVisualizerManager->updateVisibleEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showEdgeTickMarksXY(bool enable, bool showAxisLines)
{
    m_mainRendering->removeOverlayItem(m_windowEdgeAxisOverlay.p());

    if (enable)
    {
        m_windowEdgeAxisOverlay->setDomainAxes(RivWindowEdgeAxesOverlayItem::XY_AXES);
        m_windowEdgeAxisOverlay->setIsSwitchingYAxisSign(false);
        m_windowEdgeAxisOverlay->setShowAxisLines(showAxisLines);
        m_mainRendering->addOverlayItem(m_windowEdgeAxisOverlay.p());
    }
    m_showWindowEdgeAxes = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::showEdgeTickMarksXZ(bool enable, bool showAxisLines)
{
    m_mainRendering->removeOverlayItem(m_windowEdgeAxisOverlay.p());

    if (enable)
    {
        m_windowEdgeAxisOverlay->setDomainAxes(RivWindowEdgeAxesOverlayItem::XZ_AXES);
        m_windowEdgeAxisOverlay->setIsSwitchingYAxisSign(true);
        m_windowEdgeAxisOverlay->setShowAxisLines(showAxisLines);
        m_mainRendering->addOverlayItem(m_windowEdgeAxisOverlay.p());
    }
    m_showWindowEdgeAxes = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateAnnotationItems()
{
    updateTextAndTickMarkColorForOverlayItems();
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
cvf::OverlayItem* RiuViewer::pickFixedPositionedLegend(int winPosX, int winPosY)
{
    int translatedMousePosX = winPosX;
    int translatedMousePosY = height() - winPosY;

    for (auto overlayItem : m_visibleLegends)
    {
        if (overlayItem->layoutScheme() == cvf::OverlayItem::FIXED_POSITION &&
            overlayItem->pick(translatedMousePosX, translatedMousePosY, overlayItem->fixedPosition(), overlayItem->renderSize()))
        {
            return overlayItem.p();
        }
    }

    return nullptr;
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
std::vector<cvf::ref<cvf::Part>> RiuViewer::visibleParts()
{
    std::vector<cvf::ref<cvf::Part>> partsMatchingEnableMask;

    if (m_mainRendering.notNull())
    {
        auto enableMask = m_mainRendering->enableMask();
        cvf::Scene* scene = currentScene();

        for (cvf::uint i = 0; i < scene->modelCount(); i++)
        {
            cvf::Model* model = scene->model(i);
            if (enableMask & model->partEnableMask())
            {
                cvf::Collection<cvf::Part> partCollection;
                model->allParts(&partCollection);

                for (const auto& p : partCollection)
                {
                    if (enableMask & p->enableMask())
                    {
                        partsMatchingEnableMask.push_back(p);
                    }
                }
            }
        }
    }

    return partsMatchingEnableMask;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewer::showScaleLegend(bool show)
{
    if (show)
    {
        if(m_scaleLegend->orientation() == caf::OverlayScaleLegend::HORIZONTAL)
            m_scaleLegend->setRenderSize({280, 45});
        else
            m_scaleLegend->setRenderSize({50, 280});

        m_mainRendering->addOverlayItem(m_scaleLegend.p());
    }
    else
    {
        m_mainRendering->removeOverlayItem(m_scaleLegend.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::updateLegendTextAndTickMarkColor(cvf::OverlayItem* legend)
{
    if (m_rimView.isNull()) return;

    cvf::Color3f contrastColor = computeContrastColor();

    caf::OverlayScalarMapperLegend* scalarMapperLegend = dynamic_cast<caf::OverlayScalarMapperLegend*>(legend);
    if (scalarMapperLegend)
    {
        scalarMapperLegend->setTextColor(contrastColor);
        scalarMapperLegend->setLineColor(contrastColor);
    }

    caf::CategoryLegend* categoryLegend = dynamic_cast<caf::CategoryLegend*>(legend);
    if (categoryLegend)
    {
        categoryLegend->setTextColor(contrastColor);
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
        contrastColor = RiaColorTools::constrastColor(m_rimView->backgroundColor());
    }
    
    return contrastColor;
}
