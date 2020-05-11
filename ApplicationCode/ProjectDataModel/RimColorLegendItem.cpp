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

#include "RimColorLegendItem.h"

#include "cvfColor3.h"

#include <QColor>
#include <QString>

CAF_PDM_SOURCE_INIT( RimColorLegendItem, "ColorLegendItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegendItem::RimColorLegendItem()
{
    CAF_PDM_InitObject( "ColorLegendItem", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_color, "Color", "Color", "", "", "" );

    CAF_PDM_InitField( &m_categoryValue, "CategoryValue", 0, "Category Value", "", "", "" );
    CAF_PDM_InitField( &m_categoryName, "CategoryName", QString( "" ), "Category Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy", "", "", "" );
    m_nameProxy.registerGetMethod( this, &RimColorLegendItem::extractColorItemName );
    m_nameProxy.uiCapability()->setUiHidden( true );
    m_nameProxy.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegendItem::~RimColorLegendItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegendItem::setValues( const QString& categoryName, int categoryValue, const cvf::Color3f& color )
{
    m_categoryName  = categoryName;
    m_categoryValue = categoryValue;
    m_color         = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Color3f& RimColorLegendItem::color() const
{
    return m_color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimColorLegendItem::categoryName() const
{
    return m_categoryName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimColorLegendItem::categoryValue() const
{
    return m_categoryValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegendItem::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimColorLegendItem::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
/// "stringify" category value and name; category value with leading zeros presupposing max 2 digits
//--------------------------------------------------------------------------------------------------
QString RimColorLegendItem::extractColorItemName() const
{
    return QString( "%1" ).arg( m_categoryValue, 2, 10, QChar( '0' ) ) + " " + m_categoryName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegendItem::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    QColor qcolor( m_color().rByte(), m_color().gByte(), m_color().bByte() );

    caf::QIconProvider iconProvider = this->uiIconProvider();
    iconProvider.setBackgroundColor( qcolor );
    this->setUiIcon( iconProvider );
}
