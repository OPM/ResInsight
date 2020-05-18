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

#include "RimColorLegend.h"
#include "RimColorLegendItem.h"

CAF_PDM_SOURCE_INIT( RimColorLegend, "ColorLegend" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend::RimColorLegend()
{
    CAF_PDM_InitObject( "ColorLegend", ":/Legend.png", "", "" );

    CAF_PDM_InitField( &m_colorLegendName, "ColorLegendName", QString( "" ), "Color Legend Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_colorLegendItems, "ColorLegendItems", "", "", "", "" );
    m_colorLegendItems.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend::~RimColorLegend()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimColorLegend::colorLegendName()
{
    return m_colorLegendName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegend::setColorLegendName( const QString& colorLegendName )
{
    m_colorLegendName = colorLegendName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegend::setReadOnly( bool doReadOnly )
{
    m_colorLegendName.uiCapability()->setUiReadOnly( true );
    setDeletable( !doReadOnly );

    for ( auto colorLegendItem : m_colorLegendItems )
    {
        colorLegendItem->setReadOnly( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegend::appendColorLegendItem( RimColorLegendItem* colorLegendItem )
{
    m_colorLegendItems.push_back( colorLegendItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimColorLegendItem*> RimColorLegend::colorLegendItems() const
{
    return m_colorLegendItems.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimColorLegend::userDescriptionField()
{
    return &m_colorLegendName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegend::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
}
