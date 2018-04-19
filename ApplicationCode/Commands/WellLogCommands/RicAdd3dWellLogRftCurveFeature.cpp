/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicAdd3dWellLogRftCurveFeature.h"

#include "RiaApplication.h"

#include "RicWellLogTools.h"

#include "Rim3dWellLogRftCurve.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAdd3dWellLogRftCurveFeature, "RicAdd3dWellLogRftCurveFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAdd3dWellLogRftCurveFeature::isCommandEnabled()
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);
    if (cases.empty()) return false;

    RimWellPath* wellPath = RicWellLogTools::findWellPathFromSelection();
    if (wellPath)
    {
        return RicWellLogTools::wellHasRftData(wellPath->name());
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdd3dWellLogRftCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellPath* selectedWellPath = RicWellLogTools::findWellPathFromSelection();
    if (!selectedWellPath) return;

    Rim3dWellLogRftCurve* rim3dWellLogRftCurve = new Rim3dWellLogRftCurve();
    selectedWellPath->add3dWellLogCurve(rim3dWellLogRftCurve);

    RiaApplication::instance()->project()->updateConnectedEditors();

    Riu3DMainWindowTools::selectAsCurrentItem(rim3dWellLogRftCurve);
    Riu3DMainWindowTools::setExpanded(selectedWellPath);

    selectedWellPath->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdd3dWellLogRftCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
    actionToSetup->setText("Add 3D Well Log RFT Curve");
}
