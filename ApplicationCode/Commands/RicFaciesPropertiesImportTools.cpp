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

#include "RicFaciesPropertiesImportTools.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimFaciesProperties.h"
#include "RimFractureModelTemplate.h"
#include "RimProject.h"

#include "RifColorLegendData.h"
#include "RifRoffReader.h"

#include "cafColorTable.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFaciesPropertiesImportTools::importFaciesPropertiesFromFile( const QString&            filePath,
                                                                     RimFractureModelTemplate* fractureModelTemplate,
                                                                     bool                      createColorLegend )
{
    if ( filePath.isEmpty() ) return;

    std::map<int, QString> codeNames;
    try
    {
        RifRoffReader::readCodeNames( filePath, codeNames );
    }
    catch ( RifRoffReaderException& ex )
    {
        RiaLogging::error( QString::fromStdString( ex.message ) );
        return;
    }

    RimFaciesProperties* faciesProperties = fractureModelTemplate->faciesProperties();
    if ( !faciesProperties )
    {
        faciesProperties = new RimFaciesProperties;
    }

    for ( auto it : codeNames )
    {
        faciesProperties->setFaciesCodeName( it.first, it.second );
    }

    if ( createColorLegend )
    {
        const caf::ColorTable&    colorTable            = RiaColorTables::contrastCategoryPaletteColors();
        RimColorLegendCollection* colorLegendCollection = RimProject::current()->colorLegendCollection;
        RimColorLegend* rockTypeColorLegend = colorLegendCollection->findByName( RiaDefines::rockTypeColorLegendName() );

        RimColorLegend* colorLegend = new RimColorLegend;
        colorLegend->setColorLegendName( RiaDefines::faciesColorLegendName() );

        for ( auto it : codeNames )
        {
            RimColorLegendItem* colorLegendItem = new RimColorLegendItem;

            // Try to find a color from the rock type color legend by fuzzy matching names
            cvf::Color3f color;
            if ( !predefinedColorMatch( it.second, rockTypeColorLegend, color ) &&
                 !matchByName( it.second, rockTypeColorLegend, color ) )
            {
                // No match use a random color
                color = colorTable.cycledColor3f( it.first );
            }

            colorLegendItem->setValues( it.second, it.first, color );
            colorLegend->appendColorLegendItem( colorLegendItem );
        }

        colorLegendCollection->appendCustomColorLegend( colorLegend );
        colorLegendCollection->updateConnectedEditors();

        faciesProperties->setColorLegend( colorLegend );
    }

    faciesProperties->setFilePath( filePath );

    fractureModelTemplate->setFaciesProperties( faciesProperties );
    fractureModelTemplate->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicFaciesPropertiesImportTools::matchByName( const QString& name, RimColorLegend* colorLegend, cvf::Color3f& color )
{
    // No match if color legend does not exist
    if ( !colorLegend ) return false;

    // Find the best matching name from the legend
    int                 bestScore = std::numeric_limits<int>::max();
    RimColorLegendItem* bestItem  = nullptr;
    for ( RimColorLegendItem* item : colorLegend->colorLegendItems() )
    {
        int score = computeEditDistance( name, item->categoryName() );
        if ( score < bestScore )
        {
            bestScore = score;
            bestItem  = item;
        }
    }

    // Allow only small difference when determining if something matches
    const int maximumScoreToMatch = 1;
    if ( bestScore <= maximumScoreToMatch )
    {
        color = bestItem->color();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicFaciesPropertiesImportTools::computeEditDistance( const QString& a, const QString& b )
{
    // Remove common words from the domain which does not help in the matching
    std::vector<QString> stopWords   = {"rocks", "rock", "stones", "stone"};
    QString              aSimplified = a.toLower();
    QString              bSimplified = b.toLower();
    for ( auto r : stopWords )
    {
        aSimplified.remove( r );
        bSimplified.remove( r );
    }

    aSimplified = aSimplified.trimmed();
    bSimplified = bSimplified.trimmed();

    return RiaStdStringTools::computeEditDistance( aSimplified.toStdString(), bSimplified.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicFaciesPropertiesImportTools::predefinedColorMatch( const QString&  name,
                                                           RimColorLegend* colorLegend,
                                                           cvf::Color3f&   color )
{
    // Calcite should use limestone color
    if ( name.toLower().trimmed() == QString( "calcite" ) )
    {
        for ( auto i : colorLegend->colorLegendItems() )
        {
            if ( i->categoryName() == QString( "Limestone" ) )
            {
                color = i->color();
                return true;
            }
        }
    }

    return false;
}
