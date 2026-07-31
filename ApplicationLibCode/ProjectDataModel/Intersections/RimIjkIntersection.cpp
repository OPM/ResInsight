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
    setDefault( RimIjkIntersection::GridAxis::AXIS_K );
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

    m_axis       = GridAxis::AXIS_K;
    m_iMin       = 1;
    m_iMax       = ni;
    m_jMin       = 1;
    m_jMax       = nj;
    m_kMin       = 1;
    m_kMax       = nk;
    m_fixedIndex = nk / 2 + 1;
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
