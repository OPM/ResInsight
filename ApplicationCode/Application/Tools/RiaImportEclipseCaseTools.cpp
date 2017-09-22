/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RiaImportEclipseCaseTools.h"

#include "../SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigGridManager.h"

#include "RimCaseCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"

#include "cafUtils.h"
#include "cafProgressInfo.h"

#include <QFileInfo>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEclipseCaseFromFile(const QString& fileName)
{
    if (!caf::Utils::fileExists(fileName)) return false;

    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl(fileName, false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilter(const QString& fileName)
{
    if (!caf::Utils::fileExists(fileName)) return false;

    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl(fileName, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openMockModel(const QString& name)
{
    return RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl(name, false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilterImpl(const QString& fileName, bool showTimeStepFilter)
{
    QFileInfo gridFileName(fileName);
    QString caseName = gridFileName.completeBaseName();

    RimEclipseResultCase* rimResultReservoir = new RimEclipseResultCase();
    rimResultReservoir->setCaseInfo(caseName, fileName);

    RiaApplication* app = RiaApplication::instance();
    RimProject* project = app->project();

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : NULL;
    if (analysisModels == NULL) return false;

    RiuMainWindow::instance()->show();

    analysisModels->cases.push_back(rimResultReservoir);

    if (!rimResultReservoir->importGridAndResultMetaData(showTimeStepFilter))
    {
        analysisModels->removeCaseFromAllGroups(rimResultReservoir);

        delete rimResultReservoir;

        return false;
    }

    RimEclipseView* riv = rimResultReservoir->createAndAddReservoirView();

    // Select SOIL as default result variable
    riv->cellResult()->setResultType(RiaDefines::DYNAMIC_NATIVE);

    if (app->preferences()->loadAndShowSoil)
    {
        riv->cellResult()->setResultVariable("SOIL");
    }
    riv->hasUserRequestedAnimation = true;

    riv->loadDataAndUpdate();

    // Add a corresponding summary case if it exists
    {
        RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : NULL;
        if (sumCaseColl)
        {
            {
                RiuMainPlotWindow* mainPlotWindow = app->mainPlotWindow();
                if (sumCaseColl->summaryCaseCount() == 0 && mainPlotWindow)
                {
                    mainPlotWindow->hide();
                }
            }

            if (!sumCaseColl->findSummaryCaseFromEclipseResultCase(rimResultReservoir))
            {
                RimSummaryCase* newSumCase = sumCaseColl->createAndAddSummaryCaseFromEclipseResultCase(rimResultReservoir);

                if (newSumCase)
                {
                    RimSummaryCase* existingFileSummaryCase = sumCaseColl->findSummaryCaseFromFileName(newSumCase->summaryHeaderFilename());
                    if (existingFileSummaryCase)
                    {
                        // Replace all occurrences of file sum with ecl sum

                        std::vector<caf::PdmObjectHandle*> referringObjects;
                        existingFileSummaryCase->objectsWithReferringPtrFields(referringObjects);

                        std::set<RimSummaryCurveFilter*> curveFilters;

                        for (caf::PdmObjectHandle* objHandle : referringObjects)
                        {
                            RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>(objHandle);
                            if (summaryCurve)
                            {
                                //TODO: When removing curve filter functionality, move this to summaryCurveCollection
                                //loop and update "if (parentCollection)"-block
                                summaryCurve->setSummaryCase(newSumCase);
                                summaryCurve->updateConnectedEditors();

                                RimSummaryCurveFilter* parentFilter = nullptr;
                                summaryCurve->firstAncestorOrThisOfType(parentFilter);
                                if (parentFilter)
                                {
                                    curveFilters.insert(parentFilter);
                                }
                            }
                        }

                        // UI settings of a curve filter is updated based
                        // on the new case association for the curves in the curve filter
                        // UI is updated by loadDataAndUpdate()

                        for (RimSummaryCurveFilter* curveFilter : curveFilters)
                        {
                            curveFilter->loadDataAndUpdate();
                            curveFilter->updateConnectedEditors();
                        }

                        for (caf::PdmObjectHandle* objHandle : referringObjects)
                        {
                            RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>(objHandle);
                            if (summaryCurve)
                            {
                                RimSummaryCurveCollection* parentCollection = nullptr;
                                summaryCurve->firstAncestorOrThisOfType(parentCollection);
                                if (parentCollection)
                                {
                                    parentCollection->loadDataAndUpdate();
                                    parentCollection->updateConnectedEditors();
                                    break;
                                }
                            }
                        }

                        sumCaseColl->removeCase(existingFileSummaryCase);

                        delete existingFileSummaryCase;

                    }
                    else
                    {
                        if (app->preferences()->autoCreatePlotsOnImport())
                        {
                            RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
                            RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();

                            RicNewSummaryPlotFeature::createNewSummaryPlot(summaryPlotColl, newSumCase);
                        }
                    }

                    sumCaseColl->updateConnectedEditors();
                }
            }
        }
    }

    if (!riv->cellResult()->hasResult())
    {
        riv->cellResult()->setResultVariable(RiaDefines::undefinedResultName());
    }

    analysisModels->updateConnectedEditors();

    RiuMainWindow::instance()->selectAsCurrentItem(riv->cellResult());

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaImportEclipseCaseTools::addEclipseCases(const QStringList& fileNames)
{
    if (fileNames.size() == 0) return true;

    // First file is read completely including grid.
    // The main grid from the first case is reused directly in for the other cases. 
    // When reading active cell info, only the total cell count is tested for consistency
    RimEclipseResultCase* mainResultCase = NULL;
    std::vector< std::vector<int> > mainCaseGridDimensions;
    RimIdenticalGridCaseGroup* gridCaseGroup = NULL;

    RiaApplication* app = RiaApplication::instance();
    RimProject* project = app->project();

    {
        QString firstFileName = fileNames[0];
        QFileInfo gridFileName(firstFileName);

        QString caseName = gridFileName.completeBaseName();

        RimEclipseResultCase* rimResultReservoir = new RimEclipseResultCase();
        rimResultReservoir->setCaseInfo(caseName, firstFileName);
        if (!rimResultReservoir->openEclipseGridFile())
        {
            delete rimResultReservoir;

            return false;
        }

        rimResultReservoir->readGridDimensions(mainCaseGridDimensions);

        mainResultCase = rimResultReservoir;
        RimOilField* oilField = project->activeOilField();
        if (oilField && oilField->analysisModels())
        {
            gridCaseGroup = oilField->analysisModels->createIdenticalCaseGroupFromMainCase(mainResultCase);
        }
    }

    caf::ProgressInfo info(fileNames.size(), "Reading Active Cell data");

    for (int i = 1; i < fileNames.size(); i++)
    {
        QString caseFileName = fileNames[i];
        QFileInfo gridFileName(caseFileName);

        QString caseName = gridFileName.completeBaseName();

        RimEclipseResultCase* rimResultReservoir = new RimEclipseResultCase();
        rimResultReservoir->setCaseInfo(caseName, caseFileName);

        std::vector< std::vector<int> > caseGridDimensions;
        rimResultReservoir->readGridDimensions(caseGridDimensions);

        bool identicalGrid = RigGridManager::isGridDimensionsEqual(mainCaseGridDimensions, caseGridDimensions);
        if (identicalGrid)
        {
            if (rimResultReservoir->openAndReadActiveCellData(mainResultCase->eclipseCaseData()))
            {
                RimOilField* oilField = project->activeOilField();
                if (oilField && oilField->analysisModels())
                {
                    oilField->analysisModels()->insertCaseInCaseGroup(gridCaseGroup, rimResultReservoir);
                }
            }
            else
            {
                delete rimResultReservoir;
            }
        }
        else
        {
            delete rimResultReservoir;
        }

        info.setProgress(i);
    }

    if (gridCaseGroup)
    {
        // Create placeholder results and propagate results info from main case to all other cases 
        gridCaseGroup->loadMainCaseAndActiveCellInfo();
    }

    project->activeOilField()->analysisModels()->updateConnectedEditors();

    if (gridCaseGroup->statisticsCaseCollection()->reservoirs.size() > 0)
    {
        RiuMainWindow::instance()->selectAsCurrentItem(gridCaseGroup->statisticsCaseCollection()->reservoirs[0]);
    }

    return true;
}

