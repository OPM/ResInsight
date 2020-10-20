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

#include "RicEditSummaryCurveCalculationFeature.h"

#include "RicShowSummaryCurveCalculatorFeature.h"
#include "RicSummaryCurveCalculatorDialog.h"
#include "RicSummaryCurveCalculatorUi.h"

#include "RifEclipseSummaryAddress.h"

#include "RimProject.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEditSummaryCurveCalculationFeature, "RicEditSummaryCurveCalculationFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEditSummaryCurveCalculationFeature::isCommandEnabled()
{
    std::vector<RimSummaryCurve*> selectedCurves = caf::selectedObjectsByType<RimSummaryCurve*>();
    return selectedCurves.size() == 1 &&
           selectedCurves.front()->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurveCalculationFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryCurve*> selectedCurves = caf::selectedObjectsByType<RimSummaryCurve*>();
    RimSummaryCalculation*        calculation    = nullptr;

    if ( selectedCurves.size() > 0 )
    {
        RifEclipseSummaryAddress selectedAddress = selectedCurves.front()->summaryAddressY();

        RimProject*                      proj            = RimProject::current();
        RimSummaryCalculationCollection* calculationColl = proj->calculationCollection();

        if ( calculationColl )
        {
            calculation = calculationColl->findCalculationById( selectedAddress.id() );
        }
    }

    RicSummaryCurveCalculatorDialog* dialog = RicShowSummaryCurveCalculatorFeature::curveCalculatorDialog();
    dialog->setCalculationAndUpdateUi( calculation );
    dialog->show();
    dialog->raise();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurveCalculationFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Edit Curve Calculation" );
    actionToSetup->setIcon( QIcon( ":/Calculator.svg" ) );
}
