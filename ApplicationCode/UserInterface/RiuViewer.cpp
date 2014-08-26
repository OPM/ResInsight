/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "RiuViewer.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"

#include "RimReservoirView.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimCellRangeFilterCollection.h"
#include "RimFaultCollection.h"
#include "RimResultSlot.h"
#include "RimWellCollection.h"

#include "RimUiTreeModelPdm.h"

#include "RimReservoirCellResultsStorage.h"

#include "RigCaseData.h"

#include "RiuSimpleHistogramWidget.h"

#include "cafNavigationPolicy.h"
#include "cafEffectGenerator.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "RivSourceInfo.h"

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
    m_showInfoText = true;

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

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer::~RiuViewer()
{
    m_reservoirView->showWindow = false;
    m_reservoirView->cameraPosition = m_mainCamera->viewMatrix();

    delete m_InfoLabel;
    delete m_animationProgress;
    delete m_histogramWidget;
    delete m_progressBarStyle;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setDefaultView()
{
    cvf::BoundingBox bb;

    cvf::Scene* scene = m_renderingSequence->firstRendering()->scene();
    if (scene)
    {
        bb = scene->boundingBox();
    }

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
    m_mouseState.updateFromMouseEvent(event);

    if (!this->canRender()) return;

    // Picking
    if (event->button() == Qt::LeftButton)
    {
        handlePickAction(event->x(), event->y());
        return;
    }
    else if (event->button() == Qt::RightButton)
    {
        m_currentGridIdx = cvf::UNDEFINED_SIZE_T;
        m_currentCellIndex = cvf::UNDEFINED_SIZE_T;

        QPoint diffPoint = event->pos() - m_lastMousePressPosition;
        if (diffPoint.manhattanLength() > 3)
        {
            // We are possibly in navigation mode, only clean press event will launch
            return;
        }

        int winPosX = event->x();
        int winPosY = event->y();

        uint faceIndex = cvf::UNDEFINED_UINT;
        cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);

        cvf::Part * firstHitPart = NULL;
        firstHitPart = pickPointAndFace(winPosX, winPosY, &faceIndex, &localIntersectionPoint);
        if (firstHitPart)
        {
            if (faceIndex != cvf::UNDEFINED_UINT)
            {
                if (firstHitPart->sourceInfo())
                {
                    const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
                    if (rivSourceInfo)
                    {
                        if (rivSourceInfo->hasCellFaceMapping())
                        {
                            m_currentGridIdx = firstHitPart->id();
                            m_currentCellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex(faceIndex);
                            m_currentFaceIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace(faceIndex);

                            QMenu menu;

                            menu.addAction(QString("I-slice range filter"), this, SLOT(slotRangeFilterI()));
                            menu.addAction(QString("J-slice range filter"), this, SLOT(slotRangeFilterJ()));
                            menu.addAction(QString("K-slice range filter"), this, SLOT(slotRangeFilterK()));

                            const RigCaseData* reservoir = m_reservoirView->eclipseCase()->reservoirData();
                            const RigFault* fault = reservoir->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
                            if (fault)
                            {
                                menu.addSeparator();

                                QString faultName = fault->name();
                                menu.addAction(QString("Hide ") + faultName, this, SLOT(slotHideFault()));
                            }

                            menu.exec(event->globalPos());
                        }
                    }
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotRangeFilterI()
{
    size_t i, j, k;
    ijkFromCellIndex(m_currentGridIdx, m_currentCellIndex, &i, &j, &k);

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    RimUiTreeModelPdm* myModel = mainWindow->uiPdmModel();
    if (myModel)
    {
        RimCellRangeFilterCollection* rangeFilterCollection = m_reservoirView->rangeFilterCollection();

        QModelIndex collectionModelIndex = myModel->getModelIndexFromPdmObject(rangeFilterCollection);

        QModelIndex insertedIndex;
        RimCellRangeFilter* rangeFilter = myModel->addRangeFilter(collectionModelIndex, insertedIndex);

        rangeFilter->name = QString("Slice I (%1)").arg(rangeFilterCollection->rangeFilters().size());
        rangeFilter->cellCountI = 1;
        int startIndex = CVF_MAX(static_cast<int>(i + 1), 1);
        rangeFilter->startIndexI = startIndex;

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        mainWindow->setCurrentObjectInTreeView(rangeFilter);
    }

    m_reservoirView->setShowFaultsOnly(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotRangeFilterJ()
{
    size_t i, j, k;
    ijkFromCellIndex(m_currentGridIdx, m_currentCellIndex, &i, &j, &k);

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    RimUiTreeModelPdm* myModel = mainWindow->uiPdmModel();
    if (myModel)
    {
        RimCellRangeFilterCollection* rangeFilterCollection = m_reservoirView->rangeFilterCollection();

        QModelIndex collectionModelIndex = myModel->getModelIndexFromPdmObject(rangeFilterCollection);

        QModelIndex insertedIndex;
        RimCellRangeFilter* rangeFilter = myModel->addRangeFilter(collectionModelIndex, insertedIndex);

        rangeFilter->name = QString("Slice J (%1)").arg(rangeFilterCollection->rangeFilters().size());
        rangeFilter->cellCountJ = 1;
        int startIndex = CVF_MAX(static_cast<int>(j + 1), 1);
        rangeFilter->startIndexJ = startIndex;

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        mainWindow->setCurrentObjectInTreeView(rangeFilter);
    }

    m_reservoirView->setShowFaultsOnly(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotRangeFilterK()
{
    size_t i, j, k;
    ijkFromCellIndex(m_currentGridIdx, m_currentCellIndex, &i, &j, &k);

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    RimUiTreeModelPdm* myModel = mainWindow->uiPdmModel();
    if (myModel)
    {
        RimCellRangeFilterCollection* rangeFilterCollection = m_reservoirView->rangeFilterCollection();

        QModelIndex collectionModelIndex = myModel->getModelIndexFromPdmObject(rangeFilterCollection);

        QModelIndex insertedIndex;
        RimCellRangeFilter* rangeFilter = myModel->addRangeFilter(collectionModelIndex, insertedIndex);

        rangeFilter->name = QString("Slice K (%1)").arg(rangeFilterCollection->rangeFilters().size());
        rangeFilter->cellCountK = 1;
        int startIndex = CVF_MAX(static_cast<int>(k + 1), 1);
        rangeFilter->startIndexK = startIndex;

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        mainWindow->setCurrentObjectInTreeView(rangeFilter);
    }

    m_reservoirView->setShowFaultsOnly(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::keyPressEvent(QKeyEvent* event)
{
    // Trap escape key so we can get out of direct button press actions
    if (event->key() == Qt::Key_Escape)
    {

    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::handlePickAction(int winPosX, int winPosY)
{
    RiaApplication* app = RiaApplication::instance();

    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    QString pickInfo = "No hits";
    QString resultInfo = "";

    uint faceIndex = cvf::UNDEFINED_UINT;
    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);

    cvf::Part * firstHitPart = NULL;
    firstHitPart = pickPointAndFace(winPosX, winPosY, &faceIndex, &localIntersectionPoint);
    if (firstHitPart)
    {
        // If a drawable geometry was hit, get info about the picked geometry
        // and possibly the picked scalar value, if any
        if (faceIndex != cvf::UNDEFINED_UINT)
        {
            size_t gridIndex = firstHitPart->id();

            if (firstHitPart->sourceInfo())
            {
                const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
                if (rivSourceInfo)
                {
                    if (rivSourceInfo->hasCellFaceMapping())
                    {
                        CVF_ASSERT(rivSourceInfo->m_cellFaceFromTriangleMapper.notNull());

                        size_t cellIndex = cvf::UNDEFINED_SIZE_T;
                        cellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex(faceIndex);
                       
                        cvf::StructGridInterface::FaceType face = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace(faceIndex);

                        m_reservoirView->pickInfo(gridIndex, cellIndex, face, localIntersectionPoint, ", ", &pickInfo);

                        // Build up result from from both pick info and result values
                        m_reservoirView->pickInfo(gridIndex, cellIndex, face, localIntersectionPoint, "\n", &resultInfo);
                        resultInfo += "\n";
                        m_reservoirView->appendCellResultInfo(gridIndex, cellIndex, face, &resultInfo);
#if 0
                        const RigCaseData* reservoir = m_reservoirView->eclipseCase()->reservoirData();
                        const RigGridBase* grid = reservoir->grid(gridIndex);
                        const RigCell& cell = grid->cell(cellIndex);
                        const caf::SizeTArray8& cellNodeIndices = cell.cornerIndices();
                        const std::vector<cvf::Vec3d>& nodes = reservoir->mainGrid()->nodes();
                        for (int i = 0; i < 8; ++i)
                        {
                            resultInfo += QString::number(i) + " : ";
                            for (int j = 0; j < 3; ++j)
                                resultInfo += QString::number(nodes[cellNodeIndices[i]][j], 'g', 10) + " ";
                             resultInfo += "\n";
                        }
#endif
                    }
                    else if (rivSourceInfo->m_NNCIndices.notNull())
                    {
                        size_t nncIndex = rivSourceInfo->m_NNCIndices->get(faceIndex);

                        m_reservoirView->appendNNCResultInfo(nncIndex, &resultInfo);
                    }
                }
            }
        }
    }

    mainWnd->statusBar()->showMessage(pickInfo);
    mainWnd->setResultInfo(resultInfo);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotEndAnimation()
{
    cvf::Rendering* firstRendering = m_renderingSequence->firstRendering();
    CVF_ASSERT(firstRendering);

    if (m_reservoirView) m_reservoirView->endAnimation();
    
    caf::Viewer::slotEndAnimation();

    caf::EffectGenerator::releaseUnreferencedEffects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::slotSetCurrentFrame(int frameIndex)
{
    cvf::Rendering* firstRendering = m_renderingSequence->firstRendering();
    CVF_ASSERT(firstRendering);

    if (m_reservoirView) m_reservoirView->setCurrentTimeStep(frameIndex);

    caf::Viewer::slotSetCurrentFrame(frameIndex);
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
void RiuViewer::setOwnerReservoirView(RimReservoirView * owner)
{
    m_reservoirView = owner;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewer::setEnableMask(unsigned int mask)
{
    m_mainRendering->setEnableMask(mask);
}

//--------------------------------------------------------------------------------------------------
/// Perform picking and return the index of the face that was hit, if a drawable geo was hit
//--------------------------------------------------------------------------------------------------
cvf::Part* RiuViewer::pickPointAndFace(int winPosX, int winPosY, uint* faceHit, cvf::Vec3d* localIntersectionPoint)
{
    CVF_ASSERT(faceHit);

    cvf::HitItemCollection hitItems;
    bool isSomethingHit = rayPick(winPosX, winPosY, &hitItems);

    if (isSomethingHit)
    {
        CVF_ASSERT(hitItems.count() > 0);

        double characteristicCellSize = m_reservoirView->eclipseCase()->reservoirData()->mainGrid()->characteristicIJCellSize();
        double pickDepthThresholdSquared = characteristicCellSize / 100.0;
        pickDepthThresholdSquared = pickDepthThresholdSquared * pickDepthThresholdSquared;

        cvf::HitItem* hitItem = hitItems.firstItem();
        cvf::Vec3d firstItemIntersectionPoint = hitItem->intersectionPoint();

        // Check if we have a close hit item with NNC data
        for (size_t i = 0; i < hitItems.count(); i++)
        {
            cvf::HitItem* hitItemCandidate = hitItems.item(i);
            cvf::Vec3d diff = firstItemIntersectionPoint - hitItemCandidate->intersectionPoint();

            // Hit items are ordered by distance from eye
            if (diff.lengthSquared() > pickDepthThresholdSquared) break;

            const cvf::Part* pickedPartCandidate = hitItemCandidate->part();
            if (pickedPartCandidate && pickedPartCandidate->sourceInfo())
            {
                const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(pickedPartCandidate->sourceInfo());
                if (rivSourceInfo && rivSourceInfo->hasNNCIndices())
                {
                    hitItem = hitItemCandidate;
                    break;
                }
            }
        }

        const cvf::Part* pickedPart = hitItem->part();
        CVF_ASSERT(pickedPart);

        const cvf::Transform* xf = pickedPart->transform();
        cvf::Vec3d globalPickedPoint = hitItem->intersectionPoint();

        if(localIntersectionPoint) 
        {
            if (xf)
            {
                *localIntersectionPoint = globalPickedPoint.getTransformedPoint(xf->worldTransform().getInverted());
            }
            else
            {
                *localIntersectionPoint = globalPickedPoint;
            }
        }

        if (faceHit)
        {

            const cvf::HitDetailDrawableGeo* detail = dynamic_cast<const cvf::HitDetailDrawableGeo*>(hitItem->detail());
            if (detail)
            {
                *faceHit = detail->faceIndex();
            }
        }

        return const_cast<cvf::Part*>(pickedPart); // Hack. The const'ness of HitItem will probably change to non-const
    }
    else
    {
        return NULL;
    }
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
        yPos +=  m_InfoLabel->height() + margin;
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
void RiuViewer::ijkFromCellIndex(size_t gridIdx, size_t cellIndex,  size_t* i, size_t* j, size_t* k)
{
    if (m_reservoirView->eclipseCase())
    {
        m_reservoirView->eclipseCase()->reservoirData()->grid(gridIdx)->ijkFromCellIndex(cellIndex, i, j, k);
    }
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
void RiuViewer::slotHideFault()
{
    const RigCaseData* reservoir = m_reservoirView->eclipseCase()->reservoirData();
    const RigFault* fault = reservoir->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
    if (fault)
    {
        QString faultName = fault->name();

        RimFault* rimFault = m_reservoirView->faultCollection()->findFaultByName(faultName);
        if (rimFault)
        {
            rimFault->showFault.setValueFromUi(!rimFault->showFault);
        }
    }
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

