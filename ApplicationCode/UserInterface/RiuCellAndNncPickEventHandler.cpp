/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiuCellAndNncPickEventHandler.h"

#include "RiaColorTables.h"

#include "Rim2dIntersectionView.h"
#include "RimBoxIntersection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimSurfaceInView.h"

#include "Riu3dSelectionManager.h"
#include "RiuViewerCommands.h"

#include "RivBoxIntersectionSourceInfo.h"
#include "RivExtrudedCurveIntersectionSourceInfo.h"
#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivReservoirSurfaceIntersectionSourceInfo.h"
#include "RivSourceInfo.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"

#include "cafPdmObjectHandle.h"

#include "cvfPart.h"

#include <array>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCellAndNncPickEventHandler* RiuCellAndNncPickEventHandler::instance()
{
    static RiuCellAndNncPickEventHandler* singleton = new RiuCellAndNncPickEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuCellAndNncPickEventHandler::handle3dPickEvent( const Ric3dPickEvent& eventObject )
{
    const std::vector<RiuPickItemInfo>& pickItemInfos        = eventObject.m_pickItemInfos;
    Rim3dView*                          mainOrComparisonView = eventObject.m_view;
    Qt::KeyboardModifiers               keyboardModifiers    = eventObject.m_keyboardModifiers;

    const cvf::Part* firstHitPart           = nullptr;
    uint             firstPartTriangleIndex = cvf::UNDEFINED_UINT;

    cvf::Vec3d localIntersectionPoint( cvf::Vec3d::ZERO );
    size_t     nncIndex = cvf::UNDEFINED_SIZE_T;

    {
        const cvf::Part* firstNncHitPart      = nullptr;
        uint             nncPartTriangleIndex = cvf::UNDEFINED_UINT;

        if ( pickItemInfos.size() )
        {
            size_t indexToFirstNoneNncItem     = cvf::UNDEFINED_SIZE_T;
            size_t indexToNncItemNearFirstItem = cvf::UNDEFINED_SIZE_T;

            RiuViewerCommands::findFirstItems( mainOrComparisonView,
                                               pickItemInfos,
                                               &indexToFirstNoneNncItem,
                                               &indexToNncItemNearFirstItem );

            if ( indexToFirstNoneNncItem != cvf::UNDEFINED_SIZE_T )
            {
                localIntersectionPoint = pickItemInfos[indexToFirstNoneNncItem].localPickedPoint();
                firstHitPart           = pickItemInfos[indexToFirstNoneNncItem].pickedPart();
                firstPartTriangleIndex = pickItemInfos[indexToFirstNoneNncItem].faceIdx();
            }

            if ( indexToNncItemNearFirstItem != cvf::UNDEFINED_SIZE_T )
            {
                firstNncHitPart      = pickItemInfos[indexToNncItemNearFirstItem].pickedPart();
                nncPartTriangleIndex = pickItemInfos[indexToNncItemNearFirstItem].faceIdx();
            }
        }

        if ( firstNncHitPart && firstNncHitPart->sourceInfo() )
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>( firstNncHitPart->sourceInfo() );
            if ( rivSourceInfo )
            {
                if ( nncPartTriangleIndex < rivSourceInfo->m_NNCIndices->size() )
                {
                    nncIndex = rivSourceInfo->m_NNCIndices->get( nncPartTriangleIndex );
                }
            }
        }

        if ( !firstHitPart && firstNncHitPart )
        {
            // This happen if we only have NNC geometry in the scene
            firstHitPart = firstNncHitPart;
        }
    }

    if ( !firstHitPart ) return false;

    size_t                             gridIndex          = cvf::UNDEFINED_SIZE_T;
    size_t                             gridLocalCellIndex = cvf::UNDEFINED_SIZE_T;
    cvf::StructGridInterface::FaceType face               = cvf::StructGridInterface::NO_FACE;
    int                                gmFace             = -1;
    bool                               intersectionHit    = false;
    std::array<cvf::Vec3f, 3>          intersectionTriangleHit;
    RimGeoMechResultDefinition*        geomResDef     = nullptr;
    RimEclipseResultDefinition*        eclResDef      = nullptr;
    size_t                             timestepIndex  = cvf::UNDEFINED_SIZE_T;
    RimIntersectionResultDefinition*   sepInterResDef = nullptr;

    // clang-format off
    if ( const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>( firstHitPart->sourceInfo() ) )
    {
        gridIndex = rivSourceInfo->gridIndex();
        if ( rivSourceInfo->hasCellFaceMapping() )
        {
            CVF_ASSERT( rivSourceInfo->m_cellFaceFromTriangleMapper.notNull() );

            gridLocalCellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex( firstPartTriangleIndex );
            face      = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace( firstPartTriangleIndex );
        }
    }
    else if ( const RivFemPickSourceInfo* femSourceInfo = 
                dynamic_cast<const RivFemPickSourceInfo*>( firstHitPart->sourceInfo() ) )
    {
        gridIndex = femSourceInfo->femPartIndex();
        gridLocalCellIndex = femSourceInfo->triangleToElmMapper()->elementIndex( firstPartTriangleIndex );
        gmFace    = femSourceInfo->triangleToElmMapper()->elementFace( firstPartTriangleIndex );
    }
	else if ( const RivReservoirSurfaceIntersectionSourceInfo* surfeIntersectSourceInfo = 
                dynamic_cast<const RivReservoirSurfaceIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
    {
		RiuViewerCommands::findCellAndGridIndex( mainOrComparisonView,
												 surfeIntersectSourceInfo->intersection()->activeSeparateResultDefinition(),
												 surfeIntersectSourceInfo->triangleToCellIndex()[firstPartTriangleIndex],
												 &gridLocalCellIndex,
												 &gridIndex );

		intersectionHit = true;
        intersectionTriangleHit = surfeIntersectSourceInfo->triangle( firstPartTriangleIndex );
		sepInterResDef = surfeIntersectSourceInfo->intersection()->activeSeparateResultDefinition();
    }
    else if ( const RivExtrudedCurveIntersectionSourceInfo* intersectionSourceInfo = 
                dynamic_cast<const RivExtrudedCurveIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
    {
		RiuViewerCommands::findCellAndGridIndex( mainOrComparisonView,
												 intersectionSourceInfo->intersection()->activeSeparateResultDefinition(),
												 intersectionSourceInfo->triangleToCellIndex()[firstPartTriangleIndex],
												 &gridLocalCellIndex,
												 &gridIndex );

        intersectionHit         = true;
        intersectionTriangleHit = intersectionSourceInfo->triangle( firstPartTriangleIndex );
		sepInterResDef = intersectionSourceInfo->intersection()->activeSeparateResultDefinition();
    }
    else if ( const RivBoxIntersectionSourceInfo* intersectionBoxSourceInfo = 
                dynamic_cast<const RivBoxIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
    {
		RiuViewerCommands::findCellAndGridIndex( mainOrComparisonView,
												 intersectionBoxSourceInfo->intersectionBox()->activeSeparateResultDefinition(),
												 intersectionBoxSourceInfo->triangleToCellIndex()[firstPartTriangleIndex],
												 &gridLocalCellIndex,
												 &gridIndex );

        intersectionHit         = true;
        intersectionTriangleHit = intersectionBoxSourceInfo->triangle( firstPartTriangleIndex );
		sepInterResDef = intersectionBoxSourceInfo->intersectionBox()->activeSeparateResultDefinition();
    }
    // clang-format on

    if ( sepInterResDef )
    {
        if ( sepInterResDef->isEclipseResultDefinition() )
        {
            eclResDef = sepInterResDef->eclipseResultDefinition();
        }
        else
        {
            geomResDef = sepInterResDef->geoMechResultDefinition();
        }

        timestepIndex = sepInterResDef->timeStep();
    }

    if ( gridLocalCellIndex == cvf::UNDEFINED_SIZE_T )
    {
        if ( nncIndex != cvf::UNDEFINED_SIZE_T && dynamic_cast<RimEclipseView*>( mainOrComparisonView ) )
        {
            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
            if ( eclipseView )
            {
                RigMainGrid*         mainGrid = eclipseView->eclipseCase()->eclipseCaseData()->mainGrid();
                const RigConnection& nncConn  = mainGrid->nncData()->connections()[nncIndex];

                mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( nncConn.c1GlobIdx(), &gridLocalCellIndex );
            }
        }
        else
        {
            Riu3dSelectionManager::instance()->deleteAllItems();
            return false;
        }
    }

    bool appendToSelection = false;
    if ( keyboardModifiers & Qt::ControlModifier )
    {
        appendToSelection = true;
    }

    std::vector<RiuSelectionItem*> items;
    Riu3dSelectionManager::instance()->selectedItems( items );

    const caf::ColorTable& colorTable = RiaColorTables::selectionPaletteColors();

    cvf::Color3f curveColor = colorTable.cycledColor3f( items.size() );

    if ( !appendToSelection )
    {
        curveColor = colorTable.cycledColor3f( 0 );
    }

    RiuSelectionItem* selItem = nullptr;
    {
        Rim2dIntersectionView* intersectionView = dynamic_cast<Rim2dIntersectionView*>( mainOrComparisonView );

        RimGridView* associatedGridView = dynamic_cast<RimGridView*>( mainOrComparisonView );

        if ( intersectionView )
        {
            intersectionView->intersection()->firstAncestorOrThisOfType( associatedGridView );
        }

        // Use the clicked views default settings if we have not found any special stuff

        if ( !eclResDef && !geomResDef )
        {
            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
            RimGeoMechView* geomView    = dynamic_cast<RimGeoMechView*>( mainOrComparisonView );

            if ( !geomView && !eclipseView && associatedGridView )
            {
                if ( dynamic_cast<RimGeoMechView*>( associatedGridView ) )
                {
                    geomView = dynamic_cast<RimGeoMechView*>( associatedGridView );
                }
                else if ( dynamic_cast<RimEclipseView*>( associatedGridView ) )
                {
                    eclipseView = dynamic_cast<RimEclipseView*>( associatedGridView );
                }
            }

            if ( eclipseView )
            {
                if ( !eclResDef ) eclResDef = eclipseView->cellResult();
                if ( timestepIndex == cvf::UNDEFINED_SIZE_T ) timestepIndex = eclipseView->currentTimeStep();
            }

            if ( geomView )
            {
                if ( !geomResDef ) geomResDef = geomView->cellResult();
                if ( timestepIndex == cvf::UNDEFINED_SIZE_T ) timestepIndex = geomView->currentTimeStep();
            }
        }

        if ( eclResDef )
        {
            // Select the other cell if we are about to select the same cell at an nnc.
            // To make consecutive clicks toggle between closest and furthest cell
            // clang-format off

            if ( nncIndex != cvf::UNDEFINED_SIZE_T )
            {
                auto selectedItem = dynamic_cast<RiuEclipseSelectionItem*>( Riu3dSelectionManager::instance()->selectedItem() );

                RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(mainOrComparisonView);

                if ( selectedItem && eclipseView &&
                     selectedItem->m_gridIndex == gridIndex &&
                     selectedItem->m_gridLocalCellIndex == gridLocalCellIndex && 
                     selectedItem->m_nncIndex == nncIndex )
                {
                    RigMainGrid* mainGrid = eclipseView->eclipseCase()->eclipseCaseData()->mainGrid();
                    const RigConnection& nncConn = mainGrid->nncData()->connections()[nncIndex];

                    size_t c1LocalIdx = cvf::UNDEFINED_SIZE_T;
                    const RigGridBase* grid1 = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(nncConn.c1GlobIdx(), &c1LocalIdx);
                    size_t c1GridIdx = grid1->gridIndex();
                    size_t c2LocalIdx = cvf::UNDEFINED_SIZE_T;
                    const RigGridBase* grid2 = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(nncConn.c2GlobIdx(), &c2LocalIdx);
                    size_t c2GridIdx = grid2->gridIndex();

                    if (gridLocalCellIndex == c1LocalIdx && gridIndex == c1GridIdx)
                    {
                        gridLocalCellIndex = c2LocalIdx;
                        gridIndex = c2GridIdx; 
                        
                        if (face == cvf::StructGridInterface::NO_FACE)
                        {
                            face = nncConn.face();
                        }
                        else
                        {
                            face = cvf::StructGridInterface::oppositeFace(face);
                        }
                    }
                    else if (gridLocalCellIndex == c2LocalIdx && gridIndex == c2GridIdx)
                    {
                        gridLocalCellIndex = c1LocalIdx;
                        gridIndex = c1GridIdx;
                        if (face == cvf::StructGridInterface::NO_FACE)
                        {
                            face = cvf::StructGridInterface::oppositeFace(nncConn.face());
                        }
                        else
                        {
                            face = cvf::StructGridInterface::oppositeFace(face);
                        }
                    }
                    else
                    {
                        // None really matched, error in nnc data. 
                    }
                }
            }

            // clang-format on

            selItem = new RiuEclipseSelectionItem( associatedGridView,
                                                   eclResDef,
                                                   timestepIndex,
                                                   gridIndex,
                                                   gridLocalCellIndex,
                                                   nncIndex,
                                                   curveColor,
                                                   face,
                                                   localIntersectionPoint );
        }

        if ( geomResDef )
        {
            if ( intersectionHit )
                selItem = new RiuGeoMechSelectionItem( associatedGridView,
                                                       geomResDef,
                                                       timestepIndex,
                                                       gridIndex,
                                                       gridLocalCellIndex,
                                                       curveColor,
                                                       gmFace,
                                                       localIntersectionPoint,
                                                       intersectionTriangleHit );
            else
                selItem = new RiuGeoMechSelectionItem( associatedGridView,
                                                       geomResDef,
                                                       timestepIndex,
                                                       gridIndex,
                                                       gridLocalCellIndex,
                                                       curveColor,
                                                       gmFace,
                                                       localIntersectionPoint );
        }

        if ( intersectionView ) selItem = new Riu2dIntersectionSelectionItem( intersectionView, selItem );
    }

    if ( appendToSelection )
    {
        Riu3dSelectionManager::instance()->appendItemToSelection( selItem );
    }
    else if ( selItem )
    {
        Riu3dSelectionManager::instance()->setSelectedItem( selItem );
    }

    return false;
}
