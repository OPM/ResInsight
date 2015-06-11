/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuViewerCommands.h"
#include "RiuViewer.h"
#include "RimUiTreeModelPdm.h"
#include "RimCellRangeFilter.h"
#include "RimView.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimFaultCollection.h"

#include "RivSourceInfo.h"
#include "RivFemPickSourceInfo.h"
#include "RivFemPartGeometryGenerator.h"
#include "RigCaseData.h"
#include "RiuMainWindow.h"
#include "RiaApplication.h"
#include "RiuResultTextBuilder.h"
#include "RigGeomechCaseData.h"
#include "RimGeoMechCase.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"

#include "cvfDrawableGeo.h"
#include "cvfPart.h"
#include "cvfHitItemCollection.h"

#include <QMenu>
#include <QMouseEvent>

#include <QStatusBar>
#include "RiuFemResultTextBuilder.h"

//==================================================================================================
//
// RiaViewerCommands
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewerCommands::RiuViewerCommands(RiuViewer* ownerViewer) : QObject(ownerViewer), m_viewer(ownerViewer)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewerCommands::~RiuViewerCommands()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::setOwnerView(RimView * owner)
{
    m_reservoirView = owner;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::displayContextMenu(QMouseEvent* event)
{
    m_currentGridIdx = cvf::UNDEFINED_SIZE_T;
    m_currentCellIndex = cvf::UNDEFINED_SIZE_T;

    int winPosX = event->x();
    int winPosY = event->y();

    uint faceIndex = cvf::UNDEFINED_UINT;
    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);

    cvf::Part* firstHitPart = NULL;
    cvf::Part* nncFirstHitPart = NULL;

    cvf::HitItemCollection hitItems;

    if (m_viewer->rayPick(winPosX, winPosY, &hitItems))
    {
        extractIntersectionData(hitItems, &localIntersectionPoint, &firstHitPart, &faceIndex, &nncFirstHitPart, NULL);
    }

    if (!firstHitPart) return;
    
    if (faceIndex == cvf::UNDEFINED_UINT) return;

    if (!firstHitPart->sourceInfo()) return;
            
    const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
    const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());

    if (!(rivSourceInfo || femSourceInfo) ) return;
    
    if (rivSourceInfo)
    {
        if (!rivSourceInfo->hasCellFaceMapping()) return;

        // Set the data regarding what was hit

        m_currentGridIdx = rivSourceInfo->gridIndex();
        m_currentCellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex(faceIndex);
        m_currentFaceIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace(faceIndex);
    }
    else
    {
        m_currentGridIdx = femSourceInfo->femPartIndex();
        m_currentCellIndex = femSourceInfo->triangleToElmMapper()->elementIndex(faceIndex);
    }

    // IJK -slice commands

    QMenu menu;

    menu.addAction(QString("I-slice range filter"), this, SLOT(slotRangeFilterI()));
    menu.addAction(QString("J-slice range filter"), this, SLOT(slotRangeFilterJ()));
    menu.addAction(QString("K-slice range filter"), this, SLOT(slotRangeFilterK()));

    // Hide faults command
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    if (eclipseView)
    {
        const RigCaseData* reservoir = eclipseView->eclipseCase()->reservoirData();
        const RigFault* fault = reservoir->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
        if (fault)
        {
            menu.addSeparator();

            QString faultName = fault->name();
            menu.addAction(QString("Hide ") + faultName, this, SLOT(slotHideFault()));
        }
    }

    menu.exec(event->globalPos());
}

