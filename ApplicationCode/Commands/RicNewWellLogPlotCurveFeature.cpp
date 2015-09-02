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
#include "RiuMainWindow.h"
#include "RimWellLogPlotCurve.h"

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
        RimWellLogPlotCurve* curve = new RimWellLogEclipseCurve();
        wellLogPlotTrace->addCurve(curve);

        curve->setUiName(QString("Curve %1").arg(wellLogPlotTrace->curves.size()));

        wellLogPlotTrace->updateConnectedEditors();
        RiuMainWindow::instance()->setCurrentObjectInTreeView(curve);
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
