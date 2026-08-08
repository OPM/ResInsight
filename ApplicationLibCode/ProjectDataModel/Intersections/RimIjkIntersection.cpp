/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimIjkIntersection.h"

#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimEclipseView.h"

#include "RivIjkIntersectionPartMgr.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <algorithm>

namespace caf
{
template <>
void AppEnum<RimIjkIntersection::GridAxis>::setUp()
{
    addItem( RimIjkIntersection::GridAxis::AXIS_I, "AXIS_I", "I" );
    addItem( RimIjkIntersection::GridAxis::AXIS_J, "AXIS_J", "J" );
    addItem( RimIjkIntersection::GridAxis::AXIS_K, "AXIS_K", "K" );
    setDefault( RimIjkIntersection::GridAxis::AXIS_I );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimIjkIntersection, "IjkIntersection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIjkIntersection::RimIjkIntersection()
{
    CAF_PDM_InitObject( "Intersection I/J/K", ":/IntersectionBox16x16.png" );

    CAF_PDM_InitField( &m_name, "UserDescription", QString( "Intersection I/J/K" ), "Name" );

    CAF_PDM_InitFieldNoDefault( &m_axis, "Axis", "Axis" );
    CAF_PDM_InitField( &m_useNegativeFace, "UseNegativeFace", false, "Use Back Pillar Face" );

    CAF_PDM_InitField( &m_fixedIndex, "FixedIndex", 1, "Index" );
    m_fixedIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_iMin, "IMin", 1, "Min" );
    CAF_PDM_InitField( &m_iMax, "IMax", 1, "Max" );
    CAF_PDM_InitField( &m_jMin, "JMin", 1, "Min" );
    CAF_PDM_InitField( &m_jMax, "JMax", 1, "Max" );
    CAF_PDM_InitField( &m_kMin, "KMin", 1, "Min" );
    CAF_PDM_InitField( &m_kMax, "KMax", 1, "Max" );

    for ( caf::PdmField<int>* field : { &m_iMin, &m_iMax, &m_jMin, &m_jMax, &m_kMin, &m_kMax } )
    {
        field->uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    }

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIjkIntersection::~RimIjkIntersection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIjkIntersection::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimIjkIntersection::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::setName( const QString& newName )
{
    m_name = newName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIjkIntersection::GridAxis RimIjkIntersection::axis() const
{
    return m_axis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIjkIntersection::useNegativeFace() const
{
    return m_useNegativeFace();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimIjkIntersection::fixedIndex() const
{
    return m_fixedIndex() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigBoundingBoxIjk<caf::VecIjk0> RimIjkIntersection::ijkRange() const
{
    // Guard against non-positive values from hand-edited project files before converting to unsigned
    auto zeroBased = []( int oneBasedValue ) { return static_cast<size_t>( std::max( 1, oneBasedValue ) - 1 ); };

    caf::VecIjk0 min( zeroBased( m_iMin() ), zeroBased( m_jMin() ), zeroBased( m_kMin() ) );
    caf::VecIjk0 max( zeroBased( m_iMax() ), zeroBased( m_jMax() ), zeroBased( m_kMax() ) );

    return RigBoundingBoxIjk<caf::VecIjk0>( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::setAxis( GridAxis axis )
{
    m_axis = axis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::setUseNegativeFace( bool useNegativeFace )
{
    m_useNegativeFace = useNegativeFace;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::setFixedIndex( int fixedIndex )
{
    m_fixedIndex = fixedIndex + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::setIjkRange( const RigBoundingBoxIjk<caf::VecIjk0>& range )
{
    m_iMin = static_cast<int>( range.min().i() ) + 1;
    m_iMax = static_cast<int>( range.max().i() ) + 1;
    m_jMin = static_cast<int>( range.min().j() ) + 1;
    m_jMax = static_cast<int>( range.max().j() ) + 1;
    m_kMin = static_cast<int>( range.min().k() ) + 1;
    m_kMax = static_cast<int>( range.max().k() ) + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::setToDefaultValues()
{
    RigMainGrid* grid = mainGrid();
    if ( !grid ) return;

    int ni = static_cast<int>( grid->cellCountI() );
    int nj = static_cast<int>( grid->cellCountJ() );
    int nk = static_cast<int>( grid->cellCountK() );

    m_axis       = GridAxis::AXIS_I;
    m_iMin       = 1;
    m_iMax       = ni;
    m_jMin       = 1;
    m_jMax       = nj;
    m_kMin       = 1;
    m_kMax       = nk;
    m_fixedIndex = ni / 2 + 1;
}

//--------------------------------------------------------------------------------------------------
/// The I and J axes give a vertical curtain. A K slice is horizontal, and would require the surface
/// to be contoured instead of projected along a vertical ray.
//--------------------------------------------------------------------------------------------------
bool RimIjkIntersection::supportsSurfaceIntersectionCurves() const
{
    return m_axis() != GridAxis::AXIS_K;
}

//--------------------------------------------------------------------------------------------------
/// Walk the cells of the index plane along the varying axis and pick the cell face corners. Each
/// pillar runs from the top of the first k layer to the bottom of the last k layer, so a tilted
/// pillar gives a curve that lies on the curtain rather than on a vertical plane through it.
//--------------------------------------------------------------------------------------------------
RimIntersectionCurtain RimIjkIntersection::surfaceCurtain() const
{
    if ( !supportsSurfaceIntersectionCurves() ) return {};

    RigMainGrid* grid = mainGrid();
    if ( !grid ) return {};

    const auto cellRange = clampedCellRange( grid );
    if ( !cellRange ) return {};

    const bool alongJ = m_axis() == GridAxis::AXIS_I;

    // Corner indices of the cell face, see the hex vertex numbering in StructGridInterface::cellFaceVertexIndices().
    // The first corner is at the start of the varying axis, the second at the end of the same axis. Corners 0-3 are
    // the low k side of the cell, and adding 4 gives the corner on the same pillar at the high k side.
    const int cornersPerKSide = 4;

    int startCorner = 0;
    int endCorner   = 0;
    if ( alongJ )
    {
        startCorner = m_useNegativeFace() ? 0 : 1;
        endCorner   = m_useNegativeFace() ? 3 : 2;
    }
    else
    {
        startCorner = m_useNegativeFace() ? 0 : 3;
        endCorner   = m_useNegativeFace() ? 1 : 2;
    }

    // The fixed axis is collapsed to a single index by clampedCellRange()
    const size_t fixedI = cellRange->min().i();
    const size_t fixedJ = cellRange->min().j();

    const size_t firstK = cellRange->min().k();
    const size_t lastK  = cellRange->max().k();

    const size_t varyingMin = alongJ ? cellRange->min().j() : cellRange->min().i();
    const size_t varyingMax = alongJ ? cellRange->max().j() : cellRange->max().i();

    RimIntersectionCurtain curtain;

    // The pillar at one position is spanned between the first and the last cell of the k range. The
    // top of the pillar is used as the trace, so the curve is resampled along the top of the curtain.
    auto appendPillar = [&]( size_t i, size_t j, int corner )
    {
        const size_t topCellIndex    = grid->cellIndexFromIJK( i, j, firstK );
        const size_t bottomCellIndex = grid->cellIndexFromIJK( i, j, lastK );

        if ( topCellIndex == cvf::UNDEFINED_SIZE_T || bottomCellIndex == cvf::UNDEFINED_SIZE_T ) return;
        if ( grid->cell( topCellIndex ).isInvalid() || grid->cell( bottomCellIndex ).isInvalid() ) return;

        const auto topCorners    = grid->cellCornerVertices( topCellIndex );
        const auto bottomCorners = grid->cellCornerVertices( bottomCellIndex );

        curtain.trace.push_back( topCorners[corner] );
        curtain.pillars.emplace_back( topCorners[corner], bottomCorners[corner + cornersPerKSide] );
    };

    for ( size_t varying = varyingMin; varying <= varyingMax; ++varying )
    {
        const size_t i = alongJ ? fixedI : varying;
        const size_t j = alongJ ? varying : fixedJ;

        appendPillar( i, j, startCorner );

        if ( varying == varyingMax ) appendPillar( i, j, endCorner );
    }

    return curtain;
}

//--------------------------------------------------------------------------------------------------
/// The index range of the visible cells, with the fixed axis collapsed to the fixed index
//--------------------------------------------------------------------------------------------------
std::optional<RigBoundingBoxIjk<caf::VecIjk0>> RimIjkIntersection::clampedCellRange( const RigMainGrid* grid ) const
{
    if ( !grid ) return {};

    const size_t ni = grid->cellCountI();
    const size_t nj = grid->cellCountJ();
    const size_t nk = grid->cellCountK();
    if ( ni == 0 || nj == 0 || nk == 0 ) return {};

    const auto range = ijkRange();

    caf::VecIjk0 min   = range.min();
    caf::VecIjk0 max   = range.max();
    const size_t fixed = static_cast<size_t>( std::max( 0, fixedIndex() ) );

    switch ( m_axis() )
    {
        case GridAxis::AXIS_I:
            min.x() = max.x() = std::min( fixed, ni - 1 );
            break;
        case GridAxis::AXIS_J:
            min.y() = max.y() = std::min( fixed, nj - 1 );
            break;
        case GridAxis::AXIS_K:
        default:
            min.z() = max.z() = std::min( fixed, nk - 1 );
            break;
    }

    const RigBoundingBoxIjk<caf::VecIjk0> gridBounds( caf::VecIjk0::ZERO, caf::VecIjk0( ni - 1, nj - 1, nk - 1 ) );

    return RigBoundingBoxIjk<caf::VecIjk0>( min, max ).clamp( gridBounds );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivIjkIntersectionPartMgr* RimIjkIntersection::intersectionPartMgr()
{
    if ( m_intersectionPartMgr.isNull() ) m_intersectionPartMgr = new RivIjkIntersectionPartMgr( this );

    return m_intersectionPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::clearGeometry()
{
    m_intersectionPartMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RimIjkIntersection::intersectionGeometryGenerator() const
{
    if ( m_intersectionPartMgr.notNull() ) return m_intersectionPartMgr->intersectionGeometryGenerator();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimIjkIntersection::mainGrid() const
{
    auto eclipseView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( eclipseView ) return eclipseView->mainGrid();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimIjkIntersection::axisCellCount() const
{
    RigMainGrid* grid = mainGrid();
    if ( !grid ) return 0;

    switch ( m_axis() )
    {
        case GridAxis::AXIS_I:
            return static_cast<int>( grid->cellCountI() );
        case GridAxis::AXIS_J:
            return static_cast<int>( grid->cellCountJ() );
        case GridAxis::AXIS_K:
            return static_cast<int>( grid->cellCountK() );
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    auto* sliderAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
    if ( !sliderAttr ) return;

    RigMainGrid* grid = mainGrid();
    if ( !grid ) return;

    int ni = static_cast<int>( grid->cellCountI() );
    int nj = static_cast<int>( grid->cellCountJ() );
    int nk = static_cast<int>( grid->cellCountK() );

    if ( field == &m_fixedIndex )
    {
        sliderAttr->m_minimum = 1;
        sliderAttr->m_maximum = std::max( 1, axisCellCount() );
    }
    else if ( field == &m_iMin || field == &m_iMax )
    {
        sliderAttr->m_minimum = 1;
        sliderAttr->m_maximum = std::max( 1, ni );
    }
    else if ( field == &m_jMin || field == &m_jMax )
    {
        sliderAttr->m_minimum = 1;
        sliderAttr->m_maximum = std::max( 1, nj );
    }
    else if ( field == &m_kMin || field == &m_kMax )
    {
        sliderAttr->m_minimum = 1;
        sliderAttr->m_maximum = std::max( 1, nk );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Options" );
        group->add( &m_axis );
        group->add( &m_useNegativeFace );
        group->add( &m_showInactiveCells );

        QString axisLabel = caf::AppEnum<GridAxis>::uiText( m_axis() );
        m_fixedIndex.uiCapability()->setUiName( axisLabel + " Index" );
        group->add( &m_fixedIndex );
    }

    RigMainGrid* grid = mainGrid();
    int          ni   = grid ? static_cast<int>( grid->cellCountI() ) : 0;
    int          nj   = grid ? static_cast<int>( grid->cellCountJ() ) : 0;
    int          nk   = grid ? static_cast<int>( grid->cellCountK() ) : 0;

    if ( m_axis() != GridAxis::AXIS_I )
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( QString( "I Range [1 .. %1]" ).arg( std::max( 1, ni ) ) );
        group->add( &m_iMin );
        group->add( &m_iMax );
    }

    if ( m_axis() != GridAxis::AXIS_J )
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( QString( "J Range [1 .. %1]" ).arg( std::max( 1, nj ) ) );
        group->add( &m_jMin );
        group->add( &m_jMax );
    }

    if ( m_axis() != GridAxis::AXIS_K )
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( QString( "K Range [1 .. %1]" ).arg( std::max( 1, nk ) ) );
        group->add( &m_kMin );
        group->add( &m_kMax );
    }

    defineSeparateDataSourceUi( uiConfigName, uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    appendCommonMenuItems( menuBuilder );

    menuBuilder << "RicCopyIntersectionsToAllViewsInCaseFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    appendSurfaceIntersectionsToTreeOrdering( uiTreeOrdering );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RigMainGrid* grid = mainGrid();
    if ( grid )
    {
        int ni = static_cast<int>( grid->cellCountI() );
        int nj = static_cast<int>( grid->cellCountJ() );
        int nk = static_cast<int>( grid->cellCountK() );

        auto clamp = []( caf::PdmField<int>& f, int hi ) { f = std::max( 1, std::min( f(), hi ) ); };

        clamp( m_iMin, ni );
        clamp( m_iMax, ni );
        clamp( m_jMin, nj );
        clamp( m_jMax, nj );
        clamp( m_kMin, nk );
        clamp( m_kMax, nk );
        clamp( m_fixedIndex, std::max( 1, axisCellCount() ) );

        // Keep each min/max pair ordered by limiting the changed field to the other one
        if ( changedField == &m_iMin ) m_iMin = std::min( m_iMin(), m_iMax() );
        if ( changedField == &m_iMax ) m_iMax = std::max( m_iMin(), m_iMax() );
        if ( changedField == &m_jMin ) m_jMin = std::min( m_jMin(), m_jMax() );
        if ( changedField == &m_jMax ) m_jMax = std::max( m_jMin(), m_jMax() );
        if ( changedField == &m_kMin ) m_kMin = std::min( m_kMin(), m_kMax() );
        if ( changedField == &m_kMax ) m_kMax = std::max( m_kMin(), m_kMax() );
    }

    if ( changedField == &m_axis )
    {
        updateConnectedEditors();
    }

    if ( changedField != &m_name )
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIjkIntersection::rebuildGeometryAndScheduleCreateDisplayModel()
{
    m_intersectionPartMgr = nullptr;

    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView )
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}
