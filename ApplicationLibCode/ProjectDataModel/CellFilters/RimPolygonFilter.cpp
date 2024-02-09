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
#include "RigReservoirGridTools.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include <limits>

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
    : RimCellFilter( RimCellFilter::INDEX )
    , m_intervalTool( true )
{
    CAF_PDM_InitObject( "Polyline Filter", ":/CellFilter_Polygon.png" );

    CAF_PDM_InitFieldNoDefault( &m_polyFilterMode, "PolygonFilterType", "Vertical Filter" );

    CAF_PDM_InitFieldNoDefault( &m_polyIncludeType, "PolyIncludeType", "Cells to include" );

    CAF_PDM_InitFieldNoDefault( &m_internalPolygon, "InternalPolygon", "Polygon For Filter" );
    m_internalPolygon = new RimPolygon;
    m_internalPolygon->setName( "Polygon For Filter" );
    m_internalPolygon->uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_cellFilterPolygon, "Polygon", "Polygon" );
    m_cellFilterPolygon = m_internalPolygon;

    CAF_PDM_InitFieldNoDefault( &m_polygonEditor, "PolygonEditor", "Polygon Editor" );
    m_polygonEditor = new RimPolygonInView;

    CAF_PDM_InitField( &m_enableFiltering, "EnableFiltering", false, "Enable Filter" );

    CAF_PDM_InitField( &m_enableKFilter, "EnableKFilter", false, "Enable K Range Filter" );
    CAF_PDM_InitFieldNoDefault( &m_kFilterStr, "KRangeFilter", "K Range Filter", "", "Example: 2,4-6,10-20:2", "" );

    CAF_PDM_InitField( &m_editPolygonButton, "EditPolygonButton", false, "Edit" );
    m_editPolygonButton.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_editPolygonButton.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_propagateToSubGrids = false;

    updateIconState();
    setDeletable( true );
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
void RimPolygonFilter::initAfterRead()
{
    RimCellFilter::initAfterRead();

    configurePolygonEditor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_editPolygonButton )
    {
        if ( auto attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
        {
            attrib->m_buttonText = "Edit";
        }
    }
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

    uiOrdering.add( &m_cellFilterPolygon );
    if ( m_cellFilterPolygon() != m_internalPolygon() )
    {
        uiOrdering.add( &m_editPolygonButton, { .newRow = false } );
    }

    auto group1 = uiOrdering.addNewGroup( "Polygon Selection" );
    group1->add( &m_polyFilterMode );

    bool isPolygonClosed = m_cellFilterPolygon() ? m_cellFilterPolygon->isClosed() : false;
    if ( isPolygonClosed )
    {
        group1->add( &m_polyIncludeType );
    }

    m_polyIncludeType.uiCapability()->setUiName( "Cells to " + modeString() );

    auto group3 = uiOrdering.addNewGroup( "Advanced Filter Settings" );
    group3->add( &m_enableKFilter );
    group3->add( &m_kFilterStr );
    group3->setCollapsedByDefault();

    uiOrdering.skipRemainingFields( true );

    bool readOnlyState = isFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields = fields();
    for ( auto& objField : objFields )
    {
        objField->uiCapability()->setUiReadOnly( readOnlyState );
    }

    if ( !isPolygonClosed )
    {
        m_polyFilterMode = RimPolygonFilter::PolygonFilterModeType::INDEX_K;
        m_polyFilterMode.uiCapability()->setUiReadOnly( true );
    }
    else
    {
        m_polyFilterMode.uiCapability()->setUiReadOnly( readOnlyState );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    RimCellFilter::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    bool showPolygonEditor = ( m_cellFilterPolygon() == m_internalPolygon() );

    if ( showPolygonEditor )
    {
        uiTreeOrdering.add( m_polygonEditor() );
    }

    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolygonFilter::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_cellFilterPolygon )
    {
        RimTools::polygonOptionItems( &options );
        if ( !options.empty() )
        {
            options.push_front( caf::PdmOptionItemInfo( "Project Polygons", nullptr ) );
        }

        options.push_front(
            caf::PdmOptionItemInfo( m_internalPolygon()->name(), m_internalPolygon(), false, m_internalPolygon->uiIconProvider() ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_editPolygonButton )
    {
        if ( m_cellFilterPolygon() ) Riu3DMainWindowTools::selectAsCurrentItem( m_cellFilterPolygon() );

        m_editPolygonButton = false;

        return;
    }

    if ( changedField == &m_cellFilterPolygon )
    {
        configurePolygonEditor();
        updateAllRequiredEditors();
    }

    if ( changedField != &m_name )
    {
        updateCells();
        filterChanged.send();
        updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::enablePicking( bool enable )
{
    m_polygonEditor->enablePicking( enable );
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::onGridChanged()
{
    m_cells.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex )
{
    if ( !m_enableFiltering ) return;

    const int noofgrids = static_cast<int>( m_cells.size() );
    if ( ( noofgrids == 0 ) || ( gridIndex >= noofgrids ) )
    {
        updateCells();
    }

    if ( gridIndex >= static_cast<int>( m_cells.size() ) ) return;

    if ( m_filterMode == FilterModeType::INCLUDE )
    {
        for ( auto cellIdx : m_cells[gridIndex] )
        {
            ( *includeVisibility )[cellIdx] = true;
        }
    }
    else
    {
        for ( auto cellIdx : m_cells[gridIndex] )
        {
            ( *excludeVisibility )[cellIdx] = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonFilter::cellInsidePolygon2D( cvf::Vec3d center, std::array<cvf::Vec3d, 8>& corners, std::vector<cvf::Vec3d> polygon )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::updateCellsDepthEclipse( const std::vector<cvf::Vec3d>& points, const RigGridBase* grid )
{
    // we should look in depth using Z coordinate
    const int gIdx = static_cast<int>( grid->gridIndex() );
    // loop over all cells
#pragma omp parallel for
    for ( int n = 0; n < (int)grid->cellCount(); n++ )
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
#pragma omp critical
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
    const bool        closedPolygon = isPolygonClosed();

    // find all cells in the K layer that matches the polygon
#pragma omp parallel for
    for ( int i = 0; i < (int)grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            size_t         cellIdx = grid->cellIndexFromIJK( i, j, K );
            const RigCell& cell    = grid->cell( cellIdx );
            // valid cell?
            if ( cell.isInvalid() ) continue;

            // get corner coordinates
            std::array<cvf::Vec3d, 8> hexCorners;
            grid->cellCornerVertices( cellIdx, hexCorners.data() );

            if ( closedPolygon )
            {
                // check if the polygon includes the cell
                if ( cellInsidePolygon2D( cell.center(), hexCorners, points ) )
                {
#pragma omp critical
                    foundCells.push_back( cellIdx );
                }
            }
            else
            {
                // check if the polyline touches the top face of the cell
                if ( RigCellGeometryTools::polylineIntersectsCellNegK2D( points, hexCorners ) )
                {
#pragma omp critical
                    foundCells.push_back( cellIdx );
                }
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
            const RigCell& cell = grid->cell( newIdx );
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
        if ( !isPolygonClosed() ) return;

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
void RimPolygonFilter::updateCellsDepthGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid, int partId )
{
    // we should look in depth using Z coordinate
    // loop over all cells
#pragma omp parallel for
    for ( int i = 0; i < (int)grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            for ( size_t k = 0; k < grid->cellCountK(); k++ )
            {
                if ( !m_intervalTool.isNumberIncluded( k ) ) continue;

                size_t cellIdx = grid->cellIndexFromIJK( i, j, k );
                if ( cellIdx == cvf::UNDEFINED_SIZE_T ) continue;

                cvf::Vec3d vertices[8];
                grid->cellCornerVertices( cellIdx, vertices );
                cvf::Vec3d center = grid->cellCentroid( cellIdx );

                std::array<cvf::Vec3d, 8> corners;
                for ( size_t n = 0; n < 8; n++ )
                    corners[n] = vertices[n];

                // check if the polygon includes the cell
                if ( cellInsidePolygon2D( center, corners, points ) )
                {
#pragma omp critical
                    m_cells[partId].push_back( cellIdx );
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
void RimPolygonFilter::updateCellsKIndexGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid, int partId )
{
    const bool closedPolygon = isPolygonClosed();

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
                size_t cellIdx = grid->cellIndexFromIJK( i, j, k );
                if ( cellIdx == cvf::UNDEFINED_SIZE_T ) continue;

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

#pragma omp parallel for
    for ( int i = 0; i < (int)grid->cellCountI(); i++ )
    {
        for ( size_t j = 0; j < grid->cellCountJ(); j++ )
        {
            size_t cellIdx = grid->cellIndexFromIJK( i, j, nk );
            if ( cellIdx == cvf::UNDEFINED_SIZE_T ) continue;

            // get corner coordinates
            std::array<cvf::Vec3d, 8> hexCorners;
            grid->cellCornerVertices( cellIdx, hexCorners.data() );
            cvf::Vec3d center = grid->cellCentroid( cellIdx );

            if ( closedPolygon )
            {
                // check if the polygon includes the cell
                if ( cellInsidePolygon2D( center, hexCorners, points ) )
                {
#pragma omp critical
                    foundCells.push_back( cellIdx );
                }
            }
            else
            {
                // check if the polyline touches the top face of the cell
                if ( RigCellGeometryTools::polylineIntersectsCellNegK2D( points, hexCorners ) )
                {
#pragma omp critical
                    foundCells.push_back( cellIdx );
                }
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
            if ( cellIdx == cvf::UNDEFINED_SIZE_T ) continue;

            m_cells[partId].push_back( newIdx );
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
        int partCount = gCase->geoMechData()->femParts()->partCount();
        for ( int i = 0; i < partCount; i++ )
        {
            const RigFemPartGrid* grid = gCase->geoMechData()->femParts()->part( i )->getOrCreateStructGrid();

            if ( m_polyFilterMode == PolygonFilterModeType::DEPTH_Z )
            {
                if ( isPolygonClosed() )
                {
                    updateCellsDepthGeoMech( points, grid, i );
                }
            }
            else if ( m_polyFilterMode == PolygonFilterModeType::INDEX_K )
            {
                updateCellsKIndexGeoMech( points, grid, i );
            }
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
    m_intervalTool.setInterval( m_enableKFilter, m_kFilterStr().toStdString() );

    // get polyline as vector
    std::vector<cvf::Vec3d> points;

    for ( auto target : m_polygonEditor->activeTargets() )
    {
        points.push_back( target->targetPointXYZ() );
    }

    // We need at least three points to make a closed polygon, or just 2 for a polyline
    if ( ( !isPolygonClosed() && ( points.size() < 2 ) ) || ( isPolygonClosed() && ( points.size() < 3 ) ) ) return;

    // make sure first and last point is the same (req. by closed polygon methods used later)
    if ( isPolygonClosed() ) points.push_back( points.front() );

    RimEclipseCase* eCase = eclipseCase();
    RimGeoMechCase* gCase = geoMechCase();

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
void RimPolygonFilter::configurePolygonEditor()
{
    m_polygonEditor->setPolygon( m_cellFilterPolygon() );

    // Must connect the signals after polygon is assigned to the polygon editor
    // When assigning an object to a ptr field, all signals are disconnected
    connectObjectSignals( m_cellFilterPolygon() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::appendPartsToModel( cvf::ModelBasicList*              model,
                                           const caf::DisplayCoordTransform* scaleTransform,
                                           const cvf::BoundingBox&           boundingBox )
{
    return m_polygonEditor->appendPartsToModel( model, scaleTransform, boundingBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView* RimPolygonFilter::polygonEditor() const
{
    return m_polygonEditor();
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
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonFilter::isPolygonClosed() const
{
    if ( m_polygonEditor->polygon() ) return m_polygonEditor->polygon()->isClosed();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::connectObjectSignals( RimPolygon* polygon )
{
    if ( m_cellFilterPolygon() )
    {
        m_cellFilterPolygon()->objectChanged.disconnect( this );
    }

    if ( polygon )
    {
        m_cellFilterPolygon = polygon;

        polygon->objectChanged.connect( this, &RimPolygonFilter::onObjectChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFilter::onObjectChanged( const caf::SignalEmitter* emitter )
{
    updateCells();
    filterChanged.send();
    updateIconState();
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

    auto findKLayerBelowPoint = []( const cvf::Vec3d& point, RigMainGrid* mainGrid )
    {
        // Create a bounding box (ie a ray) from the point down to minimum of grid
        cvf::Vec3d lowestPoint( point.x(), point.y(), mainGrid->boundingBox().min().z() );

        cvf::BoundingBox rayBBox;
        rayBBox.add( point );
        rayBBox.add( lowestPoint );

        // Find the cells intersecting the ray
        std::vector<size_t> allCellIndices = mainGrid->findIntersectingCells( rayBBox );

        // Get the minimum K layer index
        int  minK    = std::numeric_limits<int>::max();
        bool anyHits = false;
        for ( size_t cIdx : allCellIndices )
        {
            if ( cIdx != cvf::UNDEFINED_SIZE_T )
            {
                size_t ni, nj, nk;
                mainGrid->ijkFromCellIndexUnguarded( cIdx, &ni, &nj, &nk );
                if ( mainGrid->isCellValid( ni, nj, nk ) )
                {
                    anyHits = true;
                    minK    = std::min( minK, static_cast<int>( nk ) );
                }
            }
        }

        return anyHits ? minK : -1;
    };

    // shoot a ray down from each point to try to find a valid hit there
    for ( size_t p = 0; p < points.size() - 1; p++ )
    {
        int k = findKLayerBelowPoint( points[p], data->mainGrid() );
        if ( k != -1 ) return k;
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
