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

#include "RigFemResultPosEnum.h"

#include "RimCase.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
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

        if (proj->multiSnapshotDefinitions.size() == 0)
        {
            dlg.addSnapshotItemFromActiveView();
        }

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
    
    for (RimMultiSnapshotDefinition* msd : project->multiSnapshotDefinitions())
    {
        RimView* rimView = msd->viewObject();
        if (!rimView) continue;
        if (!rimView->viewer()) continue;
        
        int initialFramIndex = rimView->viewer()->currentFrameIndex();

        exportViewVariationsToFolder(rimView, msd, folder);

        for (RimCase* rimCase : msd->additionalCases())
        {
            RimView* copyOfView = dynamic_cast<RimView*>(rimView->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));

            RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>(rimCase);
            if (eclCase)
            {
                RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(copyOfView);
                CVF_ASSERT(eclView);

                eclCase->reservoirViews().push_back(eclView);

                eclView->setEclipseCase(eclCase);

                // Resolve references after reservoir view has been inserted into Rim structures
                // Intersections referencing a well path/ simulation well requires this
                // TODO: initAfterReadRecursively can probably be removed
                eclView->initAfterReadRecursively();
                eclView->resolveReferencesRecursively();

                eclView->loadDataAndUpdate();

                exportViewVariationsToFolder(eclView, msd, folder);

                eclCase->reservoirViews().removeChildObject(eclView);
            }

            RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(rimCase);
            if (geomCase)
            {
                RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(copyOfView);
                CVF_ASSERT(geoMechView);

                geomCase->geoMechViews().push_back(geoMechView);

                geoMechView->setGeoMechCase(geomCase);

                // Resolve references after reservoir view has been inserted into Rim structures
                // Intersections referencing a well path/ simulation well requires this
                // TODO: initAfterReadRecursively can probably be removed
                geoMechView->initAfterReadRecursively();
                geoMechView->resolveReferencesRecursively();

                geoMechView->loadDataAndUpdate();

                exportViewVariationsToFolder(geoMechView, msd, folder);

                geomCase->geoMechViews().removeChildObject(geoMechView);
            }

            delete copyOfView;
        }

        // Set view back to initial state
        rimView->viewer()->setCurrentFrame(initialFramIndex);
        rimView->viewer()->animationControl()->setCurrentFrameOnly(initialFramIndex);

        rimView->scheduleCreateDisplayModelAndRedraw();
     }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportMultipleSnapshotsFeature::exportViewVariationsToFolder(RimView* rimView, RimMultiSnapshotDefinition* msd, const QString& folder)
{
    RimCase* rimCase = rimView->ownerCase();
    CVF_ASSERT(rimCase);

    RiuViewer* viewer = rimView->viewer();
    QStringList timeSteps = rimCase->timeStepStrings();

    QString resName = resultName(rimView);
    QString viewCaseResultString = rimCase->caseUserDescription() + "_" + rimView->name() + "_" + resName;

    for (int i = msd->timeStepStart(); i <= msd->timeStepEnd(); i++)
    {
        QString timeStepIndexString = QString("%1").arg(i, 2, 10, QLatin1Char('0'));

        QString timeStepString = timeStepIndexString + "_" + timeSteps[i].replace(".", "-");

        if (viewer)
        {
            viewer->setCurrentFrame(i);
            viewer->animationControl()->setCurrentFrameOnly(i);
        }

        if (msd->sliceDirection == RimMultiSnapshotDefinition::NO_RANGEFILTER)
        {
            QString fileName = viewCaseResultString + "_" + timeStepString;
            fileName.replace(" ", "-");
            QString absoluteFileName = caf::Utils::constructFullFileName(folder, fileName, ".png");

            QCoreApplication::instance()->processEvents();

            RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, rimView);
        }
        else
        {
            RimCellRangeFilter* rangeFilter = new RimCellRangeFilter;
            rimView->rangeFilterCollection()->rangeFilters.push_back(rangeFilter);

            bool rangeFilterInitState = rimView->rangeFilterCollection()->isActive();
            rimView->rangeFilterCollection()->isActive = true;

            for (int i = msd->startSliceIndex(); i <= msd->endSliceIndex(); i++)
            {
                QString rangeFilterString = msd->sliceDirection().text() + "-" + QString::number(i);
                QString fileName = viewCaseResultString + "_" + timeStepString + "_" + rangeFilterString;
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

                rimView->rangeFilterCollection()->updateDisplayModeNotifyManagedViews(rangeFilter);
                // Make sure the redraw is processed
                QCoreApplication::instance()->processEvents();

                QString absoluteFileName = caf::Utils::constructFullFileName(folder, fileName, ".png");
                RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, rimView);
            }

            rimView->rangeFilterCollection()->rangeFilters.removeChildObject(rangeFilter);
            delete rangeFilter;

            rimView->rangeFilterCollection()->isActive = rangeFilterInitState;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicExportMultipleSnapshotsFeature::resultName(RimView* rimView)
{
    if (dynamic_cast<RimEclipseView*>(rimView))
    {
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(rimView);

        return eclView->cellResult()->resultVariable();
    }
    else if (dynamic_cast<RimGeoMechView*>(rimView))
    {
        RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(rimView);

        RimGeoMechCellColors* cellResult = geoMechView->cellResult();

        if (cellResult)
        {
            QString legendTitle = caf::AppEnum<RigFemResultPosEnum>(cellResult->resultPositionType()).uiText() + "\n"
                + cellResult->resultFieldUiName();

            if (!cellResult->resultComponentUiName().isEmpty())
            {
                legendTitle += ", " + cellResult->resultComponentUiName();
            }

            return legendTitle;
        }
    }

    return "";
}

