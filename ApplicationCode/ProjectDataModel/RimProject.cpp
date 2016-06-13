/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimProject.h"

#include "RiaApplication.h"
#include "RiaVersionInfo.h"

#include "RigCaseData.h"

#include "RimCalcScript.h"
#include "RimCase.h"
#include "RimCaseCollection.h"
#include "RimCommandObject.h"
#include "RimContextCommandBuilder.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimScriptCollection.h"
#include "RimSummaryPlotCollection.h"
#include "RimView.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathImport.h"

#include "RiuMainWindow.h"

#include "OctaveScriptCommands/RicExecuteScriptForCasesFeature.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmUiTreeOrdering.h"

#include <QDir>
#include <QMenu>
#include "RimGridSummaryCase.h"
#include "RimSummaryCaseCollection.h"


CAF_PDM_SOURCE_INIT(RimProject, "ResInsightProject");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimProject::RimProject(void)
{
    CAF_PDM_InitFieldNoDefault(&m_projectFileVersionString, "ProjectFileVersionString", "", "", "", "");
    m_projectFileVersionString.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&nextValidCaseId, "NextValidCaseId", 0, "Next Valid Case ID", "", "" ,"");
    nextValidCaseId.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&nextValidCaseGroupId, "NextValidCaseGroupId", 0, "Next Valid Case Group ID", "", "" ,"");
    nextValidCaseGroupId.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&oilFields, "OilFields", "Oil Fields",  "", "", "");
    oilFields.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&scriptCollection, "ScriptCollection", "Scripts", ":/Default.png", "", "");
    scriptCollection.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&treeViewState, "TreeViewState", "",  "", "", "");
    treeViewState.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&wellPathImport, "WellPathImport", "WellPathImport", "", "", "");
    wellPathImport = new RimWellPathImport();
    wellPathImport.uiCapability()->setUiHidden(true);
    wellPathImport.uiCapability()->setUiChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&mainPlotCollection, "MainPlotCollection", "Plots", "", "", "");
    mainPlotCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&viewLinkerCollection, "LinkedViews", "Linked Views (field in RimProject", ":/chain.png", "", "");
    viewLinkerCollection.uiCapability()->setUiHidden(true);
    viewLinkerCollection = new RimViewLinkerCollection;

    CAF_PDM_InitFieldNoDefault(&commandObjects, "CommandObjects", "CommandObjects", "", "", "");
    //wellPathImport.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&currentModelIndexPath, "TreeViewCurrentModelIndexPath", "",  "", "", "");
    currentModelIndexPath.uiCapability()->setUiHidden(true);

    // Obsolete fields. The content is moved to OilFields and friends
    CAF_PDM_InitFieldNoDefault(&casesObsolete, "Reservoirs", "",  "", "", "");
    casesObsolete.uiCapability()->setUiHidden(true);
    casesObsolete.xmlCapability()->setIOWritable(false); // read but not write, they will be moved into RimAnalysisGroups
    CAF_PDM_InitFieldNoDefault(&caseGroupsObsolete, "CaseGroups", "",  "", "", "");
    caseGroupsObsolete.uiCapability()->setUiHidden(true);
    caseGroupsObsolete.xmlCapability()->setIOWritable(false); // read but not write, they will be moved into RimAnalysisGroups

    // Initialization

    scriptCollection = new RimScriptCollection();
    scriptCollection->directory.uiCapability()->setUiHidden(true);
    scriptCollection->uiCapability()->setUiName("Scripts");
    scriptCollection->uiCapability()->setUiIcon(QIcon(":/Default.png"));

    mainPlotCollection = new RimMainPlotCollection();

    // For now, create a default first oilfield that contains the rest of the project
    oilFields.push_back(new RimOilField);

    initScriptDirectories();

    this->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimProject::~RimProject(void)
{
    close();

    oilFields.deleteAllChildObjects();
    if (scriptCollection()) delete scriptCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::close()
{
    if (mainPlotCollection() && mainPlotCollection()->wellLogPlotCollection()) 
    {
         mainPlotCollection()->wellLogPlotCollection()->wellLogPlots.deleteAllChildObjects();
    }

    if (mainPlotCollection() && mainPlotCollection()->summaryPlotCollection())
    {
        mainPlotCollection()->summaryPlotCollection()->m_summaryPlots.deleteAllChildObjects();
    }

    oilFields.deleteAllChildObjects();
    oilFields.push_back(new RimOilField);

    casesObsolete.deleteAllChildObjects();
    caseGroupsObsolete.deleteAllChildObjects();

    wellPathImport->regions().deleteAllChildObjects();

    commandObjects.deleteAllChildObjects();

    delete viewLinkerCollection->viewLinker();
    viewLinkerCollection->viewLinker = NULL;

    fileName = "";

    nextValidCaseId = 0;
    nextValidCaseGroupId = 0;
    currentModelIndexPath = "";
    treeViewState = "";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::initScriptDirectories()
{
    //
    // TODO : Must store content of scripts in project file and notify user if stored content is different from disk on execute and edit
    // 
    RiaApplication* app = RiaApplication::instance();
    QString scriptDirectories = app->scriptDirectories();

    this->setScriptDirectories(scriptDirectories);

    // Find largest used caseId read from file and make sure all cases have a valid caseId
    {
        int largestId = -1;

        std::vector<RimCase*> cases;
        allCases(cases);
    
        for (size_t i = 0; i < cases.size(); i++)
        {
            if (cases[i]->caseId > largestId)
            {
                largestId = cases[i]->caseId;
            }
        }

        if (largestId > this->nextValidCaseId)
        {
            this->nextValidCaseId = largestId + 1;
        }

        // Assign case Id to cases with an invalid case Id
        for (size_t i = 0; i < cases.size(); i++)
        {
            if (cases[i]->caseId < 0)
            {
                assignCaseIdToCase(cases[i]);
            }
        }
    }

    // Find largest used groupId read from file and make sure all groups have a valid groupId
    RimEclipseCaseCollection* analysisModels = activeOilField() ? activeOilField()->analysisModels() : NULL;
    if (analysisModels)
    {
        int largestGroupId = -1;
        
        for (size_t i = 0; i < analysisModels->caseGroups().size(); i++)
        {
            RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups()[i];

            if (cg->groupId > largestGroupId)
            {
                largestGroupId = cg->groupId;
            }
        }

        if (largestGroupId > this->nextValidCaseGroupId)
        {
            this->nextValidCaseGroupId = largestGroupId + 1;
        }

        // Assign group Id to groups with an invalid Id
        for (size_t i = 0; i < analysisModels->caseGroups().size(); i++)
        {
            RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups()[i];

            if (cg->groupId < 0)
            {
                assignIdToCaseGroup(cg);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::initAfterRead()
{
    initScriptDirectories();

    // Create an empty oil field in case the project did not contain one
    if (oilFields.size() < 1)
    {
        oilFields.push_back(new RimOilField);
    }

    // Handle old project files with obsolete structure.
    // Move caseGroupsObsolete and casesObsolete to oilFields()[idx]->analysisModels()
    RimEclipseCaseCollection* analysisModels = activeOilField() ? activeOilField()->analysisModels() : NULL;
    bool movedOneRimIdenticalGridCaseGroup = false;
    for (size_t cgIdx = 0; cgIdx < caseGroupsObsolete.size(); ++cgIdx)
    {
        RimIdenticalGridCaseGroup* sourceCaseGroup = caseGroupsObsolete[cgIdx];
        if (analysisModels)
        {
            analysisModels->caseGroups.push_back(sourceCaseGroup);
            //printf("Moved m_project->caseGroupsObsolete[%i] to first oil fields analysis models\n", cgIdx);
            movedOneRimIdenticalGridCaseGroup = true; // moved at least one so assume the others will be moved too...
        }
    }

    if (movedOneRimIdenticalGridCaseGroup)
    {
        caseGroupsObsolete.clear();
    }

    bool movedOneRimCase = false;
    for (size_t cIdx = 0; cIdx < casesObsolete().size(); ++cIdx)
    {
        if (analysisModels)
        {
            RimEclipseCase* sourceCase = casesObsolete[cIdx];
            casesObsolete.set(cIdx, NULL);
            analysisModels->cases.push_back(sourceCase);
            //printf("Moved m_project->casesObsolete[%i] to first oil fields analysis models\n", cIdx);
            movedOneRimCase = true; // moved at least one so assume the others will be moved too...
        }
    }

    if (movedOneRimCase)
    {
        casesObsolete.clear();
    }
    
    if (casesObsolete().size() > 0 || caseGroupsObsolete.size() > 0)
    {
        //printf("RimProject::initAfterRead: Was not able to move all cases (%i left) or caseGroups (%i left) from Project to analysisModels", 
          //  casesObsolete().size(), caseGroupsObsolete.size());
    }

    // Set project pointer to each well path
    for (size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
        if (oilField == NULL || oilField->wellPathCollection == NULL) continue;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::setupBeforeSave()
{
    m_projectFileVersionString = STRPRODUCTVER;
}

//--------------------------------------------------------------------------------------------------
/// Support list of multiple script paths divided by ';'
//--------------------------------------------------------------------------------------------------
void RimProject::setScriptDirectories(const QString& scriptDirectories)
{
    scriptCollection->calcScripts().deleteAllChildObjects();
    scriptCollection->subDirectories().deleteAllChildObjects();

    QStringList pathList = scriptDirectories.split(';');
    foreach(QString path, pathList)
    {
        QDir dir(path);
        if (!path.isEmpty() && dir.exists() && dir.isReadable())
        {
            RimScriptCollection* sharedScriptLocation = new RimScriptCollection;
            sharedScriptLocation->directory = path;
            sharedScriptLocation->setUiName(dir.dirName());

            sharedScriptLocation->readContentFromDisc();

            scriptCollection->subDirectories.push_back(sharedScriptLocation);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimProject::projectFileVersionString() const
{
    return m_projectFileVersionString;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::setProjectFileNameAndUpdateDependencies(const QString& fileName)
{
    // Extract the filename of the project file when it was saved 
    QString oldProjectFileName =  this->fileName;
    // Replace with the new actual filename
    this->fileName = fileName;

    QFileInfo fileInfo(fileName);
    QString newProjectPath = fileInfo.path();

    QFileInfo fileInfoOld(oldProjectFileName);
    QString oldProjectPath = fileInfoOld.path();

    // Loop over all cases and update file path

    std::vector<RimCase*> cases;
    allCases(cases);
    for (size_t i = 0; i < cases.size(); i++)
    {
        cases[i]->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
    }

    // Update path to well path file cache
    for (size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
        if (oilField == NULL || oilField->wellPathCollection == NULL) continue;
        oilField->wellPathCollection->updateFilePathsFromProjectPath();
    }

    wellPathImport->updateFilePaths();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::assignCaseIdToCase(RimCase* reservoirCase)
{
    if (reservoirCase)
    {
        reservoirCase->caseId = nextValidCaseId;

        nextValidCaseId = nextValidCaseId + 1;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::assignIdToCaseGroup(RimIdenticalGridCaseGroup* caseGroup)
{
    if (caseGroup)
    {
        caseGroup->groupId = nextValidCaseGroupId;

        nextValidCaseGroupId = nextValidCaseGroupId + 1;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::allCases(std::vector<RimCase*>& cases)
{
    for (size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
        if (!oilField) continue;

        RimEclipseCaseCollection* analysisModels = oilField->analysisModels();
        if (analysisModels ) 
        {
            for (size_t caseIdx = 0; caseIdx < analysisModels->cases.size(); caseIdx++)
            {
                cases.push_back(analysisModels->cases[caseIdx]);
            }
            for (size_t cgIdx = 0; cgIdx < analysisModels->caseGroups.size(); cgIdx++)
            {
                // Load the Main case of each IdenticalGridCaseGroup
                RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups[cgIdx];
                if (cg == NULL) continue;

                if (cg->statisticsCaseCollection())
                {
                    for (size_t caseIdx = 0; caseIdx < cg->statisticsCaseCollection()->reservoirs.size(); caseIdx++)
                    {
                        cases.push_back(cg->statisticsCaseCollection()->reservoirs[caseIdx]);
                    }
                }
                if (cg->caseCollection())
                {
                    for (size_t caseIdx = 0; caseIdx < cg->caseCollection()->reservoirs.size(); caseIdx++)
                    {
                        cases.push_back(cg->caseCollection()->reservoirs[caseIdx]);
                    }
                }
            }
        }

        RimGeoMechModels* geomModels = oilField->geoMechModels();
        if (geomModels)
        {
            for (size_t caseIdx = 0; caseIdx < geomModels->cases.size(); caseIdx++)
            {
                cases.push_back(geomModels->cases[caseIdx]);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::allSummaryCases(std::vector<RimSummaryCase*>& sumCases)
{
    for (RimOilField* oilField: oilFields)
    {
        if(!oilField) continue;
        RimSummaryCaseCollection* sumCaseColl = oilField->summaryCaseCollection();
        if(sumCaseColl)
        {
            for (size_t scIdx = 0; scIdx <  sumCaseColl->summaryCaseCount(); ++scIdx)
            {
                sumCases.push_back(sumCaseColl->summaryCase(scIdx));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::allNotLinkedViews(std::vector<RimView*>& views)
{
    std::vector<RimCase*> cases;
    allCases(cases);

    std::vector<RimView*> alreadyLinkedViews;
    if (viewLinkerCollection->viewLinker())
    {
        viewLinkerCollection->viewLinker()->allViews(alreadyLinkedViews);
    }

    for (size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++)
    {
        RimCase* rimCase = cases[caseIdx];
        if (!rimCase) continue;

        std::vector<RimView*> caseViews = rimCase->views();
        for (size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++)
        {
            bool isLinked = false;
            for (size_t lnIdx = 0; lnIdx < alreadyLinkedViews.size(); lnIdx++)
            {
                if (caseViews[viewIdx] == alreadyLinkedViews[lnIdx])
                {
                    isLinked = true;
                }
            }
            if (!isLinked)
            {
                views.push_back(caseViews[viewIdx]);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::allVisibleViews(std::vector<RimView*>& views)
{
    std::vector<RimCase*> cases;
    allCases(cases);
    
    for (size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++)
    {
        RimCase* rimCase = cases[caseIdx];
        if (!rimCase) continue;

        std::vector<RimView*> caseViews = rimCase->views();
        for (size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++)
        {
            if (caseViews[viewIdx] && caseViews[viewIdx]->viewer())
            {
                views.push_back(caseViews[viewIdx]);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::createDisplayModelAndRedrawAllViews()
{
    std::vector<RimCase*> cases;
    allCases(cases);
    for (size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++)
    {
        RimCase* rimCase = cases[caseIdx];
        if (rimCase == NULL) continue;
        std::vector<RimView*> views = rimCase->views();

        for (size_t viewIdx = 0; viewIdx < views.size(); viewIdx++)
        {
            views[viewIdx]->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Currently there will be only one oil field in Resinsight, so return hardcoded first oil field
/// from the RimOilField collection.
//--------------------------------------------------------------------------------------------------
RimOilField* RimProject::activeOilField()
{
    CVF_ASSERT(oilFields.size() == 1);
  
    return oilFields[0];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::computeUtmAreaOfInterest()
{
    std::vector<RimCase*> cases;
    allCases(cases);

    cvf::BoundingBox projectBB;

    for (size_t i = 0; i < cases.size(); i++)
    {
        RimEclipseCase* rimCase = dynamic_cast<RimEclipseCase*>(cases[i]);

        if (rimCase && rimCase->reservoirData())
        {
            for (size_t gridIdx = 0; gridIdx < rimCase->reservoirData()->gridCount(); gridIdx++ )
            {
                RigGridBase* rigGrid = rimCase->reservoirData()->grid(gridIdx);

                projectBB.add(rigGrid->boundingBox());
            }
        }
        else
        {
            // Todo : calculate BBox of GeoMechCase
        }
    }

    if (projectBB.isValid())
    {
        double north, south, east, west;

        north = projectBB.max().y();
        south = projectBB.min().y();

        west = projectBB.min().x();
        east = projectBB.max().x();

        wellPathImport->north = north;
        wellPathImport->south = south;
        wellPathImport->east = east;
        wellPathImport->west = west;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::actionsBasedOnSelection(QMenu& contextMenu)
{
    QStringList commandIds = RimContextCommandBuilder::commandsFromSelection();

    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
    for (int i = 0; i < commandIds.size(); i++)
    {
        if (commandIds[i] == "Separator")
        {
            contextMenu.addSeparator();
        }
        else if (commandIds[i] == "RicExecuteScriptForCasesFeature")
        {
            // Execute script on selection of cases
             RiuMainWindow* ruiMainWindow = RiuMainWindow::instance();
             if (ruiMainWindow)
             { 
                std::vector<RimCase*> cases;
                ruiMainWindow->selectedCases(cases);

                if (cases.size() > 0)
                {
                    QMenu* executeMenu = contextMenu.addMenu("Execute script");

                    RiaApplication* app = RiaApplication::instance();
                    RimProject* proj = app->project();
                    if (proj && proj->scriptCollection())
                    {
                        RimScriptCollection* rootScriptCollection = proj->scriptCollection();

                        // Root script collection holds a list of subdirectories of user defined script folders
                        for (size_t i = 0; i < rootScriptCollection->subDirectories.size(); i++)
                        {
                            RimScriptCollection* subDir = rootScriptCollection->subDirectories[i];

                            if (subDir)
                            {
                                appendScriptItems(executeMenu, subDir);
                            }
                        }
                    }

                    contextMenu.addSeparator();
                    contextMenu.addMenu(executeMenu);
                }
            }
        }
        else
        {
            caf::CmdFeature* feature = commandManager->getCommandFeature(commandIds[i].toStdString());
            if (feature->canFeatureBeExecuted())
            {
                QAction* act = commandManager->action(commandIds[i]);
                CVF_ASSERT(act);

                contextMenu.addAction(act);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::appendScriptItems(QMenu* menu, RimScriptCollection* scriptCollection)
{
    CVF_ASSERT(menu);

    QDir dir(scriptCollection->directory);
    QMenu* subMenu = menu->addMenu(dir.dirName());

    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
    CVF_ASSERT(commandManager);

    RicExecuteScriptForCasesFeature* executeScriptFeature = dynamic_cast<RicExecuteScriptForCasesFeature*>(commandManager->getCommandFeature("RicExecuteScriptForCasesFeature"));
    CVF_ASSERT(executeScriptFeature);

    for (size_t i = 0; i < scriptCollection->calcScripts.size(); i++)
    {
        RimCalcScript* calcScript = scriptCollection->calcScripts[i];
        QFileInfo fi(calcScript->absolutePath());

        QString menuText = fi.baseName();
        QAction* scriptAction = subMenu->addAction(menuText, executeScriptFeature, SLOT(slotExecuteScriptForSelectedCases()));

        scriptAction->setData(QVariant(calcScript->absolutePath()));
    }

    for (size_t i = 0; i < scriptCollection->subDirectories.size(); i++)
    {
        RimScriptCollection* subDir = scriptCollection->subDirectories[i];

        appendScriptItems(subMenu, subDir);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (viewLinkerCollection()->viewLinker())
    {
        // Use object instead of field to avoid duplicate entries in the tree view
        uiTreeOrdering.add(viewLinkerCollection());
    }

    RimOilField* oilField = activeOilField();
    if (oilField)
    {
        if (oilField->analysisModels())     uiTreeOrdering.add(oilField->analysisModels());
        if (oilField->geoMechModels())      uiTreeOrdering.add(oilField->geoMechModels());
        if (oilField->wellPathCollection()) uiTreeOrdering.add(oilField->wellPathCollection());
    }

    if (mainPlotCollection)
    {
        if (mainPlotCollection->wellLogPlotCollection())
        {
            uiTreeOrdering.add(mainPlotCollection->wellLogPlotCollection());
        }
        if (mainPlotCollection->summaryPlotCollection())
        {
            uiTreeOrdering.add(mainPlotCollection->summaryPlotCollection());
        }
    }

    uiTreeOrdering.add(scriptCollection());
    
    uiTreeOrdering.setForgetRemainingFields(true);
}

