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

#include "RiaApplication.h"
#include "RiaColorTables.h"

#include "RicViewerEventInterface.h"
#include "RicEclipsePropertyFilterNewExec.h"
#include "RicGeoMechPropertyFilterNewExec.h"
#include "RicRangeFilterNewExec.h"
#include "WellPathCommands/RicWellPathViewerEventHandler.h"

#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"

#include "RimCellEdgeColors.h"
#include "RimContextCommandBuilder.h"
#include "RimIntersection.h"
#include "RiaDefines.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimFault.h"
#include "RimFaultCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimIntersectionBox.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivIntersectionBoxSourceInfo.h"
#include "RivIntersectionSourceInfo.h"
#include "RivObjectSourceInfo.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivTernarySaturationOverlayItem.h"
#include "RivWellPathSourceInfo.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include "cvfDrawableGeo.h"
#include "cvfHitItemCollection.h"
#include "cvfOverlayAxisCross.h"
#include "cvfOverlayScalarMapperLegend.h"
#include "cvfPart.h"
#include "cvfTransform.h"

#include <QMenu>
#include <QMouseEvent>
#include <QStatusBar>
#include <array>



//==================================================================================================
//
// RiaViewerCommands
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewerCommands::RiuViewerCommands(RiuViewer* ownerViewer) 
    : QObject(ownerViewer), 
      m_viewer(ownerViewer), 
      m_currentGridIdx(-1),
      m_currentCellIndex(-1),
      m_currentPickPositionInDomainCoords(cvf::Vec3d::UNDEFINED)
{
    {
        caf::CmdFeature* cmdFeature = caf::CmdFeatureManager::instance()->getCommandFeature("RicNewPolylineIntersectionFeature");
        CVF_ASSERT(cmdFeature);

        m_viewerEventHandlers.push_back(dynamic_cast<RicViewerEventInterface*>(cmdFeature));
    }

    {
        m_viewerEventHandlers.push_back(dynamic_cast<RicViewerEventInterface*>(RicWellPathViewerEventHandler::instance()));
    }
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
    m_currentPickedObject = nullptr;

    int winPosX = event->x();
    int winPosY = event->y();

    QMenu menu;

    uint firstPartTriangleIndex = cvf::UNDEFINED_UINT;
    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);
    cvf::Vec3d globalIntersectionPoint(cvf::Vec3d::ZERO);

    cvf::Part* firstHitPart = NULL;
    cvf::Part* nncFirstHitPart = NULL;

    m_currentPickPositionInDomainCoords = cvf::Vec3d::UNDEFINED;

    cvf::HitItemCollection hitItems;
    if (m_viewer->rayPick(winPosX, winPosY, &hitItems))
    {
        extractIntersectionData(hitItems, &localIntersectionPoint, &globalIntersectionPoint, &firstHitPart, &firstPartTriangleIndex, &nncFirstHitPart, NULL);

        cvf::Vec3d displayModelOffset = cvf::Vec3d::ZERO;

        RimView* activeView = RiaApplication::instance()->activeReservoirView();
        CVF_ASSERT(activeView);

        cvf::ref<caf::DisplayCoordTransform> transForm = activeView->displayCoordTransform();
        m_currentPickPositionInDomainCoords = transForm->transformToDomainCoord(globalIntersectionPoint);
    }

    if (firstHitPart && firstPartTriangleIndex != cvf::UNDEFINED_UINT)
    {
        const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
        const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());
        const RivIntersectionSourceInfo* crossSectionSourceInfo = dynamic_cast<const RivIntersectionSourceInfo*>(firstHitPart->sourceInfo());
        const RivIntersectionBoxSourceInfo* intersectionBoxSourceInfo = dynamic_cast<const RivIntersectionBoxSourceInfo*>(firstHitPart->sourceInfo());

        if (rivSourceInfo || femSourceInfo || crossSectionSourceInfo || intersectionBoxSourceInfo)
        {
            if (rivSourceInfo)
            {
                if (!rivSourceInfo->hasCellFaceMapping()) return;

                // Set the data regarding what was hit

                m_currentGridIdx = rivSourceInfo->gridIndex();
                m_currentCellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex(firstPartTriangleIndex);
                m_currentFaceIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace(firstPartTriangleIndex);
            }
            else if (femSourceInfo)
            {
                m_currentGridIdx = femSourceInfo->femPartIndex();
                m_currentCellIndex = femSourceInfo->triangleToElmMapper()->elementIndex(firstPartTriangleIndex);
            }
            else if (crossSectionSourceInfo)
            {
                findCellAndGridIndex(crossSectionSourceInfo, firstPartTriangleIndex, &m_currentCellIndex, &m_currentGridIdx);
                m_currentFaceIndex = cvf::StructGridInterface::NO_FACE;
                m_currentPickedObject = const_cast<RimIntersection*>(crossSectionSourceInfo->crossSection());

                menu.addAction(QString("Hide intersection"), this, SLOT(slotHideIntersection()));
            }
            else if (intersectionBoxSourceInfo)
            {
                findCellAndGridIndex(intersectionBoxSourceInfo, firstPartTriangleIndex, &m_currentCellIndex, &m_currentGridIdx);
                m_currentFaceIndex = cvf::StructGridInterface::NO_FACE;

                m_currentPickedObject = const_cast<RimIntersectionBox*>(intersectionBoxSourceInfo->intersectionBox());
            }

            // IJK -slice commands

            RimViewController* viewController = NULL;
            if (m_reservoirView) viewController = m_reservoirView->viewController();    

            if (!viewController || !viewController->isRangeFiltersControlled())
            {
                menu.addAction(QIcon(":/CellFilter_Range.png"), QString("I-slice range filter"), this, SLOT(slotRangeFilterI()));
                menu.addAction(QIcon(":/CellFilter_Range.png"), QString("J-slice range filter"), this, SLOT(slotRangeFilterJ()));
                menu.addAction(QIcon(":/CellFilter_Range.png"), QString("K-slice range filter"), this, SLOT(slotRangeFilterK()));

            }

            if (menu.actions().size() > 0) menu.addSeparator();
            menu.addAction(caf::CmdFeatureManager::instance()->action("RicNewPolylineIntersectionFeature"));
            menu.addAction(caf::CmdFeatureManager::instance()->action("RicIntersectionBoxAtPosFeature"));
            menu.addAction(caf::CmdFeatureManager::instance()->action("RicIntersectionBoxXSliceFeature"));
            menu.addAction(caf::CmdFeatureManager::instance()->action("RicIntersectionBoxYSliceFeature"));
            menu.addAction(caf::CmdFeatureManager::instance()->action("RicIntersectionBoxZSliceFeature"));
            menu.addSeparator();



            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
            if (eclipseView)
            {
                RimEclipseCellColors* cellColors = eclipseView->cellResult().p();
                if (cellColors)
                {
                    QAction* propertyAction = new QAction(QIcon(":/CellFilter_Values.png"), QString("Add property filter"), this);
                    connect(propertyAction, SIGNAL(triggered()), SLOT(slotAddEclipsePropertyFilter()));

                    bool isPerCellFaceResult = RiaDefines::isPerCellFaceResult(cellColors->resultVariable());
                    if (isPerCellFaceResult)
                    {
                        propertyAction->setEnabled(false);
                    }

                    if (!viewController || !viewController->isPropertyFilterOveridden())
                    {
                        menu.addAction(propertyAction);
                    }
                }

                // Hide faults command
                const RigFault* fault = eclipseView->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
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
                     if (!viewController || !viewController->isPropertyFilterOveridden())
                    {
                        menu.addAction(QIcon(":/CellFilter_Values.png"), QString("Add property filter"), this, SLOT(slotAddGeoMechPropertyFilter()));
                    }
                }
            }
        }
    }

    QStringList commandIds;

    // Well log curve creation commands
    if (firstHitPart && firstHitPart->sourceInfo())
    {
        const RivWellPathSourceInfo* wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(firstHitPart->sourceInfo());
        if (wellPathSourceInfo)
        {
            RimWellPath* wellPath = wellPathSourceInfo->wellPath();
            if (wellPath)
            {

                double measuredDepth = wellPathSourceInfo->measuredDepth(firstPartTriangleIndex, m_currentPickPositionInDomainCoords);
                cvf::Vec3d trueVerticalDepth = wellPathSourceInfo->trueVerticalDepth(firstPartTriangleIndex, globalIntersectionPoint);
                RiuSelectionItem* selItem = new RiuWellPathSelectionItem(wellPathSourceInfo, trueVerticalDepth, measuredDepth);
                RiuSelectionManager::instance()->setSelectedItem(selItem, RiuSelectionManager::RUI_TEMPORARY);
                 
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
                commandIds << "RicNewWellPathFractureAtPosFeature";
#endif // USE_PROTOTYPE_FEATURE_FRACTURES


                //TODO: Update so these also use RiuWellPathSelectionItem 
                caf::SelectionManager::instance()->setSelectedItem(wellPath);

                commandIds << "RicNewWellLogCurveExtractionFeature";
                commandIds << "RicNewWellLogFileCurveFeature";
                commandIds << "Separator";
                commandIds << "RicNewWellPathIntersectionFeature";
                commandIds << "RicNewFishbonesSubsAtMeasuredDepthFeature";
                commandIds << "RicNewPerforationIntervalAtMeasuredDepthFeature";
            }
        }

        const RivSimWellPipeSourceInfo* eclipseWellSourceInfo = dynamic_cast<const RivSimWellPipeSourceInfo*>(firstHitPart->sourceInfo());
        if (eclipseWellSourceInfo)
        {
            RimEclipseWell* well = eclipseWellSourceInfo->well();
            if (well)
            {
                caf::SelectionManager::instance()->setSelectedItem(well);

                RiuSelectionItem* selItem = new RiuSimWellSelectionItem(eclipseWellSourceInfo->well(), m_currentPickPositionInDomainCoords, eclipseWellSourceInfo->branchIndex());
                RiuSelectionManager::instance()->setSelectedItem(selItem, RiuSelectionManager::RUI_TEMPORARY);

                commandIds << "RicNewWellLogCurveExtractionFeature";
                commandIds << "RicNewRftPlotFeature";
                commandIds << "RicShowWellAllocationPlotFeature";
                commandIds << "RicPlotProductionRateFeature";
                commandIds << "Separator";
                commandIds << "RicShowContributingWellsFeature";
                commandIds << "Separator";
                commandIds << "RicNewSimWellIntersectionFeature";
                commandIds << "RicPlotProductionRateFeature";
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
                commandIds << "RicNewSimWellFractureAtPosFeature";
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
            }
        }

    }

    // View Link commands
    if (!firstHitPart)
    {
        commandIds << "RicLinkViewFeature";
        commandIds << "RicShowLinkOptionsFeature";
        commandIds << "RicSetMasterViewFeature";
        commandIds << "RicUnLinkViewFeature";
    }

    commandIds << "Separator";
    commandIds << "RicNewGridTimeHistoryCurveFeature";
    commandIds << "RicShowFlowCharacteristicsPlotFeature";
    commandIds << "RicSaveEclipseInputVisibleCellsFeature";


    RimContextCommandBuilder::appendCommandsToMenu(commandIds, &menu);

    if (!menu.isEmpty())
    {
        menu.exec(event->globalPos());
    }

    // Delete items in temporary selection
    RiuSelectionManager::instance()->deleteAllItems(RiuSelectionManager::RUI_TEMPORARY);
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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

    const RigFault* fault = eclipseView->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
    if (fault)
    {
        QString faultName = fault->name();

        RimFault* rimFault = eclipseView->faultCollection()->findFaultByName(faultName);
        if (rimFault)
        {
            rimFault->showFault.setValueWithFieldChanged(!rimFault->showFault);
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
        RimEclipsePropertyFilterCollection* filterCollection = eclipseView->eclipsePropertyFilterCollection();
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
        RimGeoMechPropertyFilterCollection* filterCollection = geoMechView->geoMechPropertyFilterCollection();
        RicGeoMechPropertyFilterNewExec* propCmdExe = new RicGeoMechPropertyFilterNewExec(filterCollection);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(propCmdExe);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotHideIntersection()
{
    RimIntersection* rimIntersection = dynamic_cast<RimIntersection*>(currentPickedObject());
    if (rimIntersection)
    {
        rimIntersection->isActive = false;
        rimIntersection->updateConnectedEditors();

        if (m_reservoirView)
        {
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::handlePickAction(int winPosX, int winPosY, Qt::KeyboardModifiers keyboardModifiers)
{
    if (handleOverlayItemPicking(winPosX, winPosY))
    {
        return;
    }

    size_t gridIndex = cvf::UNDEFINED_SIZE_T;
    size_t cellIndex = cvf::UNDEFINED_SIZE_T;
    size_t nncIndex = cvf::UNDEFINED_SIZE_T;
    cvf::StructGridInterface::FaceType face = cvf::StructGridInterface::NO_FACE;
    int gmFace = -1;
    bool intersectionHit = false;
    std::array<cvf::Vec3f, 3> intersectionTriangleHit;

    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);
    cvf::Vec3d globalIntersectionPoint(cvf::Vec3d::ZERO);

    // Extract all the above information from the pick
    {
        cvf::Part* firstHitPart = NULL;
        uint firstPartTriangleIndex = cvf::UNDEFINED_UINT;

        cvf::Part* firstNncHitPart = NULL;
        uint nncPartTriangleIndex = cvf::UNDEFINED_UINT;

        cvf::HitItemCollection hitItems;
        if (m_viewer->rayPick(winPosX, winPosY, &hitItems))
        {
            extractIntersectionData(hitItems, &localIntersectionPoint, &globalIntersectionPoint, &firstHitPart, &firstPartTriangleIndex, &firstNncHitPart, &nncPartTriangleIndex);

            cvf::ref<RicViewerEventObject> eventObj = new RicViewerEventObject(globalIntersectionPoint, firstHitPart, firstPartTriangleIndex);
            for (size_t i = 0; i < m_viewerEventHandlers.size(); i++)
            {
                if (m_viewerEventHandlers[i]->handleEvent(eventObj.p()))
                {
                    return;
                }
            }
        }

        if (firstHitPart && firstHitPart->sourceInfo())
        {
            const RivObjectSourceInfo* rivObjectSourceInfo = dynamic_cast<const RivObjectSourceInfo*>(firstHitPart->sourceInfo());
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
            const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());
            const RivIntersectionSourceInfo* crossSectionSourceInfo = dynamic_cast<const RivIntersectionSourceInfo*>(firstHitPart->sourceInfo());
            const RivIntersectionBoxSourceInfo* intersectionBoxSourceInfo = dynamic_cast<const RivIntersectionBoxSourceInfo*>(firstHitPart->sourceInfo());
            const RivSimWellPipeSourceInfo* eclipseWellSourceInfo = dynamic_cast<const RivSimWellPipeSourceInfo*>(firstHitPart->sourceInfo());

            if (rivObjectSourceInfo)
            {
                RiuMainWindow::instance()->selectAsCurrentItem(rivObjectSourceInfo->object());
            }
            
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
                gmFace = femSourceInfo->triangleToElmMapper()->elementFace(firstPartTriangleIndex);

            }
            else if (crossSectionSourceInfo)
            {
                findCellAndGridIndex(crossSectionSourceInfo, firstPartTriangleIndex, &cellIndex, &gridIndex);
                intersectionHit = true;
                intersectionTriangleHit = crossSectionSourceInfo->triangle(firstPartTriangleIndex);

                RiuMainWindow::instance()->selectAsCurrentItem(const_cast<RimIntersection*>(crossSectionSourceInfo->crossSection()));

            }
            else if (intersectionBoxSourceInfo)
            {
                findCellAndGridIndex(intersectionBoxSourceInfo, firstPartTriangleIndex, &cellIndex, &gridIndex);
                intersectionHit = true;
                intersectionTriangleHit = intersectionBoxSourceInfo->triangle(firstPartTriangleIndex);

                RiuMainWindow::instance()->selectAsCurrentItem(const_cast<RimIntersectionBox*>(intersectionBoxSourceInfo->intersectionBox()));

            }
            else if (eclipseWellSourceInfo)
            {
                RiuMainWindow::instance()->selectAsCurrentItem(eclipseWellSourceInfo->well());
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

    if (cellIndex == cvf::UNDEFINED_SIZE_T)
    {
        RiuSelectionManager::instance()->deleteAllItems();
    }
    else 
    {
        bool appendToSelection = false;
        if (keyboardModifiers & Qt::ControlModifier)
        {
            appendToSelection = true;
        }

        std::vector<RiuSelectionItem*> items;
        RiuSelectionManager::instance()->selectedItems(items);

        const caf::ColorTable& colorTable = RiaColorTables::selectionPaletteColors();

        cvf::Color3f curveColor = colorTable.cycledColor3f(items.size());

        if (!appendToSelection)
        {
            curveColor = colorTable.cycledColor3f(0);
        }

        RiuSelectionItem* selItem = NULL;
        {
            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
            if (eclipseView)
            {
                selItem = new RiuEclipseSelectionItem(eclipseView, gridIndex, cellIndex, nncIndex, curveColor, face, localIntersectionPoint);
            }

            RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
            if (geomView )
            {
                if(intersectionHit)   selItem = new RiuGeoMechSelectionItem(geomView, gridIndex, cellIndex, curveColor, gmFace, localIntersectionPoint, intersectionTriangleHit);
                else                  selItem = new RiuGeoMechSelectionItem(geomView, gridIndex, cellIndex, curveColor, gmFace, localIntersectionPoint);
            }


        }

        if (appendToSelection)
        {
            RiuSelectionManager::instance()->appendItemToSelection(selItem);
        }
        else
        {
            RiuSelectionManager::instance()->setSelectedItem(selItem);
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RiuViewerCommands::lastPickPositionInDomainCoords() const
{
    return m_currentPickPositionInDomainCoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiuViewerCommands::currentPickedObject() const
{
    return m_currentPickedObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::findCellAndGridIndex(const RivIntersectionSourceInfo* crossSectionSourceInfo, cvf::uint firstPartTriangleIndex, size_t* cellIndex, size_t* gridIndex)
{
    CVF_ASSERT(cellIndex && gridIndex);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
    if (eclipseView)
    {
        size_t globalCellIndex = crossSectionSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];

        const RigCell& cell = eclipseView->mainGrid()->globalCellArray()[globalCellIndex];
        *cellIndex = cell.gridLocalCellIndex();
        *gridIndex = cell.hostGrid()->gridIndex();
    }
    else if (geomView)
    {
        *cellIndex = crossSectionSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];
        *gridIndex = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::findCellAndGridIndex(const RivIntersectionBoxSourceInfo* intersectionBoxSourceInfo, cvf::uint firstPartTriangleIndex, size_t* cellIndex, size_t* gridIndex)
{
    CVF_ASSERT(cellIndex && gridIndex);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
    if (eclipseView)
    {
        size_t globalCellIndex = intersectionBoxSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];

        const RigCell& cell = eclipseView->mainGrid()->globalCellArray()[globalCellIndex];
        *cellIndex = cell.gridLocalCellIndex();
        *gridIndex = cell.hostGrid()->gridIndex();
    }
    else if (geomView)
    {
        *cellIndex = intersectionBoxSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];
        *gridIndex = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// Perform picking and return the index of the face that was hit, if a drawable geo was hit
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::extractIntersectionData(const cvf::HitItemCollection& hitItems, 
                                        cvf::Vec3d* localIntersectionPoint,
                                        cvf::Vec3d* globalIntersectionPoint,
                                        cvf::Part** firstPart, uint* firstPartFaceHit, 
                                        cvf::Part** nncPart, uint* nncPartFaceHit)
{
    CVF_ASSERT(hitItems.count() > 0);

    double pickDepthThresholdSquared = 0.05 *0.05;
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());

        if (eclipseView && eclipseView->mainGrid())
        {
            double characteristicCellSize = eclipseView->mainGrid()->characteristicIJCellSize();
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

        if (globalIntersectionPoint)
        {
            *globalIntersectionPoint = globalPickedPoint;
        }

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

            if (globalIntersectionPoint)
            {
                *globalIntersectionPoint = globalPickedPoint;
            }

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
        eclipseView->eclipseCase()->eclipseCaseData()->grid(gridIdx)->ijkFromCellIndex(cellIndex, i, j, k);
    }
    
    if (geomView && geomView->geoMechCase())
    {
        geomView->geoMechCase()->geoMechData()->femParts()->part(gridIdx)->structGrid()->ijkFromCellIndex(cellIndex, i, j, k);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuViewerCommands::handleOverlayItemPicking(int winPosX, int winPosY)
{
    cvf::OverlayItem* pickedOverlayItem = m_viewer->overlayItem(winPosX, winPosY);
    if (!pickedOverlayItem)
    {
        pickedOverlayItem = m_viewer->pickFixedPositionedLegend(winPosX, winPosY);
    }

    if (pickedOverlayItem)
    {
        caf::PdmObject* objToSelect = NULL;

        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
        if (eclipseView)
        {
            if (eclipseView->cellResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->cellResult()->legendConfig();
            }
            else if (eclipseView->cellResult()->ternaryLegendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->cellResult()->ternaryLegendConfig();
            }
            else if (eclipseView->faultResultSettings()->customFaultResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->faultResultSettings()->customFaultResult()->legendConfig();
            }
            else if (eclipseView->faultResultSettings()->customFaultResult()->ternaryLegendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->faultResultSettings()->customFaultResult()->ternaryLegendConfig();
            }
            else if (eclipseView->cellEdgeResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->cellEdgeResult()->legendConfig();
            }
        }

        RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
        if (geomView)
        {
            if (geomView->cellResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = geomView->cellResult()->legendConfig();
            }
        }

        if (objToSelect)
        {
            RiuMainWindow::instance()->selectAsCurrentItem(objToSelect);

            return true;
        }
    }

    return false;
}
