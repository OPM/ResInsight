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

        exportResultVariations(rimView, msd, folder);

        for (RimCase* rimCase : msd->additionalCases())
        {
            RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>(rimCase);
            RimEclipseView* sourceEclipseView = dynamic_cast<RimEclipseView*>(rimView);
            if (eclCase && sourceEclipseView)
            {
                RimEclipseView* copyOfEclipseView = eclCase->createCopyAndAddView(sourceEclipseView);
                CVF_ASSERT(copyOfEclipseView);

                copyOfEclipseView->loadDataAndUpdate();

                exportResultVariations(copyOfEclipseView, msd, folder);

                eclCase->reservoirViews().removeChildObject(copyOfEclipseView);
                
                delete copyOfEclipseView;
            }

            RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(rimCase);
            RimGeoMechView* sourceGeoMechView = dynamic_cast<RimGeoMechView*>(rimView);
            if (geomCase && sourceGeoMechView)
            {
                RimGeoMechView* copyOfGeoMechView = dynamic_cast<RimGeoMechView*>(sourceGeoMechView->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
                CVF_ASSERT(copyOfGeoMechView);

                geomCase->geoMechViews().push_back(copyOfGeoMechView);

                copyOfGeoMechView->setGeoMechCase(geomCase);

                // Resolve references after reservoir view has been inserted into Rim structures
                // Intersections referencing a well path/ simulation well requires this
                // TODO: initAfterReadRecursively can probably be removed
                copyOfGeoMechView->initAfterReadRecursively();
                copyOfGeoMechView->resolveReferencesRecursively();

                copyOfGeoMechView->loadDataAndUpdate();

                exportResultVariations(copyOfGeoMechView, msd, folder);

                geomCase->geoMechViews().removeChildObject(copyOfGeoMechView);
            
                delete copyOfGeoMechView;
            }
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
void RicExportMultipleSnapshotsFeature::exportResultVariations(RimView* rimView, RimMultiSnapshotDefinition* msd, const QString& folder)
{
    exportViewVariationsToFolder(rimView, msd, folder);

    if (msd->selectedEclipseResults().size() > 0)
    {
        RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>(rimView->ownerCase());
        
        RimEclipseView* copyOfView = eclCase->createCopyAndAddView(dynamic_cast<RimEclipseView*>(rimView));

        copyOfView->cellResult()->setResultType(msd->eclipseResultType());

        for (QString s : msd->selectedEclipseResults())
        {
            copyOfView->cellResult()->setResultVariable(s);

            copyOfView->loadDataAndUpdate();

            exportViewVariationsToFolder(copyOfView, msd, folder);
        }

        eclCase->reservoirViews().removeChildObject(copyOfView);
        
        delete copyOfView;
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
    viewCaseResultString = caf::Utils::makeValidFileBasename(viewCaseResultString);

    for (int i = msd->timeStepStart(); i <= msd->timeStepEnd(); i++)
    {
        QString timeStepIndexString = QString("%1").arg(i, 2, 10, QLatin1Char('0'));

        QString timeStepString = timeStepIndexString + "_" + timeSteps[i].replace(".", "-");

        if (viewer)
        {
            // Force update of scheduled display models modifying the time step
            // This is required due to visualization structures updated by the update functions,
            // and this is not triggered by changing time step only
            RiaApplication::instance()->slotUpdateScheduledDisplayModels();

            viewer->setCurrentFrame(i);
            viewer->animationControl()->setCurrentFrameOnly(i);
        }

        if (msd->sliceDirection == RimMultiSnapshotDefinition::NO_RANGEFILTER)
        {
            QString fileName = viewCaseResultString + "_" + timeStepString;
            QString absoluteFileName = caf::Utils::constructFullFileName(folder, fileName, ".png");

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
            QString title = caf::AppEnum<RigFemResultPosEnum>(cellResult->resultPositionType()).uiText() + "_"
                + cellResult->resultFieldUiName();

            if (!cellResult->resultComponentUiName().isEmpty())
            {
                title += "_" + cellResult->resultComponentUiName();
            }

            return title;
        }
    }

    return "";
}