//--------------------------------------------------------------------------------------------------
/// Todo: Move this to a command instead
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotRangeFilterI()
{
    createSliceRangeFilter(0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotRangeFilterJ()
{
    createSliceRangeFilter(1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotRangeFilterK()
{
    createSliceRangeFilter(2);
}

void RiuViewerCommands::createSliceRangeFilter(int ijOrk)
{
    RimView* eclipseView = m_reservoirView.p();
    if (!eclipseView) return;

    size_t i, j, k;
    ijkFromCellIndex(m_currentGridIdx, m_currentCellIndex, &i, &j, &k);

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    RimUiTreeModelPdm* myModel = mainWindow->uiPdmModel();
    if (myModel)
    {
        RimCellRangeFilterCollection* rangeFilterCollection = eclipseView->rangeFilterCollection();

        QModelIndex collectionModelIndex = myModel->getModelIndexFromPdmObject(rangeFilterCollection);

        QModelIndex insertedIndex;
        RimCellRangeFilter* rangeFilter = myModel->addRangeFilter(collectionModelIndex, insertedIndex);

        if (ijOrk == 0){
            rangeFilter->name = QString("Slice I (%1)").arg(rangeFilterCollection->rangeFilters().size());
            rangeFilter->cellCountI = 1;
            int startIndex = CVF_MAX(static_cast<int>(i + 1), 1);
            rangeFilter->startIndexI = startIndex;

        }
        else if (ijOrk == 1){
            rangeFilter->name = QString("Slice J (%1)").arg(rangeFilterCollection->rangeFilters().size());
            rangeFilter->cellCountJ = 1;
            int startIndex = CVF_MAX(static_cast<int>(j + 1), 1);
            rangeFilter->startIndexJ = startIndex;

        }
        else if (ijOrk == 2){
            rangeFilter->name = QString("Slice K (%1)").arg(rangeFilterCollection->rangeFilters().size());
            rangeFilter->cellCountK = 1;
            int startIndex = CVF_MAX(static_cast<int>(k + 1), 1);
            rangeFilter->startIndexK = startIndex;
        }

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        mainWindow->setCurrentObjectInTreeView(rangeFilter);
    }

    eclipseView->setSurfaceDrawstyle();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotHideFault()
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    if(!eclipseView) return;


    const RigCaseData* reservoir = eclipseView->eclipseCase()->reservoirData();
    const RigFault* fault = reservoir->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
    if (fault)
    {
        QString faultName = fault->name();

        RimFault* rimFault = eclipseView->faultCollection()->findFaultByName(faultName);
        if (rimFault)
        {
            rimFault->showFault.setValueFromUi(!rimFault->showFault);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::handlePickAction(int winPosX, int winPosY)
{

    size_t gridIndex = cvf::UNDEFINED_SIZE_T;
    size_t cellIndex = cvf::UNDEFINED_SIZE_T;
    size_t nncIndex = cvf::UNDEFINED_SIZE_T;
    cvf::StructGridInterface::FaceType face = cvf::StructGridInterface::NO_FACE;
    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);

    // Extract all the above information from the pick
    {
        cvf::Part* firstHitPart = NULL;
        uint firstPartTriangleIndex = cvf::UNDEFINED_UINT;

        cvf::Part* firstNncHitPart = NULL;
        uint nncPartTriangleIndex = cvf::UNDEFINED_UINT;

        cvf::HitItemCollection hitItems;

        if (m_viewer->rayPick(winPosX, winPosY, &hitItems))
        {
            extractIntersectionData(hitItems, &localIntersectionPoint, &firstHitPart, &firstPartTriangleIndex, &firstNncHitPart, &nncPartTriangleIndex);
        }

        if (firstHitPart && firstHitPart->sourceInfo())
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
            const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());

            if (rivSourceInfo)
            {
                gridIndex = rivSourceInfo->gridIndex();
                if (rivSourceInfo->hasCellFaceMapping())
                {
                    CVF_ASSERT(rivSourceInfo->m_cellFaceFromTriangleMapper.notNull());

                    cellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex(firstPartTriangleIndex);
                    face = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace(firstPartTriangleIndex);
                }
            }
            else if (femSourceInfo)
            {
                gridIndex = femSourceInfo->femPartIndex();
                cellIndex = femSourceInfo->triangleToElmMapper()->elementIndex(firstPartTriangleIndex);
            }
        }
       


        if (firstNncHitPart && firstNncHitPart->sourceInfo())
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstNncHitPart->sourceInfo());
            if (rivSourceInfo)
            {
                if (nncPartTriangleIndex < rivSourceInfo->m_NNCIndices->size())
                {
                    nncIndex = rivSourceInfo->m_NNCIndices->get(nncPartTriangleIndex);
                }
            }
        }
    }

    // Compose a info text regarding the hit

    QString pickInfo = "No hits";
    QString resultInfo = "";

    if (cellIndex != cvf::UNDEFINED_SIZE_T)
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
        RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());

        if (eclipseView)
        {
            RiuResultTextBuilder textBuilder(eclipseView, gridIndex, cellIndex, eclipseView->currentTimeStep());
            textBuilder.setFace(face);
            textBuilder.setNncIndex(nncIndex);
            textBuilder.setIntersectionPoint(localIntersectionPoint);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.topologyText(", ");
        }
        else if (geomView)
        {
            RiuFemResultTextBuilder textBuilder(geomView, (int)gridIndex, (int)cellIndex, geomView->currentTimeStep());
            //textBuilder.setFace(face);
           
            textBuilder.setIntersectionPoint(localIntersectionPoint);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.topologyText(", ");
        }
    }

    // Display the text

    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    mainWnd->statusBar()->showMessage(pickInfo);
    mainWnd->setResultInfo(resultInfo);
}

