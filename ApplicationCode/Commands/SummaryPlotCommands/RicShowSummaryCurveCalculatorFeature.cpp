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

#include "RimSummaryPlot.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicShowSummaryCurveCalculatorFeature, "RicShowSummaryCurveCalculatorFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowSummaryCurveCalculatorFeature::isCommandEnabled()
{
    if (selectedSummaryPlot()) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowSummaryCurveCalculatorFeature::onActionTriggered(bool isChecked)
{
    RicSummaryCurveCalculatorDialog dlg(nullptr);
    dlg.exec();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowSummaryCurveCalculatorFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Curve Calculator");
    actionToSetup->setIcon(QIcon(":/SummaryPlot16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicShowSummaryCurveCalculatorFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        selObj->firstAncestorOrThisOfType(sumPlot);
    }

    return sumPlot;
}
