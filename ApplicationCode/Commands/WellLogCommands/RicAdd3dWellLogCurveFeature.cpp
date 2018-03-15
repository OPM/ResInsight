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

#include "RicAdd3dWellLogCurveFeature.h"

#include "RiaApplication.h"

#include "RicWellLogTools.h"

#include "Rim3dWellLogExtractionCurve.h"
#include "RimCase.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAdd3dWellLogCurveFeature, "RicAdd3dWellLogCurveFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAdd3dWellLogCurveFeature::isCommandEnabled()
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);
    if (cases.empty()) return false;

    return (RicWellLogTools::selectedWellPath() != nullptr);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdd3dWellLogCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellPath* selectedWellPath = RicWellLogTools::selectedWellPath();

    Rim3dWellLogExtractionCurve* rim3dWellLogExtractionCurve = new Rim3dWellLogExtractionCurve();
    
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if (view)
    {
        rim3dWellLogExtractionCurve->setPropertiesFromView(view);
    }
    
    selectedWellPath->add3dWellLogCurve(rim3dWellLogExtractionCurve);
    
    RiaApplication::instance()->project()->updateConnectedEditors();

    Riu3DMainWindowTools::selectAsCurrentItem(rim3dWellLogExtractionCurve);
    Riu3DMainWindowTools::setExpanded(selectedWellPath);

    selectedWellPath->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAdd3dWellLogCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
    actionToSetup->setText("Add 3D Well Log Curve");
}
