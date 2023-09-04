/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RimCellIndexFilter.h"

#include "RimGeoMechCase.h"
#include "RimTools.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"

#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_SOURCE_INIT( RimCellIndexFilter, "CellIndexFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellIndexFilter::RimCellIndexFilter()
    : RimCellFilter( RimCellFilter::INDEX )
{
    CAF_PDM_InitObject( "Cell Index Filter", ":/CellFilter_UserDefined.png" );

    CAF_PDM_InitField( &m_setId, "ElementSetId", 0, "Element Set" );
    m_setId.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    m_propagateToSubGrids = true;

    updateIconState();
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellIndexFilter::~RimCellIndexFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellIndexFilter::fullName() const
{
    return QString( "%1  [%2 cells]" ).arg( RimCellFilter::fullName(), QString::number( m_cells.size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellIndexFilter::updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex )
{
    if ( gridIndex != m_gridIndex() ) return;

    if ( m_cells.size() == 0 )
    {
        updateCells();
    }

    if ( m_filterMode == FilterModeType::INCLUDE )
    {
        for ( auto cellIdx : m_cells )
        {
            ( *includeVisibility )[cellIdx] = true;
        }
    }
    else
    {
        for ( auto cellIdx : m_cells )
        {
            ( *excludeVisibility )[cellIdx] = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellIndexFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_gridIndex )
    {
        m_setId = 0;
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
void RimCellIndexFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_gridIndex.uiCapability()->setUiName( "Part" );

    auto group = uiOrdering.addNewGroup( "General" );
    group->add( &m_gridIndex );
    group->add( &m_setId );
    group->add( &m_filterMode );

    uiOrdering.skipRemainingFields( true );

    bool readOnlyState = isFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields = fields();
    for ( auto& objField : objFields )
    {
        objField->uiCapability()->setUiReadOnly( readOnlyState );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCellIndexFilter::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimCellFilter::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_setId )
    {
        RimTools::geoMechElementSetOptionItems( &options, geoMechCase(), m_gridIndex() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellIndexFilter::updateCells()
{
    m_cells.clear();

    auto gCase = geoMechCase();

    if ( gCase && gCase->geoMechData() && gCase->geoMechData()->femParts() )
    {
        auto parts = gCase->geoMechData()->femParts();

        auto part = parts->part( m_gridIndex() );

        auto setNames = part->elementSetNames();
        if ( m_setId() < (int)setNames.size() )
        {
            m_name = QString::fromStdString( part->elementSetNames()[m_setId] );
        }
        else
        {
            m_name = QString::fromStdString( part->name() );
        }

        auto cells = part->elementSet( m_setId() );

        for ( auto c : cells )
        {
            m_cells.push_back( c );
        }
    }
}
