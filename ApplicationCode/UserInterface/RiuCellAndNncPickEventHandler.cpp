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
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGeoMechView.h"

#include "Riu3dSelectionManager.h"
#include "RiuViewerCommands.h"

#include "RivBoxIntersectionSourceInfo.h"
#include "RivExtrudedCurveIntersectionSourceInfo.h"
#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivSourceInfo.h"

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
    cvf::Vec3d globalIntersectionPoint( cvf::Vec3d::ZERO );
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
                localIntersectionPoint  = pickItemInfos[indexToFirstNoneNncItem].localPickedPoint();
                globalIntersectionPoint = pickItemInfos[indexToFirstNoneNncItem].globalPickedPoint();
                firstHitPart            = pickItemInfos[indexToFirstNoneNncItem].pickedPart();
                firstPartTriangleIndex  = pickItemInfos[indexToFirstNoneNncItem].faceIdx();
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
    }

    if ( !firstHitPart ) return false;

    size_t                             gridIndex       = cvf::UNDEFINED_SIZE_T;
    size_t                             cellIndex       = cvf::UNDEFINED_SIZE_T;
    cvf::StructGridInterface::FaceType face            = cvf::StructGridInterface::NO_FACE;
    int                                gmFace          = -1;
    bool                               intersectionHit = false;
    std::array<cvf::Vec3f, 3>          intersectionTriangleHit;

    // clang-format off
        if ( const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>( firstHitPart->sourceInfo() ) )
        {
            gridIndex = rivSourceInfo->gridIndex();
            if ( rivSourceInfo->hasCellFaceMapping() )
            {
                CVF_ASSERT( rivSourceInfo->m_cellFaceFromTriangleMapper.notNull() );

                cellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex( firstPartTriangleIndex );
                face      = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace( firstPartTriangleIndex );
            }
        }
        else if ( const RivFemPickSourceInfo* femSourceInfo = 
                 dynamic_cast<const RivFemPickSourceInfo*>( firstHitPart->sourceInfo() ) )
        {
            gridIndex = femSourceInfo->femPartIndex();
            cellIndex = femSourceInfo->triangleToElmMapper()->elementIndex( firstPartTriangleIndex );
            gmFace    = femSourceInfo->triangleToElmMapper()->elementFace( firstPartTriangleIndex );
        }
        else if ( const RivExtrudedCurveIntersectionSourceInfo* crossSectionSourceInfo =
                 dynamic_cast<const RivExtrudedCurveIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
        {
            RiuViewerCommands::findCellAndGridIndex( mainOrComparisonView, 
                                 crossSectionSourceInfo, 
                                 firstPartTriangleIndex, 
                                 &cellIndex, 
                                 &gridIndex );
            intersectionHit         = true;
            intersectionTriangleHit = crossSectionSourceInfo->triangle( firstPartTriangleIndex );
        }
        else if ( const RivBoxIntersectionSourceInfo* intersectionBoxSourceInfo =
                 dynamic_cast<const RivBoxIntersectionSourceInfo*>( firstHitPart->sourceInfo() ) )
        {
            RiuViewerCommands::findCellAndGridIndex( mainOrComparisonView,
                                 intersectionBoxSourceInfo,
                                 firstPartTriangleIndex,
                                 &cellIndex,
                                 &gridIndex );
            intersectionHit         = true;
            intersectionTriangleHit = intersectionBoxSourceInfo->triangle( firstPartTriangleIndex );
        }
    // clang-format on

    if ( cellIndex == cvf::UNDEFINED_SIZE_T )
    {
        Riu3dSelectionManager::instance()->deleteAllItems();
        return false;
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
        RimEclipseView*        eclipseView      = dynamic_cast<RimEclipseView*>( mainOrComparisonView );
        RimGeoMechView*        geomView         = dynamic_cast<RimGeoMechView*>( mainOrComparisonView );

        if ( intersectionView )
        {
            intersectionView->intersection()->firstAncestorOrThisOfType( eclipseView );
            intersectionView->intersection()->firstAncestorOrThisOfType( geomView );
        }

        if ( eclipseView )
        {
            selItem = new RiuEclipseSelectionItem( eclipseView,
                                                   nullptr,
                                                   -1,
                                                   gridIndex,
                                                   cellIndex,
                                                   nncIndex,
                                                   curveColor,
                                                   face,
                                                   localIntersectionPoint );
        }

        if ( geomView )
        {
            if ( intersectionHit )
                selItem = new RiuGeoMechSelectionItem( geomView,
                                                       nullptr,
                                                       -1,
                                                       gridIndex,
                                                       cellIndex,
                                                       curveColor,
                                                       gmFace,
                                                       localIntersectionPoint,
                                                       intersectionTriangleHit );
            else
                selItem = new RiuGeoMechSelectionItem( geomView,
                                                       nullptr,
                                                       -1,
                                                       gridIndex,
                                                       cellIndex,
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
