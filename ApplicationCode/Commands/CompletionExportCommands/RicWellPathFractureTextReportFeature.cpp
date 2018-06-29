/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RicWellPathFractureTextReportFeature.h"
#include "RicWellPathFractureTextReportFeatureImpl.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RiuTextDialog.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicWellPathFractureTextReportFeature, "RicWellPathFractureTextReportFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathFractureTextReportFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureTextReportFeature::onActionTriggered(bool isChecked)
{
    RimEclipseCase* eclipseCase = nullptr;
    {
        RimProject* proj = RiaApplication::instance()->project();

        std::vector<RimCase*> cases;
        proj->allCases(cases);
        for (auto c : cases)
        {
            if (dynamic_cast<RimEclipseCase*>(c))
            {
                eclipseCase = dynamic_cast<RimEclipseCase*>(c);
                break;
            }
        }
    }

    auto wellPaths = RicWellPathFractureTextReportFeatureImpl::wellPathsWithFractures();

    RicWellPathFractureTextReportFeatureImpl impl;

    QString reportText = impl.wellPathFractureReport(eclipseCase, wellPaths);

    RiuTextDialog* textDialog = new RiuTextDialog(nullptr);
    textDialog->resize(QSize(1000, 1000));
    textDialog->setWindowTitle("Fracture Report");
    textDialog->setText(reportText);
    textDialog->show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFractureTextReportFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Fracture Completion Header for Well Paths");
}
