/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimPolygonFilter.h"

#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimPolylineTarget.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RiuViewerCommands.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfStructGrid.h"

namespace caf
{
template <>
void caf::AppEnum<RimPolygonFilter::PolygonFilterModeType>::setUp()
{
    addItem( RimPolygonFilter::PolygonFilterModeType::DEPTH_Z, "DEPTH_Z", "XY Position" );
    addItem( RimPolygonFilter::PolygonFilterModeType::INDEX_K, "INDEX_K", "IJ Index" );
    setDefault( RimPolygonFilter::PolygonFilterModeType::INDEX_K );
}

template <>
void caf::AppEnum<RimPolygonFilter::PolygonIncludeType>::setUp()
{
    addItem( RimPolygonFilter::PolygonIncludeType::FULL_CELL, "FULL_CELL", "Whole cell inside polygon" );
    addItem( RimPolygonFilter::PolygonIncludeType::CENTER, "CENTER", "Cell center inside polygon" );
    addItem( RimPolygonFilter::PolygonIncludeType::ANY, "ANY", "Any corner inside polygon" );
    setDefault( RimPolygonFilter::PolygonIncludeType::CENTER );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimPolygonFilter, "PolygonFilter", "PolyLineFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFilter::RimPolygonFilter()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Polyline Filter", ":/CellFilter_Polygon.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_polyFilterMode, "PolygonFilterType", "Vertical Filter", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_polyIncludeType, "PolyIncludeType", "Cells to include", "", "", "" );

    CAF_PDM_InitField( &m_enablePicking, "EnablePicking", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_enablePicking );
    m_enablePicking.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets", "", "", "" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitFieldNoDefault( &m_srcCase, "Case", "Case", "", "", "" );
    m_srcCase.uiCapability()->setUiHidden( true );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );
    this->uiCapability()->setUiTreeChildrenHidden( true );

