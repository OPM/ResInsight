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

#include "RicExportMultipleSnapshotsFeature.h"

#include "RiaApplication.h"

#include "RicSnapshotViewToClipboardFeature.h"

#include "RimCase.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimMultiSnapshotDefinition.h"
#include "RimProject.h"
#include "RimView.h"

#include "RiuExportMultipleSnapshotsWidget.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafFrameAnimationControl.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QDir>


CAF_CMD_SOURCE_INIT(RicExportMultipleSnapshotsFeature, "RicExportMultipleSnapshotsFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportMultipleSnapshotsFeature::isCommandEnabled()
{
    RimProject* proj = RiaApplication::instance()->project();
    
    return proj;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSnapshotsFeature::onActionTriggered(bool isChecked)
{
    RimProject* proj = RiaApplication::instance()->project();

    if (proj)
    {
        // Enable the command system to be able to assign a value to multiple fields at the same time
        caf::CmdExecCommandSystemActivator activator;

        RiuExportMultipleSnapshotsWidget dlg(nullptr, proj);
        dlg.exec();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSnapshotsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Multiple Snapshots ...");
    //actionToSetup->setIcon(QIcon(":/Save.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSnapshotsFeature::exportMultipleSnapshots(const QString& folder, RimProject* project)
{
    if (!project) return;

    QDir snapshotPath(folder);
    if (!snapshotPath.exists())
    {
        if (!snapshotPath.mkpath(".")) return;
    }

    RimCase* activeCase = nullptr;
    RimView* activeView = nullptr;
    
    QStringList timeSteps;
    QString timeStepString;
    QString rangeFilterString;

    for (RimMultiSnapshotDefinition* msd : project->multiSnapshotDefinitions())
    {

        activeCase = msd->caseObject();
        if (!activeCase) continue;

        activeView = msd->viewObject();

        if (activeView && activeView->viewer())
        {
            timeSteps = activeCase->timeStepStrings(); 
            RiuViewer* viewer = activeView->viewer();
            int initialFramIndex = viewer->currentFrameIndex();

            for (int i=msd->timeStepStart(); i <= msd->timeStepEnd(); i++) 
            {
                //TODO: This naming will not give images in the correct order when sorted on name..
                timeStepString = timeSteps[i].replace(".", "-");
                
                viewer->setCurrentFrame(i);
                viewer->animationControl()->setCurrentFrameOnly(i);

                if (msd->sliceDirection == RimMultiSnapshotDefinition::NO_RANGEFILTER)
                {
                    QString fileName = activeCase->caseUserDescription() + "_" + activeView->name() + "_" + timeStepString;
                    fileName.replace(" ", "-");
                    QString absoluteFileName = caf::Utils::constructFullFileName(folder, fileName, ".png");
                    RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, activeView);
                }
                else
                {
                    RimCellRangeFilter* rangeFilter = new RimCellRangeFilter;
                    activeView->rangeFilterCollection()->rangeFilters.push_back(rangeFilter);

                    bool rangeFilterInitState =  activeView->rangeFilterCollection()->isActive();
                    activeView->rangeFilterCollection()->isActive = true;

                    for (int i = msd->startSliceIndex(); i <= msd->endSliceIndex(); i++)  
                    {
                        rangeFilterString = msd->sliceDirection().text() + QString::number(i);
                        QString fileName = activeCase->caseUserDescription() + "_" + activeView->name() + "_" + timeStepString + "_" + rangeFilterString;
                        fileName.replace(" ", "-");

                        rangeFilter->setDefaultValues();
                        if (msd->sliceDirection == RimMultiSnapshotDefinition::RANGEFILTER_I)
                        {
                            rangeFilter->cellCountI = 1;
                            rangeFilter->startIndexI = i;
                        }
                        else if (msd->sliceDirection == RimMultiSnapshotDefinition::RANGEFILTER_J)
                        {
                            rangeFilter->cellCountJ = 1;
                            rangeFilter->startIndexJ = i;
                        }
                        else if (msd->sliceDirection == RimMultiSnapshotDefinition::RANGEFILTER_K)
                        {
                            rangeFilter->cellCountK = 1;
                            rangeFilter->startIndexK = i;
                        }

                        activeView->rangeFilterCollection()->updateDisplayModeNotifyManagedViews(rangeFilter);
                        // Make sure the redraw is processed
                        QCoreApplication::instance()->processEvents();

                        QString absoluteFileName = caf::Utils::constructFullFileName(folder, fileName, ".png");
                        RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, activeView);
                    }

                    activeView->rangeFilterCollection()->rangeFilters.removeChildObject(rangeFilter);
                    delete rangeFilter;

                    activeView->rangeFilterCollection()->isActive = rangeFilterInitState;
                    activeView->scheduleCreateDisplayModelAndRedraw();
                    QCoreApplication::instance()->processEvents();
                }
            }

            viewer->setCurrentFrame(initialFramIndex);
            viewer->animationControl()->setCurrentFrameOnly(initialFramIndex);

        }
    }

}

