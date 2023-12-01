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

#include "RicImportSummaryCalculationExpressionsFeature.h"
#include "RicExportSummaryCalculationExpressionsFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RimEclipseCaseTools.h"
#include "RimEclipseResultAddress.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "RifSummaryCalculationImporter.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

CAF_CMD_SOURCE_INIT( RicImportSummaryCalculationExpressionsFeature, "RicImportSummaryCalculationExpressionsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCalculationExpressionsFeature::onActionTriggered( bool isChecked )
{
    auto app = RiaGuiApplication::instance();

    QString defaultDir =
        app->lastUsedDialogDirectoryWithFallback( RicExportSummaryCalculationExpressionsFeature::summaryCalculationExpressionId(),
                                                  RiaPreferences::current()->summaryCalculationExpressionFolder() );

    QString fileName =
        RiuFileDialogTools::getOpenFileName( nullptr, "Import Summary Calculation Expressions", defaultDir, "Toml File(*.toml);;All files(*.*)" );
    if ( fileName.isEmpty() ) return;

    auto [calculations, errorMessage] = RifSummaryCalculationImporter::readFromFile( fileName.toStdString() );
    if ( !errorMessage.empty() )
    {
        RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(),
                                       "Summary Calculation Import Error",
                                       QString::fromStdString( errorMessage ) );
        return;
    }

    auto proj     = RimProject::current();
    auto calcColl = proj->calculationCollection();

    for ( auto calc : calculations )
    {
        bool addDefaultExpression = false;
        auto summaryCalculation   = dynamic_cast<RimSummaryCalculation*>( calcColl->addCalculation( addDefaultExpression ) );
        if ( summaryCalculation )
        {
            summaryCalculation->setExpression( QString::fromStdString( calc.expression ) );
            summaryCalculation->setDescription( QString::fromStdString( calc.description ) );
            summaryCalculation->setUnit( QString::fromStdString( calc.unit ) );
            for ( auto var : calc.variables )
            {
                auto variable =
                    dynamic_cast<RimSummaryCalculationVariable*>( summaryCalculation->addVariable( QString::fromStdString( var.name ) ) );
                RifEclipseSummaryAddress address = RifEclipseSummaryAddress::fromEclipseTextAddress( var.address );

                RimSummaryAddress summaryAddress;
                summaryAddress.setAddress( address );
                variable->setSummaryAddress( summaryAddress );
                variable->setName( QString::fromStdString( var.name ) );
            }
        }
    }

    QString absPath = QFileInfo( fileName ).absolutePath();
    app->setLastUsedDialogDirectory( RicExportSummaryCalculationExpressionsFeature::summaryCalculationExpressionId(), absPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCalculationExpressionsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Import Grid Calculation Expressions" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