    updateIconState();
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFilter::~RimPolygonFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateVisualization()
{
    updateCells();
    filterChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::initAfterRead()
{
    resolveReferencesRecursively();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateEditorsAndVisualization()
{
    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::setCase( RimCase* srcCase )
{
    m_srcCase = srcCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonFilter::fullName() const
{
    return QString( "%1  [%2 cells]" ).arg( RimCellFilter::fullName(), QString::number( m_cells.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimPolygonFilter::activeTargets() const
{
    return m_targets.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert )
{
    size_t index = m_targets.index( targetToInsertBefore );
    if ( index < m_targets.size() )
        m_targets.insert( index, targetToInsert );
    else
        m_targets.push_back( targetToInsert );

    updateCells();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::deleteTarget( RimPolylineTarget* targetToDelete )
{
    m_targets.removeChildObject( targetToDelete );
    delete targetToDelete;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                              QString                    uiConfigName,
                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_enablePicking )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            if ( !m_enablePicking )
            {
                pbAttribute->m_buttonText = "Start Picking Points";
            }
            else
            {
                pbAttribute->m_buttonText = "Stop Picking Points";
            }
        }
    }

    if ( field == &m_targets )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;

            if ( m_enablePicking )
            {
                tvAttribute->baseColor.setRgb( 255, 220, 255 );
                tvAttribute->alwaysEnforceResizePolicy = true;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                QMenu*                     menu,
                                                QWidget*                   fieldEditorWidget )
{
    caf::CmdFeatureMenuBuilder menuBuilder;

    menuBuilder << "RicDeletePolylineTargetFeature";

    menuBuilder.appendToMenu( menu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCellFilter::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_polyFilterMode );
    uiOrdering.add( &m_polyIncludeType );
    uiOrdering.add( &m_targets );
    uiOrdering.add( &m_enablePicking );

    m_polyIncludeType.uiCapability()->setUiName( "Cells to " + modeString() );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    if ( changedField == &m_enablePicking )
    {
        this->updateConnectedEditors();

        if ( m_enablePicking() )
        {
            setActive( false );
            filterChanged.send();
        }
    }
    else if ( changedField != &m_name )
    {
        updateCells();
        filterChanged.send();
        this->updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::enablePicking( bool enable )
{
    m_enablePicking = enable;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonFilter::pickingEnabled() const
{
    return m_enablePicking();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* RimPolygonFilter::pickEventHandler() const
{
    return m_pickTargetsEventHandler.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter )
{
    CVF_ASSERT( cellRangeFilter );

    if ( m_cells.size() == 0 ) updateCells();

    const auto grid = selectedGrid();
    size_t     i, j, k;

    for ( size_t cellidx : m_cells )
    {
        grid->ijkFromCellIndex( cellidx, &i, &j, &k );
        if ( this->filterMode() == RimCellFilter::INCLUDE )
        {
            cellRangeFilter->addCellInclude( i, j, k, propagateToSubGrids() );
        }
        else
        {
            cellRangeFilter->addCellExclude( i, j, k, propagateToSubGrids() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonFilter::cellInsidePolygon2D( cvf::Vec3d                 center,
                                            std::array<cvf::Vec3d, 8>& corners,
                                            std::vector<cvf::Vec3d>    polygon )
{
    bool bInside = false;
    switch ( m_polyIncludeType() )
    {
        case PolygonIncludeType::ANY:
            // any part of the cell is inside
            for ( const auto& point : corners )
            {
                if ( RigCellGeometryTools::pointInsidePolygon2D( point, polygon ) ) return true;
            }
            break;

        case PolygonIncludeType::CENTER:
            // cell center is inside
            return RigCellGeometryTools::pointInsidePolygon2D( center, polygon );

        case PolygonIncludeType::FULL_CELL:
            // entire cell is inside
            bInside = true;
            for ( const auto& point : corners )
            {
                bInside = bInside && RigCellGeometryTools::pointInsidePolygon2D( point, polygon );
            }
            break;

        default:
            break;
    }

    return bInside;
}

void RimPolygonFilter::updateCellsDepthEclipse( const std::vector<cvf::Vec3d>& points, const RigMainGrid* grid )
{
    // we should look in depth using Z coordinate
    // loop over all cells
    for ( size_t i = 0; i < grid->cellCount(); i++ )
    {
        // valid cell?
        RigCell cell = grid->cellByGridAndGridLocalCellIdx( gridIndex(), i );
        if ( cell.isInvalid() ) continue;

        // get corner coordinates
        std::array<cvf::Vec3d, 8> hexCorners;
        grid->cellCornerVertices( i, hexCorners.data() );

        // check if the polygon includes the cell
        if ( cellInsidePolygon2D( cell.center(), hexCorners, points ) )
        {
            m_cells.push_back( i );
        }
    }
}

//--------------------------------------------------------------------------------------------------
// we should look in depth using the K coordinate
// 1. find the K layer of the first point
// 2. find all cells in this K layer that matches the selection criteria
// 3. extend those cells to all K layers
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellsKIndexEclipse( const std::vector<cvf::Vec3d>& points, const RigMainGrid* grid )
{
    // we need to find the K layer we hit with the first point
    size_t ni, nj, nk;
    // move the point a bit downwards to make sure it is inside something
    cvf::Vec3d point = points[0];
    point.z() += 0.2;

    bool cellFound = false;

    // loop over all points to find at least one point with a valid K layer
    for ( size_t p = 0; p < points.size() - 1; p++ )
    {
        // loop over all cells to find the correct one
        for ( size_t i = 0; i < grid->cellCount(); i++ )
        {
            // valid cell?
            RigCell cell = grid->cellByGridAndGridLocalCellIdx( gridIndex(), i );
            if ( cell.isInvalid() ) continue;

            // is the point inside?
            cvf::BoundingBox bb = cell.boundingBox();
            if ( !bb.contains( points[p] ) ) continue;

            // found the cell, get the IJK
            grid->ijkFromCellIndexUnguarded( cell.mainGridCellIndex(), &ni, &nj, &nk );
            cellFound = true;
            break;
        }
        if ( cellFound ) break;
    }

    // should not really happen, but just to be sure
    if ( !cellFound ) return;

    std::list<size_t> foundCells;

    // find all cells in this K layer that matches the polygon
    for ( size_t i = 0; i < grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            size_t  cellIdx = grid->cellIndexFromIJK( i, j, nk );
            RigCell cell    = grid->cellByGridAndGridLocalCellIdx( gridIndex(), cellIdx );
            // valid cell?
            if ( cell.isInvalid() ) continue;

            // get corner coordinates
            std::array<cvf::Vec3d, 8> hexCorners;
            grid->cellCornerVertices( cellIdx, hexCorners.data() );

            // check if the polygon includes the cell
            if ( cellInsidePolygon2D( cell.center(), hexCorners, points ) )
            {
                foundCells.push_back( cellIdx );
            }
        }
    }

    // now extend all these cells in one K layer to all K layers
    for ( const size_t cellIdx : foundCells )
    {
        size_t ci, cj, ck;
        grid->ijkFromCellIndexUnguarded( cellIdx, &ci, &cj, &ck );

        for ( size_t k = 0; k < grid->cellCountK(); k++ )
        {
            // get the cell index
            size_t newIdx = grid->cellIndexFromIJK( ci, cj, k );
            // valid cell?
            RigCell cell = grid->cellByGridAndGridLocalCellIdx( gridIndex(), newIdx );
            if ( cell.isInvalid() ) continue;

            m_cells.push_back( newIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellsForEclipse( const std::vector<cvf::Vec3d>& points, RimEclipseCase* eCase )
{
    RigEclipseCaseData* data = eCase->eclipseCaseData();
    if ( !data ) return;

    RigMainGrid* grid = data->mainGrid();
    if ( !grid ) return;

    if ( m_polyFilterMode == PolygonFilterModeType::DEPTH_Z )
    {
        updateCellsDepthEclipse( points, grid );
    }
    else if ( m_polyFilterMode == PolygonFilterModeType::INDEX_K )
    {
        updateCellsKIndexEclipse( points, grid );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellsDepthGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid )
{
    // we should look in depth using Z coordinate
    // loop over all cells
    for ( size_t i = 0; i < grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            for ( size_t k = 0; k < grid->cellCountK(); k++ )
            {
                size_t cellIdx = grid->cellIndexFromIJK( i, j, k );

                cvf::Vec3d vertices[8];
                grid->cellCornerVertices( cellIdx, vertices );
                cvf::Vec3d center = grid->cellCentroid( cellIdx );

                std::array<cvf::Vec3d, 8> corners;
                for ( size_t n = 0; n < 8; n++ )
                    corners[n] = vertices[n];

                // check if the polygon includes the cell
                if ( cellInsidePolygon2D( center, corners, points ) )
                {
                    m_cells.push_back( cellIdx );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
// we should look in depth using the K coordinate
// 1. find the K layer of the first point
// 2. find all cells in this K layer that matches the selection criteria
// 3. extend those cells to all K layers
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellsKIndexGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid )
{
    // we need to find the K layer we hit with the first point
    size_t nk;
    // move the point a bit downwards to make sure it is inside something
    cvf::Vec3d point = points[0];
    point.z() += 0.2;

    bool cellFound = false;
    // loop over all cells to find the correct one
    for ( size_t i = 0; i < grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            for ( size_t k = 0; k < grid->cellCountK(); k++ )
            {
                // get cell bounding box
                size_t           cellIdx = grid->cellIndexFromIJK( i, j, k );
                cvf::BoundingBox bb;
                cvf::Vec3d       vertices[8];
                grid->cellCornerVertices( cellIdx, vertices );
                for ( const auto& point : vertices )
                    bb.add( point );

                // check all points for a bb match
                for ( size_t p = 0; p < points.size() - 1; p++ )
                {
                    // is the point inside?
                    if ( bb.contains( points[p] ) )
                    {
                        cellFound = true;
                        // found the cell, store the K
                        nk = k;
                        break;
                    }
                }

                if ( cellFound ) break;
            }
            if ( cellFound ) break;
        }
        if ( cellFound ) break;
    }

    // should not really happen, but just to be sure
    if ( !cellFound ) return;

    // find all cells in this K layer that matches the polygon
    std::list<size_t> foundCells;
    for ( size_t i = 0; i < grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            size_t cellIdx = grid->cellIndexFromIJK( i, j, nk );

            // get corner coordinates
            std::array<cvf::Vec3d, 8> hexCorners;
            grid->cellCornerVertices( cellIdx, hexCorners.data() );
            cvf::Vec3d center = grid->cellCentroid( cellIdx );

            // check if the polygon includes the cell
            if ( cellInsidePolygon2D( center, hexCorners, points ) )
            {
                foundCells.push_back( cellIdx );
            }
        }
    }

    // now extend all these cells in one K layer to all K layers
    for ( const size_t cellIdx : foundCells )
    {
        size_t ci, cj, ck;
        grid->ijkFromCellIndex( cellIdx, &ci, &cj, &ck );

        for ( size_t k = 0; k < grid->cellCountK(); k++ )
        {
            // get the cell index
            size_t newIdx = grid->cellIndexFromIJK( ci, cj, k );
            m_cells.push_back( newIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellsForGeoMech( const std::vector<cvf::Vec3d>& points, RimGeoMechCase* gCase )
{
    if ( gCase->geoMechData() && gCase->geoMechData()->femParts()->partCount() > 0 )
    {
        const RigFemPartGrid* grid = gCase->geoMechData()->femParts()->part( 0 )->getOrCreateStructGrid();

        if ( m_polyFilterMode == PolygonFilterModeType::DEPTH_Z )
        {
            updateCellsDepthGeoMech( points, grid );
        }
        else if ( m_polyFilterMode == PolygonFilterModeType::INDEX_K )
        {
            updateCellsKIndexGeoMech( points, grid );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///  Update the cell index list with the cells that are inside our polygon, if any
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCells()
{
    // reset
    m_cells.clear();

    // get polyline as vector
    std::vector<cvf::Vec3d> points;
    for ( auto& target : m_targets )
    {
        if ( target->isEnabled() ) points.push_back( target->targetPointXYZ() );
    }

    // We need at least three points to make a sensible polygon
    if ( points.size() < 3 ) return;

    // make sure first and last point is the same (req. by polygon methods later)
    points.push_back( points.front() );

    RimEclipseCase* eCase = dynamic_cast<RimEclipseCase*>( m_srcCase() );
    RimGeoMechCase* gCase = dynamic_cast<RimGeoMechCase*>( m_srcCase() );

    if ( eCase )
    {
        updateCellsForEclipse( points, eCase );
    }
    else if ( gCase )
    {
        updateCellsForGeoMech( points, gCase );
    }
}
