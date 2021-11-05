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

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigTimeHistoryResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"

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
    CAF_PDM_InitFieldNoDefault( &m_localIntersectionPointInDisplay,
                                "LocalIntersectionPoint",
                                "local Intersection Point",
                                "",
                                "",
                                "" );
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
    m_gridIndex                       = selectionItem->m_gridIndex;
    m_cellIndex                       = selectionItem->m_gridLocalCellIndex;
    m_localIntersectionPointInDisplay = selectionItem->m_localIntersectionPointInDisplay;

    m_eclipseCase = selectionItem->m_resultDefinition->eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseGeometrySelectionItem::setFromCaseGridAndIJK( RimEclipseCase* eclipseCase,
                                                             size_t          gridIndex,
                                                             size_t          i,
                                                             size_t          j,
                                                             size_t          k )
{
    m_eclipseCase = eclipseCase;
    m_gridIndex   = gridIndex;

    size_t lgrCellIndex       = eclipseCase->eclipseCaseData()->grid( gridIndex )->cellIndexFromIJK( i, j, k );
    size_t reservoirCellIndex = eclipseCase->eclipseCaseData()->grid( gridIndex )->reservoirCellIndex( lgrCellIndex );
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
    text +=
        RigTimeHistoryResultAccessor::geometrySelectionText( m_eclipseCase->eclipseCaseData(), m_gridIndex, m_cellIndex );

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
cvf::Vec3st RimEclipseGeometrySelectionItem::cellIJK() const
{
    cvf::Vec3st IJK( -1, -1, -1 );

    if ( m_cellIndex != cvf::UNDEFINED_SIZE_T )
    {
        if ( m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->grid( m_gridIndex ) )
        {
            m_eclipseCase->eclipseCaseData()->grid( m_gridIndex )->ijkFromCellIndex( m_cellIndex, &IJK[0], &IJK[1], &IJK[2] );
        }
    }

    return IJK;
}
