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

#include "RicEclipsePropertyFilterNewExec.h"
#include "RicGeoMechPropertyFilterNewExec.h"
#include "RicRangeFilterNewExec.h"
#include "Commands/WellLogCommands/RicNewWellLogFileCurveFeature.h"
#include "Commands/WellLogCommands/RicNewWellLogCurveExtractionFeature.h"

#include "RigCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"

#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimFaultCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimGeoMechView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellLogFile.h"

#include "RiuFemResultTextBuilder.h"
#include "RiuMainWindow.h"
#include "RiuResultTextBuilder.h"
#include "RiuViewer.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmUiTreeView.h"

#include "cvfDrawableGeo.h"
#include "cvfHitItemCollection.h"
#include "cvfPart.h"

#include <QMenu>
#include <QMouseEvent>
#include <QStatusBar>

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

    QMenu menu;

    uint faceIndex = cvf::UNDEFINED_UINT;
    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);

    cvf::Part* firstHitPart = NULL;
    cvf::Part* nncFirstHitPart = NULL;

    cvf::HitItemCollection hitItems;

    if (m_viewer->rayPick(winPosX, winPosY, &hitItems))
    {
        extractIntersectionData(hitItems, &localIntersectionPoint, &firstHitPart, &faceIndex, &nncFirstHitPart, NULL);
        updateSelectionFromPickedPart(firstHitPart);
    }

    if (firstHitPart && faceIndex != cvf::UNDEFINED_UINT)
    {
        const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
        const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());

        if (rivSourceInfo || femSourceInfo)
        {
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

            menu.addAction(QIcon(":/CellFilter_Range.png"), QString("I-slice range filter"), this, SLOT(slotRangeFilterI()));
            menu.addAction(QIcon(":/CellFilter_Range.png"), QString("J-slice range filter"), this, SLOT(slotRangeFilterJ()));
            menu.addAction(QIcon(":/CellFilter_Range.png"), QString("K-slice range filter"), this, SLOT(slotRangeFilterK()));

            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
            if (eclipseView)
            {
                RimEclipseCellColors* cellColors = eclipseView->cellResult().p();
                if (cellColors)
                {
                    QAction* propertyAction = new QAction(QIcon(":/CellFilter_Values.png"), QString("Add property filter"), this);
                    connect(propertyAction, SIGNAL(triggered()), SLOT(slotAddEclipsePropertyFilter()));

                    bool isPerCellFaceResult = RimDefines::isPerCellFaceResult(cellColors->resultVariable());
                    if (isPerCellFaceResult)
                    {
                        propertyAction->setEnabled(false);
                    }
                    menu.addAction(propertyAction);
                }

                // Hide faults command
                const RigCaseData* reservoir = eclipseView->eclipseCase()->reservoirData();
                const RigFault* fault = reservoir->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
                if (fault)
                {
                    menu.addSeparator();

                    QString faultName = fault->name();
                    menu.addAction(QString("Hide ") + faultName, this, SLOT(slotHideFault()));
                }
            }

            RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
            if (geoMechView)
            {
                RimGeoMechCellColors* cellColors = geoMechView->cellResult().p();
                if (cellColors)
                {
                    menu.addAction(QIcon(":/CellFilter_Values.png"), QString("Add property filter"), this, SLOT(slotAddGeoMechPropertyFilter()));
                }
            }

        }
    }

    // Well log curve creation commands
    if (firstHitPart && firstHitPart->sourceInfo())
    {
        const RivWellPathSourceInfo* wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(firstHitPart->sourceInfo());
        if (wellPathSourceInfo)
        {
            RimWellPath* wellPath = wellPathSourceInfo->wellPath();
            if (wellPath)
            {
                caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();

                RicNewWellLogFileCurveFeature* newWellLogFileCurveFeature = dynamic_cast<RicNewWellLogFileCurveFeature*>(commandManager->getCommandFeature("RicNewWellLogFileCurveFeature"));
                if (newWellLogFileCurveFeature && newWellLogFileCurveFeature->canFeatureBeExecuted())
                {
                    menu.addAction(newWellLogFileCurveFeature->action());
                }

                RicNewWellLogCurveExtractionFeature* newExtractionCurveFeature = dynamic_cast<RicNewWellLogCurveExtractionFeature*>(commandManager->getCommandFeature("RicNewWellLogCurveExtractionFeature"));
                if (newExtractionCurveFeature && newExtractionCurveFeature->canFeatureBeExecuted())
                {
                    menu.addAction(newExtractionCurveFeature->action());
                }
            }
        }
    }

    // View Link commands
    if (!firstHitPart)
    {
        QStringList commandIds;

        commandIds << "RicLinkViewFeature";
        commandIds << "RicUnLinkViewFeature";
        commandIds << "RicShowLinkOptionsFeature";
        commandIds << "RicSetMasterViewFeature";

        bool firstLinkAction = true;

        caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
        for (int i = 0; i < commandIds.size(); i++)
        {
            caf::CmdFeature* feature = commandManager->getCommandFeature(commandIds[i].toStdString());
            if (feature->canFeatureBeExecuted())
            {
                QAction* act = commandManager->action(commandIds[i]);
                CVF_ASSERT(act);

                if (firstLinkAction)
                {
                    if (menu.actions().size() > 0)
                    {
                        menu.addSeparator();
                    }
                    firstLinkAction = false;
                }

                menu.addAction(act);
            }
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

    RimCellRangeFilterCollection* rangeFilterCollection = eclipseView->rangeFilterCollection();

    RicRangeFilterNewExec* filterExec = new RicRangeFilterNewExec(rangeFilterCollection);

    if (ijOrk == 0){
        filterExec->m_iSlice = true;
        filterExec->m_iSliceStart = CVF_MAX(static_cast<int>(i + 1), 1);
    }
    else if (ijOrk == 1){
        filterExec->m_jSlice = true;
        filterExec->m_jSliceStart = CVF_MAX(static_cast<int>(j + 1), 1);

    }
    else if (ijOrk == 2){
        filterExec->m_kSlice = true;
        filterExec->m_kSliceStart = CVF_MAX(static_cast<int>(k + 1), 1);
    }

    caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);

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
			caf::PdmUiFieldHandle* uiFieldHandle = rimFault->showFault.uiCapability();
            if (uiFieldHandle)
            {
                uiFieldHandle->setValueFromUi(!rimFault->showFault);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotAddEclipsePropertyFilter()
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    if (eclipseView)
    {
        RimEclipsePropertyFilterCollection* filterCollection = eclipseView->propertyFilterCollection();
        RicEclipsePropertyFilterNewExec* propCmdExe = new RicEclipsePropertyFilterNewExec(filterCollection);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(propCmdExe);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotAddGeoMechPropertyFilter()
{
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
    if (geoMechView)
    {
        RimGeoMechPropertyFilterCollection* filterCollection = geoMechView->propertyFilterCollection();
        RicGeoMechPropertyFilterNewExec* propCmdExe = new RicGeoMechPropertyFilterNewExec(filterCollection);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(propCmdExe);
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
    RimWellPath* wellPath = NULL;
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
            updateSelectionFromPickedPart(firstHitPart);
        }

        if (firstHitPart && firstHitPart->sourceInfo())
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
            const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());
            const RivWellPathSourceInfo* wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(firstHitPart->sourceInfo());

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
            else if (wellPathSourceInfo)
            {
                wellPath = wellPathSourceInfo->wellPath();
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

    if (cellIndex != cvf::UNDEFINED_SIZE_T || nncIndex != cvf::UNDEFINED_SIZE_T)
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

    if (wellPath)
    {
        pickInfo = QString("Well path hit: %1").arg(wellPath->name());
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

    if (firstNonNncHitItem)
    {
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
    else
    {
        if (localIntersectionPoint && nncPart && *nncPart)
        {
            cvf::Vec3d globalPickedPoint = firstItemIntersectionPoint;

            const cvf::Transform* xf = (*nncPart)->transform();
            if (xf)
            {
                *localIntersectionPoint = globalPickedPoint.getTransformedPoint(xf->worldTransform().getInverted());
            }
            else
            {
                *localIntersectionPoint = globalPickedPoint;
            }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::updateSelectionFromPickedPart(cvf::Part* part)
{
    if (part && part->sourceInfo())
    {
        const RivWellPathSourceInfo* wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(part->sourceInfo());
        if (wellPathSourceInfo)
        {
            RimWellPath* wellPath = wellPathSourceInfo->wellPath();
            if (wellPath)
            {
                RiuMainWindow::instance()->projectTreeView()->selectAsCurrentItem(wellPath);
            }
        }
    }
}
