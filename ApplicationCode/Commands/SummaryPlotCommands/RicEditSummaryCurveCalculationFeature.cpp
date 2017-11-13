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
#include "RicSummaryCurveCalculator.h"
#include "RicSummaryCurveCalculatorDialog.h"

#include "RimSummaryPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCase.h"
#include "RimCalculatedSummaryCurveReader.h"

#include "../../FileInterface/RifEclipseSummaryAddress.h"
#include "../../ProjectDataModel/RimProject.h"
#include "../../Application/RiaApplication.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicEditSummaryCurveCalculationFeature, "RicEditSummaryCurveCalculationFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEditSummaryCurveCalculationFeature::isCommandEnabled()
{
    std::vector<RimSummaryCurve*> selectedCurves = caf::selectedObjectsByType<RimSummaryCurve*>();
    return selectedCurves.size() == 1 && selectedCurves.front()->summaryAddress().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurveCalculationFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryCurve*> selectedCurves = caf::selectedObjectsByType<RimSummaryCurve*>();
    RimSummaryCalculation* calculation = nullptr;

    if (selectedCurves.size() > 0)
    {
        RifEclipseSummaryAddress selectedAddress = selectedCurves.front()->summaryAddress();

        RimProject* proj = RiaApplication::instance()->project();
        RifCalculatedSummaryCurveReader* reader = dynamic_cast<RifCalculatedSummaryCurveReader*>(proj->calculationCollection->calculationSummaryCase()->summaryReader());
        calculation = reader != nullptr ? reader->findCalculationByName(selectedAddress) : nullptr;
    }

    RicSummaryCurveCalculatorDialog* dialog = RicShowSummaryCurveCalculatorFeature::curveCalculatorDialog();
    dialog->setCalculationAndUpdateUi(calculation);
    dialog->show();
    dialog->raise();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEditSummaryCurveCalculationFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Edit Curve Calculation");
    actionToSetup->setIcon(QIcon(":/calculator.png"));
}
