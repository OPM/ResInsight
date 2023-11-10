/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RicImportGridCalculationExpressionsFeature.h"
#include "RicExportGridCalculationExpressionsFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RimEclipseCaseTools.h"
#include "RimGridCalculationCollection.h"
#include "RimProject.h"

#include "RiuFileDialogTools.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmXmlObjectHandle.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

CAF_CMD_SOURCE_INIT( RicImportGridCalculationExpressionsFeature, "RicImportGridCalculationExpressionsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridCalculationExpressionsFeature::onActionTriggered( bool isChecked )
{
    auto app = RiaGuiApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectoryWithFallback( RicExportGridCalculationExpressionsFeature::gridCalculationExpressionId(),
                                                                   RiaPreferences::current()->gridCalculationExpressionFolder() );

    QString fileName =
        RiuFileDialogTools::getOpenFileName( nullptr, "Import Grid Calculation Expressions", defaultDir, "Xml File(*.xml);;All files(*.*)" );

    if ( fileName.isEmpty() ) return;

    QFile importFile( fileName );
    if ( !importFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        RiaLogging::error( QString( "Import Grid Calculation Expressions : Could not open the file: %1" ).arg( fileName ) );
        return;
    }

    QTextStream stream( &importFile );
    QString     objectAsText = stream.readAll();

    auto proj     = RimProject::current();
    auto calcColl = proj->gridCalculationCollection();

    RimEclipseCase* firstCase = nullptr;
    auto            eclCases  = RimEclipseCaseTools::allEclipseGridCases();
    if ( !eclCases.empty() ) firstCase = eclCases.front();

    RimGridCalculationCollection tmp;
    tmp.xmlCapability()->readObjectFromXmlString( objectAsText, caf::PdmDefaultObjectFactory::instance() );
    for ( auto calc : tmp.calculations() )
    {
        auto gridCalculation = dynamic_cast<RimGridCalculation*>( calcColl->addCalculationCopy( calc ) );
        if ( gridCalculation && firstCase )
        {
            gridCalculation->assignEclipseCaseForNullPointers( firstCase );
        }
    }

    QString absPath = QFileInfo( fileName ).absolutePath();
    app->setLastUsedDialogDirectory( RicExportGridCalculationExpressionsFeature::gridCalculationExpressionId(), absPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridCalculationExpressionsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Grid Calculation Expressions" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
