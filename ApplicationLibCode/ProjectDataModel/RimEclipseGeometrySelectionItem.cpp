/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimEclipseGeometrySelectionItem.h"

#include "RiaTextStringTools.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigTimeHistoryResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimTools.h"

#include "Riu3dSelectionManager.h"

CAF_PDM_SOURCE_INIT( RimEclipseGeometrySelectionItem, "EclipseGeometrySelectionItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseGeometrySelectionItem::RimEclipseGeometrySelectionItem()
{
    CAF_PDM_InitObject( "Eclipse Geometry Selection Item" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );
    CAF_PDM_InitFieldNoDefault( &m_gridIndex, "GridIndex", "Grid Index" );
    CAF_PDM_InitFieldNoDefault( &m_cellIndex, "CellIndex", "Cell Index" );

    CAF_PDM_InitFieldNoDefault( &m_ijkText, "CellText", "Cell I J K" );
    m_ijkText.registerGetMethod( this, &RimEclipseGeometrySelectionItem::ijkTextFromCell );
    m_ijkText.registerSetMethod( this, &RimEclipseGeometrySelectionItem::setCellFromIjkText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseGeometrySelectionItem::~RimEclipseGeometrySelectionItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseGeometrySelectionItem::setFromSelectionItem( const RiuEclipseSelectionItem* selectionItem )
{
    m_gridIndex = selectionItem->m_gridIndex;
    m_cellIndex = selectionItem->m_gridLocalCellIndex;

    m_eclipseCase = selectionItem->m_resultDefinition->eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseGeometrySelectionItem::setFromSelectionItem( RimEclipseGeometrySelectionItem* selectionItem )
{
    m_eclipseCase = selectionItem->eclipseCase();
    m_gridIndex   = selectionItem->m_gridIndex();
    m_cellIndex   = selectionItem->m_cellIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseGeometrySelectionItem::setFromCaseGridAndIJK( RimEclipseCase* eclipseCase, size_t gridIndex, size_t i, size_t j, size_t k )
{
    m_eclipseCase = eclipseCase;
    m_gridIndex   = gridIndex;

    size_t localIndex         = grid()->cellIndexFromIJK( i, j, k );
    size_t reservoirCellIndex = grid()->reservoirCellIndex( localIndex );
    m_cellIndex               = reservoirCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseGeometrySelectionItem::geometrySelectionText() const
{
    QString text;

    if ( m_eclipseCase )
    {
        text += m_eclipseCase->caseUserDescription();
    }
    else
    {
        return "No case";
    }

    text += ", ";
    text += QString( "Grid index %1" ).arg( m_gridIndex );
    text += ", ";
    text += RigTimeHistoryResultAccessor::geometrySelectionText( m_eclipseCase->eclipseCaseData(), m_gridIndex, m_cellIndex );

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseGeometrySelectionItem::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEclipseGeometrySelectionItem::gridIndex() const
{
    return m_gridIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEclipseGeometrySelectionItem::cellIndex() const
{
    return m_cellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseGeometrySelectionItem::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_ijkText );
    uiOrdering.add( &m_eclipseCase );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseGeometrySelectionItem::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::caseOptionItems( &options );

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseGeometrySelectionItem::ijkTextFromCell() const
{
    if ( grid() )
    {
        if ( auto zeroBased = grid()->ijkFromCellIndex( m_cellIndex ) )
        {
            auto oneBased = zeroBased->toOneBased();
            return QString( "%1 %2 %3" ).arg( oneBased.i() ).arg( oneBased.j() ).arg( oneBased.k() );
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseGeometrySelectionItem::setCellFromIjkText( const QString& text )
{
    auto values = RiaTextStringTools::parseDoubleValues( text );
    if ( values.size() >= 3 )
    {
        int i = values[0] - 1;
        int j = values[1] - 1;
        int k = values[2] - 1;

        if ( grid() )
        {
            size_t localIndex = grid()->cellIndexFromIJK( i, j, k );
            m_cellIndex       = grid()->reservoirCellIndex( localIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGridBase* RimEclipseGeometrySelectionItem::grid() const
{
    if ( m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->grid( m_gridIndex ) )
    {
        return m_eclipseCase->eclipseCaseData()->grid( m_gridIndex );
    }

    return nullptr;
}
