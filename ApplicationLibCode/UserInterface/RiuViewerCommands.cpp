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

#include "RiaDefines.h"
#include "RiaOptionItemFactory.h"

#include "GeoMechCommands/RicGeoMechPropertyFilterNewExec.h"
#include "MeasurementCommands/RicMeasurementPickEventHandler.h"
#include "RicContourMapPickEventHandler.h"
#include "RicEclipsePropertyFilterNewExec.h"
#include "RicPickEventHandler.h"
#include "RiuCellAndNncPickEventHandler.h"
#include "WellLogCommands/Ric3dWellLogCurvePickEventHandler.h"
#include "WellPathCommands/RicIntersectionPickEventHandler.h"
#include "WellPathCommands/RicWellPathPickEventHandler.h"

#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"
#include "RigVirtualPerforationTransmissibilities.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim2dIntersectionView.h"
#include "RimBoxIntersection.h"
#include "RimCellEdgeColors.h"
#include "RimContextCommandBuilder.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimFracture.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimLegendConfig.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimSeismicDataInterface.h"
#include "RimSeismicSection.h"
#include "RimSeismicView.h"
#include "RimSimWellInView.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimSurfaceInView.h"
#include "RimTextAnnotation.h"
#include "RimViewController.h"
#include "RimWellIASettingsCollection.h"
#include "RimWellPath.h"

#include "Riu3dSelectionManager.h"
#include "RiuMainWindow.h"
#include "RiuPickItemInfo.h"
#include "RiuPlotMainWindow.h"
#include "RiuResultTextBuilder.h"
#include "RiuViewer.h"

#include "RivBoxIntersectionSourceInfo.h"
#include "RivExtrudedCurveIntersectionSourceInfo.h"
#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivObjectSourceInfo.h"
#include "RivPartPriority.h"
#include "RivReservoirSurfaceIntersectionSourceInfo.h"
#include "RivSeismicSectionSourceInfo.h"
#include "RivSimWellConnectionSourceInfo.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivSurfacePartMgr.h"
#include "RivTernarySaturationOverlayItem.h"
#include "RivWellConnectionSourceInfo.h"
#include "RivWellFracturePartMgr.h"
#include "RivWellPathSourceInfo.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafDisplayCoordTransform.h"
#include "cafOverlayScalarMapperLegend.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfHitItemCollection.h"
#include "cvfOverlayAxisCross.h"
#include "cvfPart.h"
#include "cvfRay.h"
#include "cvfScene.h"

#include <QMenu>
#include <QMouseEvent>
#include <QStatusBar>

#include <array>

//==================================================================================================
//
// RiaViewerCommands
//
//==================================================================================================
Ric3dViewPickEventHandler* RiuViewerCommands::sm_overridingPickHandler = nullptr;

