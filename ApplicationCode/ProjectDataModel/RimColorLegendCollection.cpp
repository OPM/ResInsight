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

#include "RiaFractureDefines.h"

#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"

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
    m_standardColorLegends.xmlCapability()->disableIO();

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
        auto colorType = ColorRangeEnum::fromIndex( typeIdx );

        if ( colorType != RimRegularLegendConfig::ColorRangesType::UNDEFINED )
        {
            QString            legendName = ColorRangeEnum::uiTextFromIndex( typeIdx );
            cvf::Color3ubArray colorArray = RimRegularLegendConfig::colorArrayFromColorType( colorType );

            if ( colorType == RimRegularLegendConfig::ColorRangesType::CATEGORY )
            {
                // Reverse the ordering of the category items in the category legend to match the changes
                // for fixing issue https://github.com/OPM/ResInsight/issues/6252

                auto other = colorArray;
                for ( size_t i = 0; i < colorArray.size(); i++ )
                {
                    colorArray[i] = other[colorArray.size() - 1 - i];
                }
            }

            RimColorLegend* colorLegend = new RimColorLegend;
            colorLegend->setColorLegendName( legendName );

            for ( size_t i = 0; i < colorArray.size(); i++ )
            {
                cvf::Color3f color3f( colorArray[i] );
                QColor       colorQ( colorArray[i].r(), colorArray[i].g(), colorArray[i].b() );

                RimColorLegendItem* colorLegendItem = new RimColorLegendItem;
                colorLegendItem->setValues( colorQ.name(), static_cast<int>( i ), color3f );

                colorLegend->appendColorLegendItem( colorLegendItem );
                colorLegend->setReadOnly( true );
            }

            m_standardColorLegends.push_back( colorLegend );
        }
    }

    m_standardColorLegends.push_back( createRockTypeColorLegend() );

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimColorLegend*> RimColorLegendCollection::allColorLegends() const
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimColorLegendCollection::findByName( const QString& name ) const
{
    std::vector<RimColorLegend*> allLegends = allColorLegends();
    for ( auto legend : allLegends )
    {
        if ( legend->colorLegendName() == name )
        {
            return legend;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegendItem* RimColorLegendCollection::createColorLegendItem( const QString& name, int r, int g, int b ) const
{
    RimColorLegendItem* item = new RimColorLegendItem;
    item->setValues( name, 0, cvf::Color3f::fromByteColor( r, g, b ) );
    return item;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimColorLegendCollection::createRockTypeColorLegend() const
{
    RimColorLegend* colorLegend = new RimColorLegend;
    colorLegend->setColorLegendName( RiaDefines::rockTypeColorLegendName() );

    // Rock types colors taken from "Equinor GeoStandard - May 2020" document.
    // 6.3.1 Epiclastic rocks
    colorLegend->appendColorLegendItem( createColorLegendItem( "Conglomerate", 255, 193, 0 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Sedimentary breccia", 255, 193, 0 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Sandstone", 255, 247, 143 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Siltstone", 179, 212, 84 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Mudstone", 31, 180, 108 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Claystone", 101, 167, 64 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Fissile siltstone", 179, 212, 84 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Fissil mudstone", 31, 180, 108 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Shale", 136, 112, 0 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Grey silty clay", 197, 179, 115 ) );

    // 6.3.2 Carbonate rocks
    colorLegend->appendColorLegendItem( createColorLegendItem( "Limestone", 0, 165, 203 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Dolomitic limestone", 0, 155, 219 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Dolostone", 54, 17, 99 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Calcareous dolostone", 115, 173, 222 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Chalk", 0, 181, 214 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Marl", 98, 200, 206 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Spiculities", 214, 133, 137 ) );

    // 6.3.3 Evaporitic rocks
    colorLegend->appendColorLegendItem( createColorLegendItem( "Gypsum", 241, 119, 170 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Anhydrite", 241, 119, 170 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Gypsum / Anhydrite", 241, 119, 170 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Halite", 246, 160, 169 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Salt", 246, 160, 169 ) );

    // 6.3.4 Coal
    colorLegend->appendColorLegendItem( createColorLegendItem( "Coal", 0, 0, 0 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Brown coal", 76, 75, 57 ) );

    // 6.3.5 Magmatic rocks
    colorLegend->appendColorLegendItem( createColorLegendItem( "Volcanic rocks", 255, 50, 50 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Intrusive (plutonic) rocks", 251, 184, 141 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Silicic plutonic rocks", 240, 81, 70 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Mafic plutonic rocks", 128, 69, 0 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Ooze", 119, 110, 85 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Dykes and sills", 75, 57, 0 ) );

    // 6.3.6 Metamorphic rocks
    colorLegend->appendColorLegendItem( createColorLegendItem( "Metamorphic rocks", 245, 127, 51 ) );

    // 6.3.6 Combined symbols
    colorLegend->appendColorLegendItem( createColorLegendItem( "Tuffitt", 211, 128, 181 ) );
    colorLegend->appendColorLegendItem( createColorLegendItem( "Bitumenious", 235, 220, 175 ) );

    int index = 0;
    for ( auto item : colorLegend->colorLegendItems() )
    {
        item->setCategoryValue( index );
        index++;
    }

    return colorLegend;
}
