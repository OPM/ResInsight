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

#include "RicDeleteTemporaryLgrsFeature.h"

#include "RiaGuiApplication.h"

#include "RimEclipseCase.h"
#include "RimGridCollection.h"
#include "RimReloadCaseTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeleteTemporaryLgrsFeature, "RicDeleteTemporaryLgrsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteTemporaryLgrsFeature::deleteAllTemporaryLgrs(RimEclipseCase* eclipseCase)
{
    RiaGuiApplication::clearAllSelections();

    if (eclipseCase)
    {
        RimReloadCaseTools::reloadAllEclipseGridData(eclipseCase);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteTemporaryLgrsFeature::isCommandEnabled()
{
    std::vector<RimGridInfoCollection*> selGridInfos = caf::selectedObjectsByTypeStrict<RimGridInfoCollection*>();
    if (selGridInfos.size() == 1 && selGridInfos.front()->uiName() == RimGridCollection::temporaryGridUiName())
    {
        return !selGridInfos[0]->gridInfos().empty();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteTemporaryLgrsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimGridInfoCollection*> selGridInfos = caf::selectedObjectsByTypeStrict<RimGridInfoCollection*>();
    if (selGridInfos.size() == 1 && selGridInfos.front()->uiName() == RimGridCollection::temporaryGridUiName())
    {
        RimEclipseCase* eclipseCase;
        selGridInfos.front()->firstAncestorOrThisOfType(eclipseCase);

        deleteAllTemporaryLgrs(eclipseCase);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteTemporaryLgrsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Temporary LGRs");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
    applyShortcutWithHintToAction(actionToSetup, QKeySequence::Delete);
}
