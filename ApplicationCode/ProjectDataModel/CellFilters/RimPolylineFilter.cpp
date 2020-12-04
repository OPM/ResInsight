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

#include "RimPolylineFilter.h"

#include "RigCellGeometryTools.h"
#include "RigMainGrid.h"
#include "RigPolyLinesData.h"
#include "Rim3dView.h"
#include "RimCellFilterCollection.h"
#include "RimPolylineTarget.h"
#include "RiuViewerCommands.h"
#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfBoundingBox.h"
#include "cvfStructGrid.h"

namespace caf
{
template <>
void caf::AppEnum<RimPolylineFilter::PolylineFilterModeType>::setUp()
{
    addItem( RimPolylineFilter::DEPTH_Z, "DEPTH_Z", "Depth" );
    addItem( RimPolylineFilter::INDEX_K, "INDEX_K", "K index" );
    setDefault( RimPolylineFilter::DEPTH_Z );
}

template <>
void caf::AppEnum<RimPolylineFilter::PolylineIncludeType>::setUp()
{
    addItem( RimPolylineFilter::FULL_CELL, "FULL_CELL", "Whole cell inside" );
    addItem( RimPolylineFilter::CENTER, "CENTER", "Cell center inside" );
    addItem( RimPolylineFilter::ANY, "ANY", "Any part inside" );
    setDefault( RimPolylineFilter::CENTER );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimPolylineFilter, "PolyLineFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineFilter::RimPolylineFilter()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Polyline Filter", ":/CellFilter_Polyline.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_polyFilterMode, "PolylineFilterType", "Depth Type", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_polyIncludeType, "PolyIncludeType", "Cells to include", "", "", "" );

    CAF_PDM_InitField( &m_showPolygon, "ShowPolygon", true, "Show Polygon", "", "", "" );

    CAF_PDM_InitField( &m_enablePicking, "EnablePicking", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_enablePicking );
    m_enablePicking.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets", "", "", "" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    // m_targets.uiCapability()->setUiTreeHidden(true);
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( true );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );
    this->uiCapability()->setUiTreeChildrenHidden( true );

    updateIconState();
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineFilter::~RimPolylineFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::updateVisualization()
{
    Rim3dView* view;
    this->firstAncestorOrThisOfType( view );

    if ( view ) view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::updateEditorsAndVisualization()
{
    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimPolylineFilter::activeTargets() const
{
    return m_targets.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert )
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
void RimPolylineFilter::defineEditorAttribute( const caf::PdmFieldHandle* field,
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
void RimPolylineFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCellFilter::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_polyFilterMode );
    uiOrdering.add( &m_polyIncludeType );
    uiOrdering.add( &m_targets );
    uiOrdering.add( &m_enablePicking );
    uiOrdering.add( &m_showPolygon );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( changedField == &m_enablePicking )
    {
        this->updateConnectedEditors();

        if ( !m_enablePicking() )
        {
            updateCells();
            filterChanged.send();
        }
    }
    else if ( ( changedField == &m_polyFilterMode ) || ( changedField == &m_polyIncludeType ) )
    {
        updateCells();
        filterChanged.send();
    }

    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::enablePicking( bool enable )
{
    m_enablePicking = enable;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylineFilter::pickingEnabled() const
{
    return m_enablePicking();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* RimPolylineFilter::pickEventHandler() const
{
    return m_pickTargetsEventHandler.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter ) const
{
    CVF_ASSERT( cellRangeFilter );

    const auto grid = selectedGrid();
    size_t     i, j, k;

    for ( size_t cellidx : m_cells )
    {
        grid->ijkFromCellIndex( cellidx, &i, &j, &k );
        if ( this->filterMode() == RimCellFilter::INCLUDE )
        {
            cellRangeFilter->addCellInclude( i - 1, j - 1, k - 1, propagateToSubGrids() );
        }
        else
        {
            cellRangeFilter->addCellExclude( i - 1, j - 1, k - 1, propagateToSubGrids() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolylineFilter::cellInsidePolygon( cvf::Vec3d                 center,
                                           std::array<cvf::Vec3d, 8>& corners,
                                           std::vector<cvf::Vec3d>    polygon )
{
    bool bInside = false;
    switch ( m_polyIncludeType() )
    {
        case PolylineIncludeType::ANY:
            // any part of the cell is inside
            for ( const auto& point : corners )
            {
                if ( RigCellGeometryTools::pointInsidePolygon2D( point, polygon ) ) return true;
            }
            break;

        case PolylineIncludeType::CENTER:
            // cell center is inside
            return RigCellGeometryTools::pointInsidePolygon2D( center, polygon );

        case PolylineIncludeType::FULL_CELL:
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

//--------------------------------------------------------------------------------------------------
///  Update the cell index list with the cells that are inside our polygon, if any
//--------------------------------------------------------------------------------------------------
void RimPolylineFilter::updateCells()
{
    // reset
    m_cells.clear();

    // need at least 3 points
    if ( m_targets.size() < 3 ) return;

    // get polyline as vector
    size_t                  count = m_targets.size() + 1;
    std::vector<cvf::Vec3d> points( count );
    int                     i = 0;
    for ( auto& target : m_targets )
    {
        points[i++] = target->targetPointXYZ();
    }
    // make sure first and last point is the same (req. by polygon methods later)
    points[i] = m_targets[0]->targetPointXYZ();

    // TODO : get our grid - need to use the case, then find the grid here!
    RigMainGrid* grid = nullptr;
    this->firstAncestorOrThisOfType( grid );
    if ( !grid ) return;

    if ( m_polyFilterMode == PolylineFilterModeType::DEPTH_Z )
    {
        // we should look in depth using Z coordinate
        // loop over all cells
        for ( size_t i = 0; i < grid->cellCount(); i++ )
        {
            // valid cell?
            RigCell cell = grid->cellByGridAndGridLocalCellIdx( gridIndex(), i );
            if ( !cell.isInvalid() ) continue;

            // get corner coordinates
            std::array<cvf::Vec3d, 8> hexCorners;
            grid->cellCornerVertices( i, hexCorners.data() );

            // check if the polygon includes the cell
            if ( cellInsidePolygon( cell.center(), hexCorners, points ) )
            {
                m_cells.push_back( i );
            }
        }
    }
    else if ( m_polyFilterMode == PolylineFilterModeType::INDEX_K )
    {
        // we should look in depth using the K coordinate
        // not yet supported
    }
}