std::vector<RicDefaultPickEventHandler*> RiuViewerCommands::sm_defaultPickEventHandlers;
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuViewerCommands::RiuViewerCommands( RiuViewer* ownerViewer )
    : QObject( ownerViewer )
    , m_currentGridIdx( -1 )
    , m_currentCellIndex( -1 )
    , m_currentFaceIndex( cvf::StructGridInterface::NO_FACE )
    , m_currentPickPositionInDomainCoords( cvf::Vec3d::UNDEFINED )
    , m_isCurrentPickInComparisonView( false )
    , m_viewer( ownerViewer )
{
    if ( sm_defaultPickEventHandlers.empty() )
    {
        addDefaultPickEventHandler( RicIntersectionPickEventHandler::instance() );
        addDefaultPickEventHandler( Ric3dWellLogCurvePickEventHandler::instance() );
        addDefaultPickEventHandler( RicWellPathPickEventHandler::instance() );
        addDefaultPickEventHandler( RicContourMapPickEventHandler::instance() );
        addDefaultPickEventHandler( RiuCellAndNncPickEventHandler::instance() );
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
void RiuViewerCommands::setOwnerView( Rim3dView* owner )
{
    m_reservoirView = owner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::addCompareToViewMenu( caf::CmdFeatureMenuBuilder* menuBuilder )
{
    auto* mainGridView = m_reservoirView.p();
    if ( mainGridView && !mainGridView->activeComparisonView() )
    {
        std::vector<Rim3dView*> validComparisonViews = mainGridView->validComparisonViews();
        if ( !validComparisonViews.empty() )
        {
            menuBuilder->subMenuStart( "Compare To ...", QIcon( ":/ComparisonView16x16.png" ) );
            for ( auto view : validComparisonViews )
            {
                menuBuilder->addCmdFeatureWithUserData( "RicCompareTo3dViewFeature",
                                                        view->autoName(),
                                                        QVariant::fromValue( static_cast<void*>( view ) ) );
            }
            menuBuilder->subMenuEnd();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::displayContextMenu( QMouseEvent* event )
{
    // Do the ray pick, and extract the infos

    std::vector<RiuPickItemInfo> pickItemInfos;
    {
        cvf::HitItemCollection hitItems;
        cvf::Vec3d             globalRayOrigin;
        if ( m_viewer->rayPick( event->x(), event->y(), &hitItems, &globalRayOrigin ) )
        {
            pickItemInfos = RiuPickItemInfo::convertToPickItemInfos( hitItems, globalRayOrigin );
        }
    }

    m_isCurrentPickInComparisonView = m_viewer->isMousePosWithinComparisonView( event->x(), event->y() );
    Rim3dView* mainOrComparisonView = m_isCurrentPickInComparisonView ? m_reservoirView->activeComparisonView() : m_reservoirView.p();

    // Find the following data

    const cvf::Part* firstHitPart           = nullptr;
    const cvf::Part* additionalHitPart      = nullptr;
    uint             firstPartTriangleIndex = cvf::UNDEFINED_UINT;
    m_currentPickPositionInDomainCoords     = cvf::Vec3d::UNDEFINED;

    if ( !pickItemInfos.empty() )
    {
        cvf::Vec3d globalIntersectionPoint = pickItemInfos[0].globalPickedPoint();

        for ( const auto& pickItem : pickItemInfos )
        {
            const RivObjectSourceInfo* objectSourceInfo = dynamic_cast<const RivObjectSourceInfo*>( pickItem.sourceInfo() );
            if ( objectSourceInfo && dynamic_cast<RimWellPathComponentInterface*>( objectSourceInfo->object() ) )
            {
                // Store any component hit, but keep going to find main well path
                additionalHitPart = pickItem.pickedPart();
                continue;
            }

            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>( pickItem.sourceInfo() );
            if ( rivSourceInfo && rivSourceInfo->hasNNCIndices() )
            {
                // Skip picking on nnc-s
                continue;
            }

            firstHitPart            = pickItem.pickedPart();
            firstPartTriangleIndex  = pickItem.faceIdx();
            globalIntersectionPoint = pickItem.globalPickedPoint();
            break;
        }

        if ( mainOrComparisonView )
        {
            cvf::ref<caf::DisplayCoordTransform> transForm = mainOrComparisonView->displayCoordTransform();
            m_currentPickPositionInDomainCoords            = transForm->transformToDomainCoord( globalIntersectionPoint );
        }
    }

    // Build menus

    caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget( m_viewer );

    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;
    m_currentGridIdx   = cvf::UNDEFINED_SIZE_T;
    m_currentCellIndex = cvf::UNDEFINED_SIZE_T;

    // Check type of view

    RimSeismicView* seisView = dynamic_cast<RimSeismicView*>( mainOrComparisonView );
    if ( seisView )
    {
        // no context menu support in seismic views, yet
        return;
    }

    RimGridView*           gridView  = dynamic_cast<RimGridView*>( mainOrComparisonView );
    Rim2dIntersectionView* int2dView = dynamic_cast<Rim2dIntersectionView*>( mainOrComparisonView );

    if ( firstHitPart && firstPartTriangleIndex != cvf::UNDEFINED_UINT )
    {
        const auto* rivSourceInfo           = dynamic_cast<const RivSourceInfo*>( firstHitPart->sourceInfo() );
        const auto* femSourceInfo           = dynamic_cast<const RivFemPickSourceInfo*>( firstHitPart->sourceInfo() );
        const auto* surfIntersectSourceInfo = dynamic_cast<const RivReservoirSurfaceIntersectionSourceInfo*>( firstHitPart->sourceInfo() );
        const auto* crossSectionSourceInfo  = dynamic_cast<const RivExtrudedCurveIntersectionSourceInfo*>( firstHitPart->sourceInfo() );
        const auto* intersectionBoxSourceInfo = dynamic_cast<const RivBoxIntersectionSourceInfo*>( firstHitPart->sourceInfo() );

        if ( rivSourceInfo || femSourceInfo || crossSectionSourceInfo || intersectionBoxSourceInfo || surfIntersectSourceInfo )
        {
            if ( rivSourceInfo )
            {
                if ( !rivSourceInfo->hasCellFaceMapping() ) return;

                // Set the data regarding what was hit

                m_currentGridIdx   = rivSourceInfo->gridIndex();
                m_currentCellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex( firstPartTriangleIndex );
                m_currentFaceIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace( firstPartTriangleIndex );
            }
            else if ( femSourceInfo )
            {
                m_currentGridIdx   = femSourceInfo->femPartIndex();
                m_currentCellIndex = femSourceInfo->triangleToElmMapper()->elementIndex( firstPartTriangleIndex );
            }
            else if ( surfIntersectSourceInfo )
            {
                findCellAndGridIndex( mainOrComparisonView,
                                      surfIntersectSourceInfo->intersection()->activeSeparateResultDefinition(),
                                      surfIntersectSourceInfo->triangleToCellIndex()[firstPartTriangleIndex],
                                      &m_currentCellIndex,
                                      &m_currentGridIdx );

                m_currentFaceIndex = cvf::StructGridInterface::NO_FACE;

                RiuSelectionItem* selItem = new RiuGeneralSelectionItem( surfIntersectSourceInfo->intersection() );
                Riu3dSelectionManager::instance()->setSelectedItem( selItem, Riu3dSelectionManager::RUI_TEMPORARY );
            }
            else if ( crossSectionSourceInfo )
            {
                findCellAndGridIndex( mainOrComparisonView,
                                      crossSectionSourceInfo->intersection()->activeSeparateResultDefinition(),
                                      crossSectionSourceInfo->triangleToCellIndex()[firstPartTriangleIndex],
                                      &m_currentCellIndex,
                                      &m_currentGridIdx );

                m_currentFaceIndex = cvf::StructGridInterface::NO_FACE;

                RiuSelectionItem* selItem = new RiuGeneralSelectionItem( crossSectionSourceInfo->intersection() );
                Riu3dSelectionManager::instance()->setSelectedItem( selItem, Riu3dSelectionManager::RUI_TEMPORARY );

                if ( gridView )
                {
                    menuBuilder << "RicHideIntersectionFeature";
                    menuBuilder.addSeparator();
                    menuBuilder << "RicNewIntersectionViewFeature";
                    menuBuilder.addSeparator();
                }
                else if ( int2dView )
                {
                    menuBuilder << "RicSelectColorResult";
                }
            }
            else if ( intersectionBoxSourceInfo )
            {
                findCellAndGridIndex( mainOrComparisonView,
                                      intersectionBoxSourceInfo->intersectionBox()->activeSeparateResultDefinition(),
                                      intersectionBoxSourceInfo->triangleToCellIndex()[firstPartTriangleIndex],
                                      &m_currentCellIndex,
                                      &m_currentGridIdx );

                m_currentFaceIndex = cvf::StructGridInterface::NO_FACE;

                RiuSelectionItem* selItem = new RiuGeneralSelectionItem( intersectionBoxSourceInfo->intersectionBox() );
                Riu3dSelectionManager::instance()->setSelectedItem( selItem, Riu3dSelectionManager::RUI_TEMPORARY );

                menuBuilder << "RicHideIntersectionBoxFeature";
                menuBuilder.addSeparator();
            }

            if ( gridView )
            {
                // IJK -slice commands
                RimViewController* viewController = nullptr;
                if ( mainOrComparisonView ) viewController = mainOrComparisonView->viewController();

                if ( !viewController || !viewController->isCellFiltersControlled() )
                {
                    size_t i, j, k;
                    ijkFromCellIndex( mainOrComparisonView, m_currentGridIdx, m_currentCellIndex, &i, &j, &k );

                    QVariantList iSliceList;
                    iSliceList.push_back( 0 );
                    iSliceList.push_back( std::max( static_cast<int>( i + 1 ), 1 ) );
                    iSliceList.push_back( static_cast<int>( m_currentGridIdx ) );

                    QVariantList jSliceList;
                    jSliceList.push_back( 1 );
                    jSliceList.push_back( std::max( static_cast<int>( j + 1 ), 1 ) );
                    jSliceList.push_back( static_cast<int>( m_currentGridIdx ) );

                    QVariantList kSliceList;
                    kSliceList.push_back( 2 );
                    kSliceList.push_back( std::max( static_cast<int>( k + 1 ), 1 ) );
                    kSliceList.push_back( static_cast<int>( m_currentGridIdx ) );

                    menuBuilder.subMenuStart( "Range Filter", QIcon( ":/CellFilter_Range.png" ) );

                    menuBuilder.addCmdFeatureWithUserData( "RicNewRangeFilterSlice3dviewFeature", "I Slice", iSliceList );
                    menuBuilder.addCmdFeatureWithUserData( "RicNewRangeFilterSlice3dviewFeature", "J Slice", jSliceList );
                    menuBuilder.addCmdFeatureWithUserData( "RicNewRangeFilterSlice3dviewFeature", "K Slice", kSliceList );
                    menuBuilder.addCmdFeature( "RicNewCellRangeFilterFeature", "IJK Filter" );

                    menuBuilder.subMenuEnd();
                }

                // Property filter commands
                menuBuilder << "RicEclipsePropertyFilterNewInViewFeature";
                menuBuilder << "RicGeoMechPropertyFilterNewInViewFeature";

                // Polygon commands
                menuBuilder << "RicNewPolygonFilter3dviewFeature";

                menuBuilder.addSeparator();

                menuBuilder.subMenuStart( "Intersections", QIcon( ":/IntersectionXPlane16x16.png" ) );

                menuBuilder << "RicNewPolylineIntersectionFeature";
                menuBuilder << "RicNewAzimuthDipIntersectionFeature";
                menuBuilder << "RicIntersectionBoxAtPosFeature";

                menuBuilder << "RicIntersectionBoxXSliceFeature";
                menuBuilder << "RicIntersectionBoxYSliceFeature";
                menuBuilder << "RicIntersectionBoxZSliceFeature";
            }

            menuBuilder.subMenuEnd();

            menuBuilder.addSeparator();

            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
            if ( eclipseView )
            {
                // fault commands
                const RigFault* fault = eclipseView->mainGrid()->findFaultFromCellIndexAndCellFace( m_currentCellIndex, m_currentFaceIndex );
                if ( fault )
                {
                    menuBuilder.addSeparator();

                    QString      faultName = fault->name();
                    QVariantList faultDataList;
                    qulonglong   currentCellIndex = m_currentCellIndex;
                    faultDataList.push_back( currentCellIndex );
                    faultDataList.push_back( m_currentFaceIndex );

                    menuBuilder.addCmdFeatureWithUserData( "RicEclipseHideFaultFeature", QString( "Hide " ) + faultName, faultDataList );

                    menuBuilder.addCmdFeatureWithUserData( "RicEclipseShowOnlyFaultFeature",
                                                           QString( "Show " ) + faultName + QString( " - Others Off" ),
                                                           QVariant( fault->name() ) );

                    menuBuilder.addCmdFeatureWithUserData( "RicNewFaultReactModelingFeature",
                                                           QString( "New Fault Re-activation Model" ),
                                                           faultDataList );

                    menuBuilder.addSeparator();
                }
            }

            menuBuilder << "RicToggleMeasurementModeFeature";
            menuBuilder << "RicTogglePolyMeasurementModeFeature";
        }
        menuBuilder.addCmdFeature( "RicCreatePolygonFeature", "Polygon" );
    }

    // Well log curve creation commands
    if ( firstHitPart && firstHitPart->sourceInfo() )
    {
        RimWellPath* wellPath           = nullptr;
        const auto*  wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>( firstHitPart->sourceInfo() );
        if ( wellPathSourceInfo )
        {
            wellPath = wellPathSourceInfo->wellPath();
        }

        RimWellPathComponentInterface* wellPathComponent = nullptr;
        if ( additionalHitPart )
        {
            const RivObjectSourceInfo* objectSourceInfo = dynamic_cast<const RivObjectSourceInfo*>( additionalHitPart->sourceInfo() );
            if ( objectSourceInfo )
            {
                wellPathComponent = dynamic_cast<RimWellPathComponentInterface*>( objectSourceInfo->object() );
            }
        }

        if ( wellPath )
        {
            if ( firstPartTriangleIndex != cvf::UNDEFINED_UINT )
            {
                cvf::Vec3d pickedPositionInUTM = m_currentPickPositionInDomainCoords;
                if ( int2dView ) pickedPositionInUTM = int2dView->transformToUtm( pickedPositionInUTM );

                double     measuredDepth = wellPathSourceInfo->measuredDepth( firstPartTriangleIndex, pickedPositionInUTM );
                cvf::Vec3d closestPointOnCenterLine =
                    wellPathSourceInfo->closestPointOnCenterLine( firstPartTriangleIndex, pickedPositionInUTM );
                RiuSelectionItem* selItem =
                    new RiuWellPathSelectionItem( wellPathSourceInfo, closestPointOnCenterLine, measuredDepth, wellPathComponent );
                Riu3dSelectionManager::instance()->setSelectedItem( selItem, Riu3dSelectionManager::RUI_TEMPORARY );
            }

            // TODO: Update so these also use RiuWellPathSelectionItem
            caf::SelectionManager::instance()->setSelectedItem( wellPath );

            menuBuilder << "RicNewWellLogExtractionCurveFeature";
            menuBuilder << "RicNewWellLogFileCurveFeature";

            menuBuilder.addSeparator();

            menuBuilder.subMenuStart( "Well Plots", QIcon( ":/WellLogTrack16x16.png" ) );

            menuBuilder << "RicNewRftPlotFeature";
            menuBuilder << "RicNewPltPlotFeature";

            menuBuilder.addSeparator();

            menuBuilder << "RicShowWellAllocationPlotFeature";
            menuBuilder << "RicNewWellBoreStabilityPlotFeature";

            menuBuilder.subMenuEnd();

            menuBuilder.addSeparator();

            menuBuilder.subMenuStart( "3D Well Log Curves", QIcon( ":/WellLogCurve16x16.png" ) );

            menuBuilder << "RicAdd3dWellLogCurveFeature";
            menuBuilder << "RicAdd3dWellLogFileCurveFeature";

            menuBuilder.subMenuEnd();

            menuBuilder.addSeparator();

            if ( wellPath->wellIASettingsCollection()->isEnabled() )
            {
                menuBuilder << "RicNewWellIntegrityAnalysisFeature";
                menuBuilder.addSeparator();
            }

            menuBuilder.subMenuStart( "Create Completions", QIcon( ":/FishBoneGroup16x16.png" ) );

            menuBuilder << "RicNewPerforationIntervalAtMeasuredDepthFeature";
            menuBuilder << "RicNewValveAtMeasuredDepthFeature";
            menuBuilder << "RicNewFishbonesSubsAtMeasuredDepthFeature";
            menuBuilder << "RicNewWellPathFractureAtPosFeature";
            menuBuilder << "RicNewWellPathStimPlanModelAtPosFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicNewWellPathAttributeFeature";

            menuBuilder.subMenuEnd();

            menuBuilder.addSeparator();
            menuBuilder << "RicNewWellPathLateralAtDepthFeature";
            menuBuilder << "RicNewWellPathIntersectionFeature";
            menuBuilder << "RicLinkWellPathFeature";
            menuBuilder << "RicDuplicateWellPathFeature";
        }

        const auto* eclipseWellSourceInfo = dynamic_cast<const RivSimWellPipeSourceInfo*>( firstHitPart->sourceInfo() );
        if ( eclipseWellSourceInfo )
        {
            RimSimWellInView* well = eclipseWellSourceInfo->well();
            if ( well )
            {
                caf::SelectionManager::instance()->setSelectedItem( well );

                RiuSelectionItem* selItem = new RiuSimWellSelectionItem( eclipseWellSourceInfo->well(),
                                                                         m_currentPickPositionInDomainCoords,
                                                                         eclipseWellSourceInfo->branchIndex() );
                Riu3dSelectionManager::instance()->setSelectedItem( selItem, Riu3dSelectionManager::RUI_TEMPORARY );

                menuBuilder << "RicNewWellLogExtractionCurveFeature";
                menuBuilder << "RicNewWellLogRftCurveFeature";

                menuBuilder.addSeparator();

                menuBuilder.subMenuStart( "Well Plots", QIcon( ":/WellLogTrack16x16.png" ) );

                menuBuilder << "RicNewRftPlotFeature";
                menuBuilder << "RicNewPltPlotFeature";

                menuBuilder.addSeparator();

                menuBuilder << "RicPlotProductionRateFeature";
                menuBuilder << "RicShowWellAllocationPlotFeature";
                menuBuilder << "RicShowCumulativePhasePlotFeature";

                menuBuilder.subMenuEnd();

                menuBuilder.addSeparator();
                menuBuilder << "RicShowContributingWellsFeature";
                menuBuilder.addSeparator();
                menuBuilder << "RicNewSimWellIntersectionFeature";
            }
        }
    }

    // View Link commands
    if ( !firstHitPart )
    {
        if ( gridView || int2dView )
        {
            menuBuilder << "RicLinkViewFeature";
            menuBuilder << "RicShowLinkOptionsFeature";
            menuBuilder << "RicSetMasterViewFeature";
            addCompareToViewMenu( &menuBuilder );
            menuBuilder.addSeparator();
            menuBuilder << "RicUnLinkViewFeature";
            menuBuilder << "RicRemoveComparison3dViewFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicSelectColorResult";
        }
    }

    if ( gridView )
    {
        bool isContourView = dynamic_cast<RimEclipseContourMapView*>( gridView ) || dynamic_cast<RimGeoMechContourMapView*>( gridView );
        if ( isContourView )
        {
            menuBuilder << "RicCreateContourMapPolygonFeature";
            menuBuilder << "RicCreateContourMapPolygonAdvancedFeature";
            menuBuilder.addSeparator();
            menuBuilder << "RicExportContourMapToTextFeature";
        }
        else
        {
            menuBuilder.addSeparator();
            menuBuilder << "RicNewGridTimeHistoryCurveFeature";
            menuBuilder << "RicShowFlowCharacteristicsPlotFeature";
            if ( dynamic_cast<RimEclipseView*>( gridView ) )
            {
                menuBuilder << "RicCreateGridCrossPlotFeature";
            }
            menuBuilder.addSeparator();
            menuBuilder.subMenuStart( "Export" );
            menuBuilder << "RicExportEclipseInputGridFeature";
            menuBuilder << "RicSaveEclipseInputActiveVisibleCellsFeature";
            menuBuilder << "RicSaveEclipseResultAsInputPropertyFeature";
            menuBuilder.subMenuEnd();
            menuBuilder.addSeparator();

            menuBuilder << "RicCreateGridStatisticsPlotFeature";
            menuBuilder << "RicShowGridStatisticsFeature";
            menuBuilder << "RicCopyGridStatisticsToClipboardFeature";
            menuBuilder << "RicSelectColorResult";
        }
    }

    if ( firstHitPart )
    {
        menuBuilder.addSeparator();
        menuBuilder << "RicCreateTextAnnotationIn3dViewFeature";
    }

    menuBuilder.appendToMenu( &menu );

    if ( !menu.isEmpty() )
    {
        menu.exec( event->globalPos() );
    }

    caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget( nullptr );

    // Delete items in temporary selection
    Riu3dSelectionManager::instance()->deleteAllItems( Riu3dSelectionManager::RUI_TEMPORARY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::handlePickAction( int winPosX, int winPosY, Qt::KeyboardModifiers keyboardModifiers )
{
    // Overlay item picking

    if ( handleOverlayItemPicking( winPosX, winPosY ) )
    {
        return;
    }

    // Do the ray intersection with the scene

    std::vector<RiuPickItemInfo> pickItemInfos;
    {
        cvf::Vec3d             globalRayOrigin;
        cvf::HitItemCollection hitItems;
        m_viewer->rayPick( winPosX, winPosY, &hitItems, &globalRayOrigin );

        // Do specialized text pick, since vizfwk does not hit text
        handleTextPicking( winPosX, winPosY, &hitItems );

        if ( hitItems.count() )
        {
            pickItemInfos = RiuPickItemInfo::convertToPickItemInfos( hitItems, globalRayOrigin );
        }
    }

    m_isCurrentPickInComparisonView = m_viewer->isMousePosWithinComparisonView( winPosX, winPosY );
    Rim3dView* mainOrComparisonView = m_isCurrentPickInComparisonView ? m_reservoirView->activeComparisonView() : m_reservoirView.p();

    // Make pickEventHandlers do their stuff

    if ( !pickItemInfos.empty() )
    {
        Ric3dPickEvent viewerEventObject( pickItemInfos, mainOrComparisonView, keyboardModifiers );

        if ( sm_overridingPickHandler && sm_overridingPickHandler->handle3dPickEvent( viewerEventObject ) )
        {
            return;
        }

        for ( size_t i = 0; i < sm_defaultPickEventHandlers.size(); i++ )
        {
            if ( sm_defaultPickEventHandlers[i]->handle3dPickEvent( viewerEventObject ) )
            {
                return;
            }
        }
    }
    else
    {
        Riu3dSelectionManager::instance()->deleteAllItems();
    }

    // Old pick handling. Todo: Encapsulate in pickEventHandlers
    {
        const cvf::Part* firstHitPart           = nullptr;
        uint             firstPartTriangleIndex = cvf::UNDEFINED_UINT;
        cvf::Vec3d       globalIntersectionPoint( cvf::Vec3d::ZERO );

        if ( !pickItemInfos.empty() )
        {
            size_t indexToFirstNoneNncItem     = cvf::UNDEFINED_SIZE_T;
            size_t indexToNncItemNearFirstItem = cvf::UNDEFINED_SIZE_T;

            findFirstItems( mainOrComparisonView, pickItemInfos, &indexToFirstNoneNncItem, &indexToNncItemNearFirstItem );

            if ( indexToFirstNoneNncItem != cvf::UNDEFINED_SIZE_T )
            {
                firstHitPart            = pickItemInfos[indexToFirstNoneNncItem].pickedPart();
                firstPartTriangleIndex  = pickItemInfos[indexToFirstNoneNncItem].faceIdx();
                globalIntersectionPoint = pickItemInfos[indexToFirstNoneNncItem].globalPickedPoint();
            }
        }

        if ( firstHitPart && firstHitPart->sourceInfo() )
        {
            const auto* rivObjectSourceInfo      = dynamic_cast<const RivObjectSourceInfo*>( firstHitPart->sourceInfo() );
            const auto* eclipseWellSourceInfo    = dynamic_cast<const RivSimWellPipeSourceInfo*>( firstHitPart->sourceInfo() );
            const auto* wellConnectionSourceInfo = dynamic_cast<const RivWellConnectionSourceInfo*>( firstHitPart->sourceInfo() );
            const auto* seismicSourceInfo        = dynamic_cast<const RivSeismicSectionSourceInfo*>( firstHitPart->sourceInfo() );

            if ( rivObjectSourceInfo )
            {
                RimFracture* fracture = dynamic_cast<RimFracture*>( rivObjectSourceInfo->object() );
                if ( fracture )
                {
                    {
                        bool blockSelectionOfFracture = false;

                        {
                            std::vector<caf::PdmUiItem*> uiItems;
                            RiuMainWindow::instance()->projectTreeView( 0 )->selectedUiItems( uiItems );

                            if ( uiItems.size() == 1 )
                            {
                                auto selectedFractureTemplate = dynamic_cast<RimFractureTemplate*>( uiItems[0] );

                                if ( selectedFractureTemplate != nullptr && selectedFractureTemplate == fracture->fractureTemplate() )
                                {
                                    blockSelectionOfFracture = true;
                                }
                            }
                        }

                        if ( !blockSelectionOfFracture )
                        {
                            RiuMainWindow::instance()->selectAsCurrentItem( fracture );
                        }
                    }

                    RimMeshFractureTemplate* stimPlanTempl = fracture ? dynamic_cast<RimMeshFractureTemplate*>( fracture->fractureTemplate() )
                                                                      : nullptr;
                    RimEllipseFractureTemplate* ellipseTempl =
                        fracture ? dynamic_cast<RimEllipseFractureTemplate*>( fracture->fractureTemplate() ) : nullptr;
                    if ( stimPlanTempl || ellipseTempl )
                    {
                        // Set fracture resultInfo text
                        QString resultInfoText;

                        cvf::ref<caf::DisplayCoordTransform> transForm   = mainOrComparisonView->displayCoordTransform();
                        cvf::Vec3d                           domainCoord = transForm->transformToDomainCoord( globalIntersectionPoint );

                        RimEclipseView*         eclView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
                        RivWellFracturePartMgr* partMgr = fracture->fracturePartManager();
                        if ( eclView ) resultInfoText = partMgr->resultInfoText( *eclView, domainCoord );

                        // Set intersection point result text
                        QString intersectionPointText = QString( "Intersection point : Global [E: %1, N: %2, Depth: %3]" )
                                                            .arg( domainCoord.x(), 5, 'f', 2 )
                                                            .arg( domainCoord.y(), 5, 'f', 2 )
                                                            .arg( -domainCoord.z(), 5, 'f', 2 );
                        resultInfoText.append( intersectionPointText );

                        // Display result info text
                        RiuMainWindow::instance()->setResultInfo( resultInfoText );
                    }
                }

                RimTextAnnotation* textAnnot = dynamic_cast<RimTextAnnotation*>( rivObjectSourceInfo->object() );
                if ( textAnnot )
                {
                    RiuMainWindow::instance()->selectAsCurrentItem( textAnnot, true );
                }

                RimSurfaceInView* surf = dynamic_cast<RimSurfaceInView*>( rivObjectSourceInfo->object() );
                if ( surf )
                {
                    RiuMainWindow::instance()->selectAsCurrentItem( surf, true );
                    cvf::ref<caf::DisplayCoordTransform> transForm   = mainOrComparisonView->displayCoordTransform();
                    cvf::Vec3d                           domainCoord = transForm->transformToDomainCoord( globalIntersectionPoint );

                    // Set surface resultInfo text
                    QString resultInfoText = "Surface: " + surf->name() + "\n\n";

                    RivSurfacePartMgr* partMgr = surf->surfacePartMgr();
                    resultInfoText += partMgr->resultInfoText( mainOrComparisonView, firstPartTriangleIndex, domainCoord );

                    // Set intersection point result text
                    QString intersectionPointText = QString( "Intersection point : Global [E: %1, N: %2, Depth: %3]" )
                                                        .arg( domainCoord.x(), 5, 'f', 2 )
                                                        .arg( domainCoord.y(), 5, 'f', 2 )
                                                        .arg( -domainCoord.z(), 5, 'f', 2 );
                    resultInfoText.append( intersectionPointText );

                    // Display result info text
                    RiuMainWindow::instance()->setResultInfo( resultInfoText );
                }
            }
            else if ( const auto* surfIntersectSourceInfo =
                          dynamic_cast<const RivReservoirSurfaceIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
            {
                RiuMainWindow::instance()->selectAsCurrentItem( surfIntersectSourceInfo->intersection() );
            }

            else if ( const auto* crossSectionSourceInfo =
                          dynamic_cast<const RivExtrudedCurveIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
            {
                bool allowActiveViewChange = dynamic_cast<Rim2dIntersectionView*>( m_viewer->ownerViewWindow() ) == nullptr;

                RiuMainWindow::instance()->selectAsCurrentItem( crossSectionSourceInfo->intersection(), allowActiveViewChange );
            }
            else if ( const auto* intersectionBoxSourceInfo = dynamic_cast<const RivBoxIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
            {
                RiuMainWindow::instance()->selectAsCurrentItem( intersectionBoxSourceInfo->intersectionBox() );
            }
            else if ( eclipseWellSourceInfo )
            {
                bool allowActiveViewChange = dynamic_cast<Rim2dIntersectionView*>( m_viewer->ownerViewWindow() ) == nullptr;

                RiuPlotMainWindow::onWellSelected( eclipseWellSourceInfo->well()->name(), mainOrComparisonView->currentTimeStep() );

                RiuMainWindow::instance()->selectAsCurrentItem( eclipseWellSourceInfo->well(), allowActiveViewChange );
            }
            else if ( wellConnectionSourceInfo )
            {
                bool allowActiveViewChange = dynamic_cast<Rim2dIntersectionView*>( m_viewer->ownerViewWindow() ) == nullptr;

                size_t globalCellIndex = wellConnectionSourceInfo->globalCellIndexFromTriangleIndex( firstPartTriangleIndex );

                RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
                if ( eclipseView )
                {
                    auto eclipseCase = eclipseView->firstAncestorOrThisOfType<RimEclipseCase>();
                    if ( eclipseCase->eclipseCaseData() && eclipseCase->eclipseCaseData()->virtualPerforationTransmissibilities() )
                    {
                        std::vector<RigCompletionData> completionsForOneCell;

                        {
                            auto   connectionFactors = eclipseCase->eclipseCaseData()->virtualPerforationTransmissibilities();
                            size_t timeStep          = eclipseView->currentTimeStep();

                            const auto& multipleCompletions =
                                connectionFactors->multipleCompletionsPerEclipseCell( wellConnectionSourceInfo->wellPath(), timeStep );

                            auto completionDataIt = multipleCompletions.find( globalCellIndex );
                            if ( completionDataIt != multipleCompletions.end() )
                            {
                                completionsForOneCell = completionDataIt->second;
                            }
                        }

                        if ( !completionsForOneCell.empty() )
                        {
                            double aggregatedConnectionFactor = 0.0;
                            for ( const auto& completionData : completionsForOneCell )
                            {
                                aggregatedConnectionFactor += completionData.transmissibility();
                            }

                            QString resultInfoText;
                            resultInfoText += QString( "<b>Well Connection Factor :</b> %1<br><br>" ).arg( aggregatedConnectionFactor );

                            {
                                RiuResultTextBuilder textBuilder( eclipseView,
                                                                  eclipseView->cellResult(),
                                                                  globalCellIndex,
                                                                  eclipseView->currentTimeStep() );

                                resultInfoText += textBuilder.geometrySelectionText( "<br>" );
                            }

                            resultInfoText += "<br><br>Details : <br>";

                            for ( const auto& completionData : completionsForOneCell )
                            {
                                for ( const auto& metaData : completionData.metadata() )
                                {
                                    resultInfoText +=
                                        QString( "<b>Name</b> %1 <b>Description</b> %2 <br>" ).arg( metaData.name ).arg( metaData.comment );
                                }
                            }

                            RiuMainWindow::instance()->setResultInfo( resultInfoText );
                        }
                    }
                }

                RiuMainWindow::instance()->selectAsCurrentItem( wellConnectionSourceInfo->wellPath(), allowActiveViewChange );
            }
            else if ( dynamic_cast<const RivSimWellConnectionSourceInfo*>( firstHitPart->sourceInfo() ) )
            {
                const RivSimWellConnectionSourceInfo* simWellConnectionSourceInfo =
                    dynamic_cast<const RivSimWellConnectionSourceInfo*>( firstHitPart->sourceInfo() );

                bool allowActiveViewChange = dynamic_cast<Rim2dIntersectionView*>( m_viewer->ownerViewWindow() ) == nullptr;

                size_t globalCellIndex  = simWellConnectionSourceInfo->globalCellIndexFromTriangleIndex( firstPartTriangleIndex );
                double connectionFactor = simWellConnectionSourceInfo->connectionFactorFromTriangleIndex( firstPartTriangleIndex );

                RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
                if ( eclipseView )
                {
                    auto eclipseCase = eclipseView->firstAncestorOrThisOfType<RimEclipseCase>();
                    if ( eclipseCase->eclipseCaseData() && eclipseCase->eclipseCaseData()->virtualPerforationTransmissibilities() )
                    {
                        auto   connectionFactors = eclipseCase->eclipseCaseData()->virtualPerforationTransmissibilities();
                        size_t timeStep          = eclipseView->currentTimeStep();

                        const auto& completionData =
                            connectionFactors->completionsForSimWell( simWellConnectionSourceInfo->simWellInView()->simWellData(), timeStep );

                        for ( const auto& compData : completionData )
                        {
                            if ( compData.completionDataGridCell().globalCellIndex() == globalCellIndex )
                            {
                                {
                                    QString resultInfoText =
                                        QString( "<b>Simulation Well Connection Factor :</b> %1<br><br>" ).arg( connectionFactor );

                                    {
                                        RiuResultTextBuilder textBuilder( eclipseView,
                                                                          eclipseView->cellResult(),
                                                                          globalCellIndex,
                                                                          eclipseView->currentTimeStep() );

                                        resultInfoText += textBuilder.geometrySelectionText( "<br>" );
                                    }

                                    RiuMainWindow::instance()->setResultInfo( resultInfoText );
                                }

                                break;
                            }
                        }
                    }
                }
                RiuMainWindow::instance()->selectAsCurrentItem( simWellConnectionSourceInfo->simWellInView(), allowActiveViewChange );
            }
            else if ( seismicSourceInfo != nullptr )
            {
                auto section = seismicSourceInfo->section();

                RiuMainWindow::instance()->selectAsCurrentItem( section );

                cvf::ref<caf::DisplayCoordTransform> transForm   = mainOrComparisonView->displayCoordTransform();
                cvf::Vec3d                           domainCoord = transForm->transformToDomainCoord( globalIntersectionPoint );

                // Set surface resultInfo text
                QString resultInfoText = "Seismic Section: \"" + section->fullName() + "\"\n\n";

                resultInfoText += section->resultInfoText( domainCoord, seismicSourceInfo->partIndex() );

                // Set intersection point result text
                QString pointText = QString( "Global point : [E: %1, N: %2, Depth: %3]" )
                                        .arg( domainCoord.x(), 5, 'f', 2 )
                                        .arg( domainCoord.y(), 5, 'f', 2 )
                                        .arg( -domainCoord.z(), 5, 'f', 2 );
                resultInfoText.append( pointText );

                // Display result info text
                RiuMainWindow::instance()->setResultInfo( resultInfoText );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::setPickEventHandler( Ric3dViewPickEventHandler* pickEventHandler )
{
    if ( sm_overridingPickHandler ) sm_overridingPickHandler->notifyUnregistered();

    sm_overridingPickHandler = pickEventHandler;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::removePickEventHandlerIfActive( Ric3dViewPickEventHandler* pickEventHandler )
{
    if ( sm_overridingPickHandler == pickEventHandler )
    {
        sm_overridingPickHandler = nullptr;
        if ( pickEventHandler ) pickEventHandler->notifyUnregistered();
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
bool RiuViewerCommands::isCurrentPickInComparisonView() const
{
    return m_isCurrentPickInComparisonView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::addDefaultPickEventHandler( RicDefaultPickEventHandler* pickEventHandler )
{
    removeDefaultPickEventHandler( pickEventHandler );
    if ( pickEventHandler )
    {
        sm_defaultPickEventHandlers.push_back( pickEventHandler );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::removeDefaultPickEventHandler( RicDefaultPickEventHandler* pickEventHandler )
{
    for ( auto it = sm_defaultPickEventHandlers.begin(); it != sm_defaultPickEventHandlers.end(); ++it )
    {
        if ( *it == pickEventHandler )
        {
            sm_defaultPickEventHandlers.erase( it );
            break;
        }
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::findCellAndGridIndex( Rim3dView*                       mainOrComparisonView,
                                              RimIntersectionResultDefinition* sepInterResDef,
                                              size_t                           globalCellIndex,
                                              size_t*                          cellIndex,
                                              size_t*                          gridIndex )
{
    CVF_ASSERT( cellIndex && gridIndex );
    RimEclipseCase* eclipseCase = nullptr;
    RimGeoMechCase* geomechCase = dynamic_cast<RimGeoMechCase*>( mainOrComparisonView->ownerCase() );

    if ( sepInterResDef )
    {
        if ( sepInterResDef->isEclipseResultDefinition() )
        {
            eclipseCase = dynamic_cast<RimEclipseCase*>( sepInterResDef->activeCase() );
        }
    }
    else
    {
        eclipseCase = dynamic_cast<RimEclipseCase*>( mainOrComparisonView->ownerCase() );
    }

    if ( eclipseCase )
    {
        const RigCell& cell = eclipseCase->mainGrid()->cell( globalCellIndex );
        *cellIndex          = cell.gridLocalCellIndex();
        *gridIndex          = cell.hostGrid()->gridIndex();
    }
    else if ( geomechCase )
    {
        RigFemPartCollection* parts = geomechCase->geoMechData()->femParts();
        auto [partId, elementIdx]   = parts->partIdAndElementIndex( globalCellIndex );
        *cellIndex                  = elementIdx;
        *gridIndex                  = (size_t)partId;
    }
    else
    {
        *cellIndex = globalCellIndex;
        *gridIndex = 0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::findFirstItems( Rim3dView*                          mainOrComparisonView,
                                        const std::vector<RiuPickItemInfo>& pickItemInfos,
                                        size_t*                             indexToFirstNoneNncItem,
                                        size_t*                             indexToNncItemNearFirsItem )
{
    CVF_ASSERT( !pickItemInfos.empty() );
    CVF_ASSERT( indexToFirstNoneNncItem );
    CVF_ASSERT( indexToNncItemNearFirsItem );

    double pickDepthThresholdSquared = 0.05 * 0.05;
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );

        if ( eclipseView && eclipseView->mainGrid() )
        {
            double characteristicCellSize = eclipseView->mainGrid()->characteristicIJCellSize();
            pickDepthThresholdSquared     = characteristicCellSize / 100.0;
            pickDepthThresholdSquared     = pickDepthThresholdSquared * pickDepthThresholdSquared;
        }
    }

    size_t     firstNonNncHitIndex                 = cvf::UNDEFINED_SIZE_T;
    size_t     nncNearFirstItemIndex               = cvf::UNDEFINED_SIZE_T;
    cvf::Vec3d firstOrFirstNonNncIntersectionPoint = pickItemInfos[0].globalPickedPoint();

    // Find first nnc part, and store as a separate thing if the nnc is first or close behind the first hit item.
    // Find index to first ordinary (non-nnc) part

    for ( size_t i = 0; i < pickItemInfos.size(); i++ )
    {
        // If hit item is nnc and is close to first (none-nnc) hit, store nncpart and face id

        bool canFindRelvantNNC = true;

        const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>( pickItemInfos[i].sourceInfo() );
        if ( rivSourceInfo && rivSourceInfo->hasNNCIndices() )
        {
            if ( nncNearFirstItemIndex == cvf::UNDEFINED_SIZE_T && canFindRelvantNNC )
            {
                cvf::Vec3d distFirstNonNNCToCandidate = firstOrFirstNonNncIntersectionPoint - pickItemInfos[i].globalPickedPoint();

                // This candidate is an NNC hit
                if ( distFirstNonNNCToCandidate.lengthSquared() < pickDepthThresholdSquared )
                {
                    nncNearFirstItemIndex = i;
                }
                else
                {
                    canFindRelvantNNC = false;
                }
            }
        }
        else
        {
            if ( firstNonNncHitIndex == cvf::UNDEFINED_SIZE_T )
            {
                firstNonNncHitIndex = i;
            }
        }

        if ( firstNonNncHitIndex != cvf::UNDEFINED_SIZE_T && ( nncNearFirstItemIndex != cvf::UNDEFINED_SIZE_T || !canFindRelvantNNC ) )
        {
            break; // Found what can be found
        }
    }

    ( *indexToFirstNoneNncItem )    = firstNonNncHitIndex;
    ( *indexToNncItemNearFirsItem ) = nncNearFirstItemIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::ijkFromCellIndex( Rim3dView* mainOrComparisonView, size_t gridIdx, size_t cellIndex, size_t* i, size_t* j, size_t* k )
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
    RimGeoMechView* geomView    = dynamic_cast<RimGeoMechView*>( mainOrComparisonView );

    if ( eclipseView && eclipseView->eclipseCase() )
    {
        eclipseView->eclipseCase()->eclipseCaseData()->grid( gridIdx )->ijkFromCellIndex( cellIndex, i, j, k );
    }

    if ( geomView && geomView->geoMechCase() )
    {
        geomView->femParts()->part( gridIdx )->getOrCreateStructGrid()->ijkFromCellIndex( cellIndex, i, j, k );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuViewerCommands::handleOverlayItemPicking( int winPosX, int winPosY )
{
    if ( dynamic_cast<Rim2dIntersectionView*>( m_reservoirView.p() ) )
    {
        return false;
    }

    cvf::OverlayItem* pickedOverlayItem = m_viewer->overlayItem( winPosX, winPosY );
    if ( !pickedOverlayItem )
    {
        pickedOverlayItem = m_viewer->pickFixedPositionedLegend( winPosX, winPosY );
    }

    if ( pickedOverlayItem )
    {
        std::vector<RimLegendConfig*> legendConfigs = m_reservoirView->legendConfigs();
        if ( m_reservoirView->activeComparisonView() )
        {
            std::vector<RimLegendConfig*> compViewLegendConfigs = m_reservoirView->activeComparisonView()->legendConfigs();
            legendConfigs.insert( legendConfigs.end(), compViewLegendConfigs.begin(), compViewLegendConfigs.end() );
        }

        for ( const auto& legendConfig : legendConfigs )
        {
            if ( legendConfig && legendConfig->titledOverlayFrame() == pickedOverlayItem )
            {
                auto seisInterface = legendConfig->firstAncestorOfType<RimSeismicDataInterface>();
                if ( seisInterface != nullptr )
                {
                    RiuMainWindow::instance()->selectAsCurrentItem( seisInterface );
                }
                else
                {
                    RiuMainWindow::instance()->selectAsCurrentItem( legendConfig );
                }

                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::handleTextPicking( int winPosX, int winPosY, cvf::HitItemCollection* hitItems )
{
    using namespace cvf;

    m_isCurrentPickInComparisonView = m_viewer->isMousePosWithinComparisonView( winPosX, winPosY );

    int translatedMousePosX = winPosX;
    int translatedMousePosY = m_viewer->height() - winPosY;

    Scene* scene = m_viewer->currentScene( m_isCurrentPickInComparisonView );

    if ( !scene ) return;

    Collection<Part> partCollection;
    scene->allParts( &partCollection );

    ref<Ray> ray = m_viewer->mainCamera()->rayFromWindowCoordinates( translatedMousePosX, translatedMousePosY );

    if ( ray.notNull() )
    {
        for ( size_t pIdx = 0; pIdx < partCollection.size(); ++pIdx )
        {
            DrawableText* textDrawable = dynamic_cast<DrawableText*>( partCollection[pIdx]->drawable() );
            if ( textDrawable )
            {
                cvf::Vec3d ppoint;
                if ( textDrawable->rayIntersect( *ray, *( m_viewer->mainCamera() ), &ppoint ) )
                {
                    cvf::ref<HitItem> hitItem = new HitItem( 0, ppoint );
                    hitItem->setPart( partCollection[pIdx].p() );
                    hitItems->add( hitItem.p() );
                }
            }
        }
    }

    hitItems->sort();
}
