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

#include "RimCase.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RivGridBoxGenerator.h"

#include "RiuCadNavigation.h"
#include "RiuGeoQuestNavigation.h"
#include "RiuRmsNavigation.h"
#include "RiuSimpleHistogramWidget.h"
#include "RiuViewerCommands.h"

#include "cafCeetronPlusNavigation.h"
#include "cafEffectGenerator.h"

#include "cvfCamera.h"
#include "cvfFont.h"
#include "cvfOverlayAxisCross.h"
#include "cvfRenderQueueSorter.h"
#include "cvfRenderSequence.h"
#include "cvfRendering.h"
#include "cvfScene.h"
#include "cvfOpenGLResourceManager.h"

#include <QCDEStyle>
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include "RimCase.h"

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
    cvf::OverlayAxisCross* axisCross = new cvf::OverlayAxisCross(m_mainCamera.p(), standardFont);
    axisCross->setAxisLabels("E", "N", "Z");
    axisCross->setLayout(cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_LEFT);
    m_mainRendering->addOverlayItem(axisCross);

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
    m_InfoLabel = new QLabel();
    m_InfoLabel->setPalette(p);
    m_InfoLabel->setFrameShape(QFrame::Box);
    m_InfoLabel->setMinimumWidth(275);
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
        QFont regTestFont = m_InfoLabel->font();
        regTestFont.setPixelSize(11);

        m_InfoLabel->setFont(regTestFont);
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
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer::~RiuViewer()
{
    if (m_rimView)
    {
        m_rimView->showWindow = false;
        m_rimView->uiCapability()->updateUiIconFromToggleField();

        m_rimView->cameraPosition = m_mainCamera->viewMatrix();
    }
    delete m_InfoLabel;
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
    cvf::BoundingBox bb = m_renderingSequence->boundingBox();
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
    cvf::Rendering* firstRendering = m_renderingSequence->firstRendering();
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
    if (m_showInfoText) columnWidth = CVF_MAX(columnWidth, m_InfoLabel->sizeHint().width());

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
        m_InfoLabel->resize(columnWidth, m_InfoLabel->sizeHint().height());
        m_InfoLabel->render(painter, QPoint(columnPos, yPos));
        yPos +=  m_InfoLabel->height() + margin;
    }

    if (m_showHistogram)
    {
        m_histogramWidget->resize(columnWidth, 40);
        m_histogramWidget->render(painter,QPoint(columnPos, yPos));
        yPos +=  m_histogramWidget->height() + margin;
    }

    if (m_showInfoText)
    {
        QSize size(m_versionInfoLabel->sizeHint().width(), m_versionInfoLabel->sizeHint().height());
        QPoint pos(this->width() - size.width() - margin, this->height() - size.height() - margin);
        m_versionInfoLabel->resize(size.width(), size.height());
        m_versionInfoLabel->render(painter, pos);
        yPos +=  size.height() + margin;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setInfoText(QString text)
{
    m_InfoLabel->setText(text);
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
    cvf::Rendering* firstRendering = m_renderingSequence->firstRendering();
    CVF_ASSERT(firstRendering);

    if (legend)
    {
        legend->setLayout(cvf::OverlayItem::VERTICAL, cvf::OverlayItem::BOTTOM_LEFT);
        firstRendering->addOverlayItem(legend);

        m_visibleLegends.push_back(legend);
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
    cvf::Rendering* firstRendering = m_renderingSequence->firstRendering();
    CVF_ASSERT(firstRendering);

    if (m_rimView) m_rimView->setCurrentTimeStep(frameIndex);

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
void RiuViewer::optimizeClippingPlanes()
{
    caf::Viewer::optimizeClippingPlanes();

    m_gridBoxGenerator->updateFromCamera(mainCamera());
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
cvf::Model* RiuViewer::gridBoxModel() const
{
    return m_gridBoxGenerator->model();
}
