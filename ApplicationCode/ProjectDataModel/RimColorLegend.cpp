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

#include "RiaColorTools.h"

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegend::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    this->setUiIcon( paletteIconProvider() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3ubArray RimColorLegend::colorArray() const
{
    std::vector<RimColorLegendItem*> legendItems = colorLegendItems();
    cvf::Color3ubArray               colorArray( legendItems.size() );
    for ( size_t i = 0; i < legendItems.size(); i++ )
    {
        colorArray.set( i, cvf::Color3ub( legendItems[i]->color() ) );
    }
    return colorArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider RimColorLegend::paletteIconProvider() const
{
    std::vector<QString>             colorNames;
    std::vector<RimColorLegendItem*> legendItems = colorLegendItems();
    for ( auto legendItem : legendItems )
    {
        QColor color = RiaColorTools::toQColor( legendItem->color() );
        colorNames.push_back( color.name() );
    }
    caf::IconProvider iconProvider( QSize( 24, 16 ) );
    iconProvider.setBackgroundColorGradient( colorNames );
    return iconProvider;
}
