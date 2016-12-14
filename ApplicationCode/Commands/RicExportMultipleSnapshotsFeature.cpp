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
#include "RimMultiSnapshotDefinition.h"
#include "RimProject.h"
#include "RimView.h"
#include "RiuExportMultipleSnapshotsWidget.h"

#include "cafCmdExecCommandManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include "QDir"


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
    if (!project) return; //Er dette OK syntax??

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

            for (int i=msd->timeStepStart(); i <= msd->timeStepEnd(); i++) 
            {
                timeStepString = timeSteps[i].replace(".", "-");

                if (msd->sliceDirection == RimMultiSnapshotDefinition::NO_RANGEFILTER)
                {
                    QString fileName = activeCase->caseUserDescription() + "_" + activeView->name() + "_" + timeStepString + ".png";
                    fileName.replace(" ", "-");

                    //TODO: Before saveSnapshotAs is called the folder name must be changed from dummy value in widget
                    QString absoluteFileName = caf::Utils::constructFullFileName(folder, fileName, ".png");
                    //RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, activeView);

                    qDebug() << absoluteFileName;                    

                }
                else
                {
                    for (int i = msd->startSliceIndex(); i <= msd->endSliceIndex(); i++)  
                    {
                        rangeFilterString = msd->sliceDirection().text() + QString::number(i);

                        //             setActiveReservoirView(riv);
                        //             RiuViewer* viewer = riv->viewer();
                        //             mainWnd->setActiveViewer(viewer->layoutWidget());
                        //             clearViewsScheduledForUpdate();
                        //             //riv->updateCurrentTimeStepAndRedraw();    
                        //             riv->createDisplayModelAndRedraw();
                        //             viewer->repaint();
                        //            for (int i=msd->timeStepStart(); i<msd->timeStepEnd(); i++)


                        QString fileName = activeCase->caseUserDescription() + "_" + activeView->name() + "_" + timeStepString + "_" + rangeFilterString + ".png";
                        fileName.replace(" ", "-");

                        //TODO: Before saveSnapshotAs is called the folder name must be changed from dummy value in widget
                        QString absoluteFileName = caf::Utils::constructFullFileName(folder, fileName, ".png");
                        //RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, activeView);

                        qDebug() << absoluteFileName;


                    }

                }

            }
        }
    }

}

