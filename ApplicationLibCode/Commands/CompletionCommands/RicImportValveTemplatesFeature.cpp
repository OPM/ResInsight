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
                                                                  "Text Files (*.txt);;All Files (*.*)" );

    RimValveTemplateCollection* templateColl = RimProject::current()->allValveTemplateCollections().front();
    for ( const auto& fileName : fileNames )
    {
        auto values = RiaOpmParserTools::extractWsegAicd( fileName.toStdString() );
        for ( const auto value : values )
        {
            auto newTemplate = RimValveTemplate::createAicdTemplate( value );
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
