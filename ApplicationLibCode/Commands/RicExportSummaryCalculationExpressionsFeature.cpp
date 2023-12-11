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

#include "RicExportSummaryCalculationExpressionsFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifSummaryCalculation.h"
#include "RifSummaryCalculationExporter.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"

#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicExportSummaryCalculationExpressionsFeature, "RicExportSummaryCalculationExpressionsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSummaryCalculationExpressionsFeature::summaryCalculationExpressionId()
{
    return "SUMMARY_CALCULATION_EXPRESSION";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSummaryCalculationExpressionsFeature::onActionTriggered( bool isChecked )
{
    auto proj     = RimProject::current();
    auto calcColl = proj->calculationCollection();

    if ( calcColl->calculations().empty() ) return;

    QString fallbackPath = RiaPreferences::current()->summaryCalculationExpressionFolder();
    auto    app          = RiaGuiApplication::instance();
    QString startPath =
        app->lastUsedDialogDirectoryWithFallback( RicExportSummaryCalculationExpressionsFeature::summaryCalculationExpressionId(),
                                                  fallbackPath );

    QString fileName = RiuFileDialogTools::getSaveFileName( nullptr,
                                                            "Select File for Summary Calculation Expression Export",
                                                            startPath,
                                                            "Toml File(*.toml);;All files(*.*)" );
    if ( fileName.isEmpty() ) return;

    auto fi = QFileInfo( fileName );
    if ( fi.suffix().isEmpty() )
    {
        fileName += ".toml";
    }
    QString absPath = fi.absolutePath();

    app->setLastUsedDialogDirectory( RicExportSummaryCalculationExpressionsFeature::summaryCalculationExpressionId(), absPath );

    std::vector<RifSummaryCalculation> calculations;
    for ( auto calculation : calcColl->calculations() )
    {
        if ( auto summaryCalculation = dynamic_cast<RimSummaryCalculation*>( calculation ) )
        {
            RifSummaryCalculation calc;
            calc.expression           = calculation->expression().toStdString();
            calc.unit                 = calculation->unitName().toStdString();
            calc.distributeToAllCases = summaryCalculation->isDistributeToAllCases();
            calc.distributeToOther    = summaryCalculation->isDistributeToOtherItems();

            for ( auto variable : calculation->allVariables() )
            {
                if ( auto gridVariable = dynamic_cast<RimSummaryCalculationVariable*>( variable ) )
                {
                    RifSummaryCalculationVariable var;
                    var.address = gridVariable->summaryAddress()->address().toEclipseTextAddress();
                    var.name    = gridVariable->name().toStdString();
                    calc.variables.push_back( var );
                }
            }

            calculations.push_back( calc );
        }
    }

    auto [isOk, errorMessage] = RifSummaryCalculationExporter::writeToFile( calculations, fileName.toStdString() );
    if ( !isOk )
    {
        RiaLogging::errorInMessageBox( RiuPlotMainWindow::instance(), "Summary Calculation Export Error", QString::fromStdString( errorMessage ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSummaryCalculationExpressionsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Summary Calculation Expressions" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
