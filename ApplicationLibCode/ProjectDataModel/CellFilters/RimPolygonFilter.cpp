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
#include "RigPolyLinesData.h"
#include "RigReservoirGridTools.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimPolylineTarget.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RiuViewerCommands.h"

#include "RiaStdStringTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include <cafPdmUiDoubleSliderEditor.h>

#include "cvfBoundingBox.h"
#include "cvfStructGrid.h"

#include <QValidator>

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class ThicknessValidator : public QValidator
{
public:
    State validate( QString& input, int& pos ) const override
    {
        if ( input.isEmpty() ) return State::Intermediate;

        int val = RiaStdStringTools::toInt( input.toStdString() );
        if ( val > 0 && val < 8 )
            return State::Acceptable;
        else
            return State::Invalid;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RadiusValidator : public QValidator
{
public:
    State validate( QString& input, int& pos ) const override
    {
        if ( input.isEmpty() ) return State::Intermediate;

        double val = RiaStdStringTools::toDouble( input.toStdString() );
        if ( val > 0.001 && val <= 2.0 )
            return State::Acceptable;
        else
            return State::Invalid;
    }
};

CAF_PDM_SOURCE_INIT( RimPolygonFilter, "PolygonFilter", "PolyLineFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFilter::RimPolygonFilter()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
    , m_intervalTool( true )
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

    CAF_PDM_InitField( &m_showLines, "ShowLines", true, "Show Lines", "", "", "" );
    CAF_PDM_InitField( &m_showSpheres, "ShowSpheres", false, "Show Spheres", "", "", "" );

    CAF_PDM_InitField( &m_lineThickness, "LineThickness", 3, "Line Thickness", "", "", "" );
    CAF_PDM_InitField( &m_sphereRadiusFactor, "SphereRadiusFactor", 0.15, "Sphere Radius Factor", "", "", "" );

    CAF_PDM_InitField( &m_lineColor, "LineColor", cvf::Color3f( cvf::Color3f::WHITE ), "Line Color", "", "", "" );
    CAF_PDM_InitField( &m_sphereColor, "SphereColor", cvf::Color3f( cvf::Color3f::WHITE ), "Sphere Color", "", "", "" );

    CAF_PDM_InitField( &m_enableFiltering, "EnableFiltering", false, "Enable Filter", "", "", "" );

    CAF_PDM_InitField( &m_enableKFilter, "EnableKFilter", false, "Enable K Range Filter", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_kFilterStr, "KRangeFilter", "K Range Filter", "", "Example: 2,4,10-20,31", "" );

    CAF_PDM_InitField( &m_polygonPlaneDepth, "PolygonPlaneDepth", 0.0, "Polygon Plane Depth", "", "", "" );
    CAF_PDM_InitField( &m_lockPolygonToPlane, "LockPolygon", false, "Lock Polygon to Plane", "", "", "" );

    m_polygonPlaneDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_polygonPlaneDepth.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::TOP );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );
    this->uiCapability()->setUiTreeChildrenHidden( true );

    m_propagateToSubGrids = false;

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
void RimPolygonFilter::enableFilter( bool bEnable )
{
    m_enableFiltering = bEnable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::enableKFilter( bool bEnable )
{
    m_enableKFilter = bEnable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonFilter::isFilterEnabled() const
{
    return m_isActive() && m_enableFiltering;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonFilter::fullName() const
{
    if ( m_enableFiltering )
    {
        int cells = 0;
        for ( const auto& item : m_cells )
        {
            cells += (int)item.size();
        }
        return QString( "%1  [%2 cells]" ).arg( RimCellFilter::fullName(), QString::number( cells ) );
    }
    return QString( "%1  [off]" ).arg( RimCellFilter::fullName() );
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
    else if ( field == &m_targets )
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
    else if ( field == &m_lineThickness )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->validator = new ThicknessValidator();
        }
    }
    else if ( field == &m_lineThickness )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->validator = new RadiusValidator();
        }
    }
    else if ( field == &m_polygonPlaneDepth )
    {
        auto* attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( attr )
        {
            if ( m_srcCase )
            {
                auto bb         = m_srcCase->allCellsBoundingBox();
                attr->m_minimum = -bb.max().z();
                attr->m_maximum = -bb.min().z();
            }
            else
            {
                attr->m_minimum = 0;
                attr->m_maximum = 10000;
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
    uiOrdering.add( &m_name );

    auto group = uiOrdering.addNewGroup( "General" );
    group->add( &m_filterMode );
    group->add( &m_enableFiltering );
    group->add( &m_showLines );
    group->add( &m_showSpheres );

    auto group1 = uiOrdering.addNewGroup( "Polygon Selection" );
    group1->add( &m_polyFilterMode );
    group1->add( &m_polyIncludeType );
    group1->add( &m_targets );
    group1->add( &m_enablePicking );

    m_polyIncludeType.uiCapability()->setUiName( "Cells to " + modeString() );

    auto group2 = uiOrdering.addNewGroup( "Appearance" );
    if ( m_showLines )
    {
        group2->add( &m_lineThickness );
        group2->add( &m_lineColor );
    }
    if ( m_showSpheres )
    {
        group2->add( &m_sphereRadiusFactor );
        group2->add( &m_sphereColor );
    }
    group2->add( &m_lockPolygonToPlane );
    if ( m_lockPolygonToPlane ) group2->add( &m_polygonPlaneDepth );
    group2->setCollapsedByDefault( true );

    auto group3 = uiOrdering.addNewGroup( "Advanced Filter Settings" );
    group3->add( &m_enableKFilter );
    group3->add( &m_kFilterStr );
    group3->setCollapsedByDefault( true );

    uiOrdering.skipRemainingFields( true );

    bool readOnlyState = isFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields;
    this->fields( objFields );
    for ( auto& objField : objFields )
    {
        objField->uiCapability()->setUiReadOnly( readOnlyState );
    }
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
            enableFilter( false );
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
void RimPolygonFilter::updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter, int gridIndex )
{
    CVF_ASSERT( cellRangeFilter );

    if ( !m_enableFiltering ) return;

    const int noofgrids = static_cast<int>( m_cells.size() );
    if ( ( noofgrids == 0 ) || ( gridIndex >= noofgrids ) )
    {
        updateCells();
    }

    if ( gridIndex >= static_cast<int>( m_cells.size() ) ) return;

    const auto grid = RigReservoirGridTools::gridByIndex( m_srcCase, gridIndex );
    size_t     i, j, k;

    for ( size_t cellidx : m_cells[gridIndex] )
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

void RimPolygonFilter::updateCellsDepthEclipse( const std::vector<cvf::Vec3d>& points, const RigGridBase* grid )
{
    // we should look in depth using Z coordinate
    const int gIdx = static_cast<int>( grid->gridIndex() );
    // loop over all cells
    for ( size_t n = 0; n < grid->cellCount(); n++ )
    {
        // valid cell?
        RigCell cell = grid->cell( n );
        if ( cell.isInvalid() ) continue;

        // get corner coordinates
        std::array<cvf::Vec3d, 8> hexCorners;
        grid->cellCornerVertices( n, hexCorners.data() );

        // get cell ijk for k filter
        size_t i, j, k;
        grid->ijkFromCellIndex( n, &i, &j, &k );
        if ( !m_intervalTool.isNumberIncluded( k ) ) continue;

        // check if the polygon includes the cell
        if ( cellInsidePolygon2D( cell.center(), hexCorners, points ) )
        {
            m_cells[gIdx].push_back( n );
        }
    }
}

//--------------------------------------------------------------------------------------------------
// we should look in depth using the K coordinate
// 1. find the K layer of the first point
// 2. find all cells in this K layer that matches the selection criteria
// 3. extend those cells to all K layers
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellsKIndexEclipse( const std::vector<cvf::Vec3d>& points, const RigGridBase* grid, int K )
{
    const int gIdx = static_cast<int>( grid->gridIndex() );

    std::list<size_t> foundCells;

    // find all cells in the K layer that matches the polygon
    for ( size_t i = 0; i < grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            size_t  cellIdx = grid->cellIndexFromIJK( i, j, K );
            RigCell cell    = grid->cell( cellIdx );
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
            if ( !m_intervalTool.isNumberIncluded( k ) ) continue;

            // get the cell index
            size_t newIdx = grid->cellIndexFromIJK( ci, cj, k );
            // valid cell?
            RigCell cell = grid->cell( newIdx );
            if ( cell.isInvalid() ) continue;

            m_cells[gIdx].push_back( newIdx );
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

    if ( m_polyFilterMode == PolygonFilterModeType::DEPTH_Z )
    {
        for ( size_t gridIndex = 0; gridIndex < data->gridCount(); gridIndex++ )
        {
            auto grid = data->grid( gridIndex );
            updateCellsDepthEclipse( points, grid );
        }
    }
    else if ( m_polyFilterMode == PolygonFilterModeType::INDEX_K )
    {
        int K = findEclipseKLayer( points, data );
        if ( K < 0 ) return;

        for ( size_t gridIndex = 0; gridIndex < data->gridCount(); gridIndex++ )
        {
            auto grid = data->grid( gridIndex );
            updateCellsKIndexEclipse( points, grid, K );
        }
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
                if ( !m_intervalTool.isNumberIncluded( k ) ) continue;

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
                    m_cells[0].push_back( cellIdx );
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
            if ( !m_intervalTool.isNumberIncluded( k ) ) continue;

            // get the cell index
            size_t newIdx = grid->cellIndexFromIJK( ci, cj, k );
            m_cells[0].push_back( newIdx );
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
///  Update the cell index map with the cells that are inside our polygon, if any
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCells()
{
    // reset cell map for all grids
    initializeCellList();

    // get optional k-cell filter
    m_intervalTool.setInterval( m_enableKFilter, m_kFilterStr );

    // get polyline as vector
    std::vector<cvf::Vec3d> points;
    for ( auto& target : m_targets )
    {
        if ( target->isEnabled() ) points.push_back( target->targetPointXYZ() );
    }

    // We need at least three points to make a sensible polygon
    if ( points.size() < 3 ) return;

    // make sure first and last point is the same (req. by polygon methods used later)
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimPolygonFilter::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;
    std::vector<cvf::Vec3d>    line;
    for ( const RimPolylineTarget* target : m_targets )
    {
        if ( target->isEnabled() ) line.push_back( target->targetPointXYZ() );
    }
    pld->setPolyLine( line );

    pld->setLineAppearance( m_lineThickness, m_lineColor, true );
    pld->setSphereAppearance( m_sphereRadiusFactor, m_sphereColor );
    pld->setZPlaneLock( m_lockPolygonToPlane, -m_polygonPlaneDepth );

    if ( isActive() )
    {
        pld->setVisibility( m_showLines, m_showSpheres );
    }
    else
    {
        pld->setVisibility( false, false );
    }

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::initializeCellList()
{
    m_cells.clear();

    int gridCount = RigReservoirGridTools::gridCount( m_srcCase() );
    for ( int i = 0; i < gridCount; i++ )
    {
        m_cells.push_back( std::vector<size_t>() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Find which K layer we hit, in any of the grids, for any of the selected points
//--------------------------------------------------------------------------------------------------
int RimPolygonFilter::findEclipseKLayer( const std::vector<cvf::Vec3d>& points, RigEclipseCaseData* data )
{
    size_t ni, nj, nk;

    // look for a hit in the main grid frist
    RigMainGrid* mainGrid = data->mainGrid();
    for ( size_t p = 0; p < points.size() - 1; p++ )
    {
        size_t cIdx = mainGrid->findReservoirCellIndexFromPoint( points[p] );
        if ( cIdx != cvf::UNDEFINED_SIZE_T )
        {
            mainGrid->ijkFromCellIndexUnguarded( cIdx, &ni, &nj, &nk );
            if ( mainGrid->isCellValid( ni, nj, nk ) ) return static_cast<int>( nk );
        }
    }

    // loop over all sub-grids to find one with a cell hit in case main grid search failed
    for ( size_t gridIndex = 1; gridIndex < data->gridCount(); gridIndex++ )
    {
        auto grid = data->grid( gridIndex );

        // loop over all cells to find the correct one
        for ( size_t i = 0; i < grid->cellCount(); i++ )
        {
            // valid cell?
            RigCell cell = grid->cell( i );
            if ( cell.isInvalid() ) continue;

            // is the point inside cell bb?
            cvf::BoundingBox          bb;
            std::array<cvf::Vec3d, 8> hexCorners;
            grid->cellCornerVertices( i, hexCorners.data() );
            for ( const auto& corner : hexCorners )
            {
                bb.add( corner );
            }

            // loop over all points to find at least one point with a valid K layer
            for ( size_t p = 0; p < points.size() - 1; p++ )
            {
                if ( bb.contains( points[p] ) )
                {
                    // found the cell, get the IJK
                    grid->ijkFromCellIndexUnguarded( i, &ni, &nj, &nk );
                    return static_cast<int>( nk );
                }
            }
        }
    }
    return -1;
}
