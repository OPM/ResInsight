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

#include "RicExportGridCalculationExpressionsFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RimGridCalculationCollection.h"
#include "RimProject.h"

#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicExportGridCalculationExpressionsFeature, "RicExportGridCalculationExpressionsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportGridCalculationExpressionsFeature::gridCalculationExpressionId()
{
    return "GRID_CALCULATION_EXPRESSION";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportGridCalculationExpressionsFeature::onActionTriggered( bool isChecked )
{
    auto proj     = RimProject::current();
    auto calcColl = proj->gridCalculationCollection();

    if ( calcColl->calculations().empty() ) return;

    auto objectAsText = calcColl->writeObjectToXmlString();
    if ( objectAsText.isEmpty() ) return;

    QString fallbackPath = RiaPreferences::current()->gridCalculationExpressionFolder();
    auto    app          = RiaGuiApplication::instance();
    QString startPath =
        app->lastUsedDialogDirectoryWithFallback( RicExportGridCalculationExpressionsFeature::gridCalculationExpressionId(), fallbackPath );

    QString fileName = RiuFileDialogTools::getSaveFileName( nullptr,
                                                            "Select File for Grid Calculation Expression Export",
                                                            startPath,
                                                            "Xml File(*.xml);;All files(*.*)" );
    if ( fileName.isEmpty() ) return;

    QFile exportFile( fileName );
    if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        RiaLogging::errorInMessageBox( nullptr,
                                       "Export Grid Calculation Expressions",
                                       QString( "Could not save to the file: %1" ).arg( fileName ) );
        return;
    }

    QString absPath = QFileInfo( fileName ).absolutePath();

    app->setLastUsedDialogDirectory( RicExportGridCalculationExpressionsFeature::gridCalculationExpressionId(), absPath );
    QTextStream stream( &exportFile );
    stream << objectAsText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportGridCalculationExpressionsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Grid Calculation Expressions" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
