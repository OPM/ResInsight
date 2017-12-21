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

#include "RicWellPathImportPerforationIntervalsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimPerforationInterval.h"
#include "RimPerforationCollection.h"

#include "RifPerforationIntervalReader.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicWellPathImportPerforationIntervalsFeature, "RicWellPathImportPerforationIntervalsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathImportPerforationIntervalsFeature::isCommandEnabled()
{
    if (RicWellPathImportPerforationIntervalsFeature::selectedWellPathCollection() != nullptr)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathImportPerforationIntervalsFeature::onActionTriggered(bool isChecked)
{
    RimWellPathCollection* wellPathCollection = RicWellPathImportPerforationIntervalsFeature::selectedWellPathCollection();
    CVF_ASSERT(wellPathCollection);

    // Open dialog box to select well path files
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("WELLPATH_DIR");
    QStringList wellPathFilePaths = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Import Well Path Perforation Intervals", defaultDir, "Well Path Perforation Intervals (*.ev);;All Files (*.*)");

    if (wellPathFilePaths.size() < 1) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("WELLPATH_DIR", QFileInfo(wellPathFilePaths.last()).absolutePath());

    std::map<QString, std::vector<RifPerforationInterval> > perforationIntervals = RifPerforationIntervalReader::readPerforationIntervals(wellPathFilePaths);

    for (auto& entry : perforationIntervals)
    {
        RimWellPath* wellPath = wellPathCollection->wellPathByName(entry.first);
        if (wellPath == nullptr)
        {
            RiaLogging::warning(QString("Import Well Path Perforation Intervals : Imported file contains unknown well path '%1'.").arg(entry.first));
        }
        else
        {
            for (auto& interval : entry.second)
            {
                RimPerforationInterval* perforationInterval = new RimPerforationInterval;
                perforationInterval->setStartAndEndMD(interval.startMD, interval.endMD);
                perforationInterval->setDiameter(interval.diameter);
                perforationInterval->setSkinFactor(interval.skinFactor);
                if (interval.startOfHistory)
                {
                    perforationInterval->setStartOfHistory();
                }
                else
                {
                    perforationInterval->setDate(interval.date);
                }
                wellPath->perforationIntervalCollection()->appendPerforation(perforationInterval);
            }
        }
    }

    if (app->project())
    {
        app->project()->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathImportPerforationIntervalsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Import Perforation Intervals");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RicWellPathImportPerforationIntervalsFeature::selectedWellPathCollection()
{
    std::vector<RimWellPathCollection*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() == 1)
    {
        return objects[0];
    }

    return nullptr;
}
