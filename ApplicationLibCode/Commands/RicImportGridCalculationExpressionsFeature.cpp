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
#include "RimEclipseResultAddress.h"
#include "RimGridCalculationCollection.h"
#include "RimGridCalculationVariable.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "RifGridCalculationImporter.h"

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
        RiuFileDialogTools::getOpenFileName( nullptr, "Import Grid Calculation Expressions", defaultDir, "Toml File(*.toml);;All files(*.*)" );
    if ( fileName.isEmpty() ) return;

    auto [calculations, errorMessage] = RifGridCalculationImporter::readFromFile( fileName.toStdString() );
    if ( !errorMessage.empty() )
    {
        RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(),
                                       "Grid Calculation Import Error",
                                       QString::fromStdString( errorMessage ) );
        return;
    }

    auto proj     = RimProject::current();
    auto calcColl = proj->gridCalculationCollection();

    RimEclipseCase* firstCase = nullptr;
    auto            eclCases  = RimEclipseCaseTools::nativeEclipseGridCases();
    if ( !eclCases.empty() ) firstCase = eclCases.front();

    for ( auto calc : calculations )
    {
        bool addDefaultExpression = false;
        auto gridCalculation      = dynamic_cast<RimGridCalculation*>( calcColl->addCalculation( addDefaultExpression ) );
        if ( gridCalculation )
        {
            gridCalculation->setExpression( QString::fromStdString( calc.expression ) );
            gridCalculation->setDescription( QString::fromStdString( calc.description ) );
            gridCalculation->setUnit( QString::fromStdString( calc.unit ) );
            for ( auto var : calc.variables )
            {
                auto variable = dynamic_cast<RimGridCalculationVariable*>( gridCalculation->addVariable( QString::fromStdString( var.name ) ) );
                RimEclipseResultAddress address;

                RiaDefines::ResultCatType myEnum =
                    caf::AppEnum<RiaDefines::ResultCatType>::fromText( QString::fromStdString( var.resultType ) );
                address.setEclipseCase( firstCase );
                address.setResultName( QString::fromStdString( var.resultVariable ) );
                address.setResultType( myEnum );
                variable->setEclipseResultAddress( address );
            }

            if ( firstCase )
            {
                gridCalculation->assignEclipseCaseForNullPointers( firstCase );
            }
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
