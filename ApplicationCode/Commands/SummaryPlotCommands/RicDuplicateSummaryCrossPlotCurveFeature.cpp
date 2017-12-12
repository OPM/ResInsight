/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicDuplicateSummaryCrossPlotCurveFeature.h"

#include "RiaApplication.h"

#include "RicPasteSummaryCurveFeature.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RiaSummaryTools.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"

#include "RiuMainPlotWindow.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicDuplicateSummaryCrossPlotCurveFeature, "RicDuplicateSummaryCrossPlotCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDuplicateSummaryCrossPlotCurveFeature::isCommandEnabled()
{
    RimSummaryCrossPlot* selectedPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCrossPlot*>();
    return (selectedPlot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDuplicateSummaryCrossPlotCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Duplicate Summary Cross Plot Curve");
    actionToSetup->setIcon(QIcon(":/SummaryCurve16x16.png"));
}
