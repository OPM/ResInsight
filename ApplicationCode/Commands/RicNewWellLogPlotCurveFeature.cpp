/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewWellLogPlotCurveFeature.h"

#include "RimWellLogPlotTrace.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT(RicNewWellLogPlotCurveFeature, "RicNewWellLogPlotCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogPlotCurveFeature::isCommandEnabled()
{
    return selectedWellLogPlotTrace() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogPlotTrace* wellLogPlotTrace = selectedWellLogPlotTrace();
    if (wellLogPlotTrace)
    {
        // TODO: replace dummy values with values extracted from model or read from well log files
        std::vector<double> depthValues, values;
        depthValues.push_back(0);
        depthValues.push_back(250);
        depthValues.push_back(1000);

        values.push_back(wellLogPlotTrace->curves.size() + 1);
        values.push_back(wellLogPlotTrace->curves.size() + 0.5);
        values.push_back(wellLogPlotTrace->curves.size() + 1);

        wellLogPlotTrace->addCurve(depthValues, values);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Curve");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrace* RicNewWellLogPlotCurveFeature::selectedWellLogPlotTrace()
{
    std::vector<RimWellLogPlotTrace*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}
