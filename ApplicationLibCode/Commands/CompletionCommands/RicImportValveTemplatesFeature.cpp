/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicImportValveTemplatesFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaOpmParserTools.h"

#include "RimProject.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportValveTemplatesFeature, "RicImportValveTemplatesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportValveTemplatesFeature::isCommandEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportValveTemplatesFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );

    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Valve Templates",
                                                                  defaultDir,
                                                                  "Valve Files (*.sch *.case);;All Files (*.*)" );

    if ( RimProject::current()->allValveTemplateCollections().empty() )
    {
        RiaLogging::error( "No valve template collection found, failed to import valves." );
        return;
    }

    RimValveTemplateCollection* templateColl = RimProject::current()->allValveTemplateCollections().front();
    for ( const auto& fileName : fileNames )
    {
        std::vector<RiaOpmParserTools::AicdTemplateValues> aicdTemplates;

        if ( fileName.contains( ".case" ) )
        {
            aicdTemplates = RiaOpmParserTools::extractWsegAicdCompletor( fileName.toStdString() );
        }
        else
        {
            aicdTemplates = RiaOpmParserTools::extractWsegAicd( fileName.toStdString() );
        }

        // There can be multiple items of the same template, make sure we have unique templates
        std::sort( aicdTemplates.begin(),
                   aicdTemplates.end(),
                   []( RiaOpmParserTools::AicdTemplateValues& templateA, RiaOpmParserTools::AicdTemplateValues& templateB )
                   {
                       int idA = std::numeric_limits<int>::max();
                       int idB = std::numeric_limits<int>::max();

                       auto itA = templateA.find( RiaOpmParserTools::aicdTemplateId() );
                       if ( itA != templateA.end() )
                       {
                           idA = itA->second;
                       }

                       auto itB = templateB.find( RiaOpmParserTools::aicdTemplateId() );
                       if ( itB != templateB.end() )
                       {
                           idB = itB->second;
                       }

                       if ( idA != std::numeric_limits<int>::max() && idB != std::numeric_limits<int>::max() )
                       {
                           // Sort by id if both have id
                           return idA < idB;
                       }

                       return templateA < templateB;
                   } );

        auto it = std::unique( aicdTemplates.begin(), aicdTemplates.end() );
        aicdTemplates.resize( std::distance( aicdTemplates.begin(), it ) );

        int number = 1;
        for ( const auto& aicdValue : aicdTemplates )
        {
            auto newTemplate = RimValveTemplate::createAicdTemplate( aicdValue, number++ );
            templateColl->addValveTemplate( newTemplate );
        }
    }

    templateColl->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportValveTemplatesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ICDValve16x16.png" ) );
    actionToSetup->setText( "Import Valve Templates" );
}
