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

#include "RicImportFaciesFeature.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaLogging.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimProject.h"

#include "RifColorLegendData.h"
#include "RifRoffReader.h"

#include "Riu3DMainWindowTools.h"

#include "cafColorTable.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicImportFaciesFeature, "RicImportFaciesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFaciesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "ROFF_FILE" );

    QString filterText = QString( "Roff ascii file (*.roff);;All Files (*.*)" );

    QString fileName =
        QFileDialog::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(), "Import Facies", defaultDir, filterText );

    if ( fileName.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "ROFF_FILE", QFileInfo( fileName ).absolutePath() );

    std::map<int, QString> codeNames;
    try
    {
        RifRoffReader::readCodeNames( fileName, codeNames );
    }
    catch ( RifRoffReaderException& ex )
    {
        RiaLogging::error( QString::fromStdString( ex.message ) );
        return;
    }

    // TODO: try to map names of facies to a sensible color
    const caf::ColorTable& colorTable = RiaColorTables::contrastCategoryPaletteColors();

    RimColorLegend* colorLegend = new RimColorLegend;
    colorLegend->setColorLegendName( "Facies colors" );

    // Iterate over the map using Iterator till end.
    for ( auto it : codeNames )
    {
        RimColorLegendItem* colorLegendItem = new RimColorLegendItem;
        colorLegendItem->setValues( it.second, it.first, colorTable.cycledColor3f( it.first ) );
        colorLegend->appendColorLegendItem( colorLegendItem );
    }

    RimColorLegendCollection* colorLegendCollection = RimProject::current()->colorLegendCollection;
    colorLegendCollection->appendCustomColorLegend( colorLegend );
    colorLegendCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesFeature::setupActionLook( QAction* actionToSetup )
{
    // TODO: add icon?
    // actionToSetup->setIcon( QIcon( ":/Formations16x16.png" ) );
    actionToSetup->setText( "Import Facies" );
}
