/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RicImportFaciesPropertiesFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RifFaciesPropertiesReader.h"
#include "RifFileParseTools.h"

#include "RigFaciesProperties.h"

#include "RimFaciesProperties.h"
#include "RimFractureModel.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

#include <set>
#include <vector>

CAF_CMD_SOURCE_INIT( RicImportFaciesPropertiesFeature, "RicImportFaciesPropertiesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFaciesPropertiesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesPropertiesFeature::onActionTriggered( bool isChecked )
{
    RimFractureModel* fractureModel = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimFractureModel>();
    if ( !fractureModel ) return;

    // Open dialog box to select files
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "FACIES_DIR" );
    QString         filePath   = QFileDialog::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(),
                                                     "Import Facies Properties",
                                                     defaultDir,
                                                     "Facies Properties (*.csv);;All Files (*.*)" );

    if ( filePath.isNull() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "FACIES_DIR", QFileInfo( filePath ).absolutePath() );

    typedef std::tuple<QString, QString, QString> FaciesKey;

    // Read the facies properties from file
    std::vector<RifFaciesProperties> rifFaciesProperties;
    try
    {
        QStringList filePaths;
        filePaths << filePath;
        RifFaciesPropertiesReader::readFaciesProperties( rifFaciesProperties, filePaths );
    }
    catch ( FileParseException& exception )
    {
        RiaLogging::warning( QString( "Facies properties import failed: '%1'." ).arg( exception.message ) );
        return;
    }

    // Find the unique facies keys (combination of field, formation and facies names)
    std::set<FaciesKey> faciesKeys;
    for ( RifFaciesProperties item : rifFaciesProperties )
    {
        FaciesKey faciesKey = std::make_tuple( item.fieldName, item.formationName, item.faciesName );
        faciesKeys.insert( faciesKey );
    }

    RimFaciesProperties* rimFaciesProperties = new RimFaciesProperties;
    //    rimFaciesProperties->setFilePath();
    for ( FaciesKey key : faciesKeys )
    {
        std::vector<RifFaciesProperties> matchingFacies;

        QString fieldName     = std::get<0>( key );
        QString formationName = std::get<1>( key );
        QString faciesName    = std::get<2>( key );

        // Group the items with a given facies key
        for ( RifFaciesProperties item : rifFaciesProperties )
        {
            if ( item.fieldName == fieldName && item.formationName == formationName && item.faciesName == faciesName )
            {
                matchingFacies.push_back( item );
            }
        }

        // Sort the matching items by porosity
        std::sort( matchingFacies.begin(),
                   matchingFacies.end(),
                   []( const RifFaciesProperties& a, const RifFaciesProperties& b ) { return a.porosity < b.porosity; } );

        // Finally add the values
        RigFaciesProperties rigFaciesProperties( fieldName, formationName, faciesName );
        for ( RifFaciesProperties item : matchingFacies )
        {
            rigFaciesProperties.appendValues( item.porosity,
                                              item.youngsModulus,
                                              item.poissonsRatio,
                                              item.K_Ic,
                                              item.proppantEmbedment );
        }

        rimFaciesProperties->setPropertiesForFacies( key, rigFaciesProperties );
    }

    rimFaciesProperties->setFilePath( filePath );
    fractureModel->setFaciesProperties( rimFaciesProperties );
    fractureModel->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFaciesPropertiesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Facies Properties" );
    // TODO: add icon?
    // actionToSetup->setIcon( QIcon( ":/FaciesProperties16x16.png" ) );
}
