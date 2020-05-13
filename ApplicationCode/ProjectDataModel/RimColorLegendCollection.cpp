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

#include "RimColorLegendCollection.h"
#include "RimColorLegend.h"
#include "RimRegularLegendConfig.h"

#include "RimColorLegendItem.h"
#include "RimProject.h"
#include <QString>

CAF_PDM_SOURCE_INIT( RimColorLegendCollection, "ColorLegendCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegendCollection::RimColorLegendCollection()
{
    CAF_PDM_InitObject( "Color Legend Collection", ":/Legend.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_standardColorLegends,
                                "StandardColorLegends",
                                "Standard Color Legends",
                                ":/Legend.png",
                                "",
                                "" );

    CAF_PDM_InitFieldNoDefault( &m_customColorLegends, "CustomColorLegends", "Custom Color Legends", ":/Legend.png", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegendCollection::~RimColorLegendCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegendCollection::appendCustomColorLegend( RimColorLegend* colorLegend )
{
    m_customColorLegends.push_back( colorLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimColorLegendCollection::isStandardColorLegend( RimColorLegend* legend )
{
    for ( auto standardLegend : m_standardColorLegends )
    {
        if ( legend == standardLegend ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegendCollection::deleteCustomColorLegends()
{
    m_customColorLegends.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegendCollection::createStandardColorLegends()
{
    typedef caf::AppEnum<RimRegularLegendConfig::ColorRangesType> ColorRangeEnum;

    for ( size_t typeIdx = 0; typeIdx < ColorRangeEnum::size(); typeIdx++ )
    {
        QString            legendName = ColorRangeEnum::uiTextFromIndex( typeIdx );
        cvf::Color3ubArray colorArray =
            RimRegularLegendConfig::colorArrayFromColorType( ColorRangeEnum::fromIndex( typeIdx ) );

        RimColorLegend* colorLegend = new RimColorLegend;
        colorLegend->setColorLegendName( legendName );

        for ( int i = (int)colorArray.size() - 1; i > -1; i-- ) // reverse to assign last color to top of legend
        {
            cvf::Color3f color3f( colorArray[i] );
            QColor       colorQ( colorArray[i].r(), colorArray[i].g(), colorArray[i].b() );

            RimColorLegendItem* colorLegendItem = new RimColorLegendItem;
            colorLegendItem->setValues( colorQ.name(), i, color3f );

            colorLegend->appendColorLegendItem( colorLegendItem );
            colorLegend->setReadOnly( true );
        }

        m_standardColorLegends.push_back( colorLegend );
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimColorLegend*> RimColorLegendCollection::customColorLegends() const
{
    std::vector<RimColorLegend*> allLegends;

    auto standardLegends = m_standardColorLegends.childObjects();
    for ( auto l : standardLegends )
    {
        allLegends.push_back( l );
    }

    auto customLegends = m_customColorLegends.childObjects();
    for ( auto l : customLegends )
    {
        allLegends.push_back( l );
    }

    return allLegends;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimColorLegendCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
}