//--------------------------------------------------------------------------------------------------
/// Perform picking and return the index of the face that was hit, if a drawable geo was hit
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::extractIntersectionData(const cvf::HitItemCollection& hitItems, 
                                        cvf::Vec3d* localIntersectionPoint, cvf::Part** firstPart, uint* firstPartFaceHit, 
                                        cvf::Part** nncPart, uint* nncPartFaceHit)
{
    CVF_ASSERT(hitItems.count() > 0);

    double pickDepthThresholdSquared = 0.05 *0.05;
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());

        if (eclipseView && eclipseView->eclipseCase())
        {
            double characteristicCellSize = eclipseView->eclipseCase()->reservoirData()->mainGrid()->characteristicIJCellSize();
            pickDepthThresholdSquared = characteristicCellSize / 100.0;
            pickDepthThresholdSquared = pickDepthThresholdSquared * pickDepthThresholdSquared;
        }
    }

    const cvf::HitItem* firstNonNncHitItem = NULL;
    cvf::Vec3d firstItemIntersectionPoint = hitItems.item(0)->intersectionPoint();

    // Check if we have a close hit item with NNC data

    for (size_t i = 0; i < hitItems.count(); i++)
    {
        const cvf::HitItem* hitItemCandidate = hitItems.item(i);
        cvf::Vec3d diff = firstItemIntersectionPoint - hitItemCandidate->intersectionPoint();

        const cvf::Part* pickedPartCandidate = hitItemCandidate->part();
        bool isNncpart = false;

        if (pickedPartCandidate && pickedPartCandidate->sourceInfo())
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(pickedPartCandidate->sourceInfo());
            if (rivSourceInfo && rivSourceInfo->hasNNCIndices())
            {

                // Hit items are ordered by distance from eye
                if (diff.lengthSquared() < pickDepthThresholdSquared)
                {
                    *nncPart = const_cast<cvf::Part*>(pickedPartCandidate);

                    const cvf::HitDetailDrawableGeo* detail = dynamic_cast<const cvf::HitDetailDrawableGeo*>(hitItemCandidate->detail());
                    if (detail && nncPartFaceHit)
                    {
                        *nncPartFaceHit = detail->faceIndex();
                    }

                    isNncpart = true;
                }
            }
        }

        if (!isNncpart && !firstNonNncHitItem)
        {
            firstNonNncHitItem = hitItemCandidate;
            firstItemIntersectionPoint = firstNonNncHitItem->intersectionPoint();
        }

        if (firstNonNncHitItem && *nncPart)
        {
            break;
        }
    }

    const cvf::Part* pickedPart = firstNonNncHitItem->part();
    CVF_ASSERT(pickedPart);
    *firstPart = const_cast<cvf::Part*>(pickedPart);

    const cvf::Transform* xf = pickedPart->transform();
    cvf::Vec3d globalPickedPoint = firstNonNncHitItem->intersectionPoint();

    if (localIntersectionPoint)
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

    if (firstPartFaceHit)
    {
        const cvf::HitDetailDrawableGeo* detail = dynamic_cast<const cvf::HitDetailDrawableGeo*>(firstNonNncHitItem->detail());
        if (detail)
        {
            *firstPartFaceHit = detail->faceIndex();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::ijkFromCellIndex(size_t gridIdx, size_t cellIndex,  size_t* i, size_t* j, size_t* k)
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());


    if (eclipseView && eclipseView->eclipseCase())
    {
        eclipseView->eclipseCase()->reservoirData()->grid(gridIdx)->ijkFromCellIndex(cellIndex, i, j, k);
    }
    
    if (geomView && geomView->geoMechCase())
    {
        geomView->geoMechCase()->geoMechData()->femParts()->part(gridIdx)->structGrid()->ijkFromCellIndex(cellIndex, i, j, k);
    }
}


