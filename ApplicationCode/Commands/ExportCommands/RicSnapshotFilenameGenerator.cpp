/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicSnapshotFilenameGenerator.h"

#include "RimViewWindow.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"
#include "RimGeoMechView.h"
#include "RimGeoMechCellColors.h"

#include "cafUtils.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSnapshotFilenameGenerator::generateSnapshotFileName(RimViewWindow* viewWindow)
{
    {
        Rim3dView* view = dynamic_cast<Rim3dView*>(viewWindow);
        if (view != nullptr)
        {
            return generateSnapshotFilenameForRimView(view);
        }
    }
    QString fileName;
    if (viewWindow->userDescriptionField())
    {
        fileName = viewWindow->userDescriptionField()->uiCapability()->uiValue().toString();
    }
    else
    {
        fileName = viewWindow->uiCapability()->uiName();
    }

    fileName = caf::Utils::makeValidFileBasename(fileName);

    return fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSnapshotFilenameGenerator::generateSnapshotFilenameForRimView(Rim3dView* rimView)
{
    QStringList timeSteps = rimView->ownerCase()->timeStepStrings();
    int timeStep = rimView->currentTimeStep();

    QString fileName = QString("%1_%2_%3").arg(rimView->ownerCase()->caseUserDescription())
                                          .arg(rimView->name())
                                          .arg(resultName(rimView));

    if ( !timeSteps.empty() ) fileName += QString("_%1_%2").arg(timeStep, 2, 10, QChar('0'))
                                                           .arg(timeSteps[timeStep]);

    fileName.replace("-", "_");

    fileName = caf::Utils::makeValidFileBasename(fileName);

    return fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSnapshotFilenameGenerator::resultName(Rim3dView * rimView)
{
    if (dynamic_cast<RimEclipseView*>(rimView))
    {
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(rimView);
        
        return caf::Utils::makeValidFileBasename(eclView->cellResult()->resultVariableUiShortName());
    }
    else if (dynamic_cast<RimGeoMechView*>(rimView))
    {
        RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(rimView);

        RimGeoMechCellColors* cellResult = geoMechView->cellResult();

        if (cellResult)
        {
            QString title = QString("%1_%2").arg(caf::AppEnum<RigFemResultPosEnum>(cellResult->resultPositionType()).uiText())
                                            .arg(cellResult->resultFieldUiName());

            if (!cellResult->resultComponentUiName().isEmpty())
            {
                title += "_" + cellResult->resultComponentUiName();
            }

            return title;
        }
    }

    return "";
}
