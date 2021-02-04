/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicShowSummaryCurveCalculatorFeature.h"

#include "RicSummaryCurveCalculatorDialog.h"

#include "RiaGuiApplication.h"

#include "RimProject.h"
#include "RimSummaryCalculationCollection.h"

#include "RiuPlotMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowSummaryCurveCalculatorFeature, "RicShowSummaryCurveCalculatorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorDialog* RicShowSummaryCurveCalculatorFeature::curveCalculatorDialog()
{
    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();

    if ( mainPlotWindow )
    {
        return mainPlotWindow->summaryCurveCalculatorDialog();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowSummaryCurveCalculatorFeature::hideCurveCalculatorDialog()
{
    auto dialog = RicShowSummaryCurveCalculatorFeature::curveCalculatorDialog();

    dialog->hide();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowSummaryCurveCalculatorFeature::isCommandEnabled()
{
    RimProject* proj = RimProject::current();
    if ( !proj ) return false;

    const auto& allSumCases = proj->allSummaryCases();

    return !allSumCases.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowSummaryCurveCalculatorFeature::onActionTriggered( bool isChecked )
{
    RicSummaryCurveCalculatorDialog* dialog = RicShowSummaryCurveCalculatorFeature::curveCalculatorDialog();

    RimProject*                      proj     = RimProject::current();
    RimSummaryCalculationCollection* calcColl = proj->calculationCollection();
    if ( calcColl->textExpressionCalculations().size() == 0 )
    {
        calcColl->addCalculation();
    }

    dialog->setCalculationAndUpdateUi( calcColl->textExpressionCalculations()[0] );

    dialog->show();
    dialog->raise();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowSummaryCurveCalculatorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Curve Calculator" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
