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
#include "RiaProjectFileVersionTools.h"
#include "RiaVersionInfo.h"
#include "RiaFilePathTools.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"

#include "RimCalcScript.h"
#include "RimCase.h"
#include "RimCaseCollection.h"
#include "RimCommandObject.h"
#include "RimContextCommandBuilder.h"
#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimGridSummaryCase.h"
#include "RimGridView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimMultiSnapshotDefinition.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimPltPlotCollection.h"
#include "RimRftPlotCollection.h"
#include "RimScriptCollection.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryPlotCollection.h"
#include "RimTools.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathImport.h"

#include "RiuPlotMainWindow.h"
#include "RiuMainWindow.h"

#include "OctaveScriptCommands/RicExecuteScriptForCasesFeature.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfBoundingBox.h"

#include <QDir>
#include <QMenu>
#include "RiaCompletionTypeCalculationScheduler.h"


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

    CAF_PDM_InitFieldNoDefault(&scriptCollection, "ScriptCollection", "Octave Scripts", ":/octave.png", "", "");
    scriptCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&wellPathImport, "WellPathImport", "WellPathImport", "", "", "");
    wellPathImport = new RimWellPathImport();
    wellPathImport.uiCapability()->setUiHidden(true);
    wellPathImport.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&mainPlotCollection, "MainPlotCollection", "Plots", "", "", "");
    mainPlotCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&viewLinkerCollection, "LinkedViews", "Linked Views (field in RimProject", ":/chain.png", "", "");
    viewLinkerCollection.uiCapability()->setUiHidden(true);
    viewLinkerCollection = new RimViewLinkerCollection;

    CAF_PDM_InitFieldNoDefault(&calculationCollection, "CalculationCollection", "Calculation Collection", "", "", "");
    calculationCollection = new RimSummaryCalculationCollection;

    CAF_PDM_InitFieldNoDefault(&commandObjects, "CommandObjects", "Command Objects", "", "", "");
    //wellPathImport.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&multiSnapshotDefinitions, "MultiSnapshotDefinitions", "Multi Snapshot Definitions", "", "", "");

    CAF_PDM_InitFieldNoDefault(&mainWindowTreeViewState, "TreeViewState", "",  "", "", "");
    mainWindowTreeViewState.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&mainWindowCurrentModelIndexPath, "TreeViewCurrentModelIndexPath", "",  "", "", "");
    mainWindowCurrentModelIndexPath.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&plotWindowTreeViewState, "PlotWindowTreeViewState", "", "", "", "");
    plotWindowTreeViewState.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&plotWindowCurrentModelIndexPath, "PlotWindowTreeViewCurrentModelIndexPath", "", "", "", "");
    plotWindowCurrentModelIndexPath.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_show3DWindow, "show3DWindow", true, "Show 3D Window", "", "", "");
    m_show3DWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_showPlotWindow, "showPlotWindow", false, "Show Plot Window", "", "", "");
    m_showPlotWindow.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_dialogData, "DialogData", "DialogData", "", "", "");
    m_dialogData = new RimDialogData();
    m_dialogData.uiCapability()->setUiHidden(true);
    m_dialogData.uiCapability()->setUiTreeChildrenHidden(true);

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
    scriptCollection->uiCapability()->setUiIcon(QIcon(":/octave.png"));

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
    if (mainPlotCollection()) 
    {
        mainPlotCollection()->deleteAllContainedObjects();
    }

    oilFields.deleteAllChildObjects();
    oilFields.push_back(new RimOilField);

    casesObsolete.deleteAllChildObjects();
    caseGroupsObsolete.deleteAllChildObjects();

    wellPathImport->regions().deleteAllChildObjects();

    commandObjects.deleteAllChildObjects();

    multiSnapshotDefinitions.deleteAllChildObjects();

    m_dialogData->clearProjectSpecificData();

    calculationCollection->deleteAllContainedObjects();

    delete viewLinkerCollection->viewLinker();
    viewLinkerCollection->viewLinker = nullptr;

    fileName = "";

    nextValidCaseId = 0;
    nextValidCaseGroupId = 0;
    mainWindowCurrentModelIndexPath = "";
    mainWindowTreeViewState = "";
    plotWindowCurrentModelIndexPath = "";
    plotWindowTreeViewState = "";
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
    RimEclipseCaseCollection* analysisModels = activeOilField() ? activeOilField()->analysisModels() : nullptr;
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
    RimEclipseCaseCollection* analysisModels = activeOilField() ? activeOilField()->analysisModels() : nullptr;
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
            casesObsolete.set(cIdx, nullptr);
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
        if (oilField == nullptr || oilField->wellPathCollection == nullptr) continue;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::setupBeforeSave()
{
    m_show3DWindow = RiuMainWindow::instance()->isVisible();

    if (RiaApplication::instance()->mainPlotWindow() &&
        RiaApplication::instance()->mainPlotWindow()->isVisible())
    {
        m_showPlotWindow = true;
    }
    else
    {
        m_showPlotWindow = false;
    }

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
bool RimProject::isProjectFileVersionEqualOrOlderThan(const QString& otherProjectFileVersion) const
{
    QString candidateProjectFileVersion = projectFileVersionString();

    return !RiaProjectFileVersionTools::isCandidateVersionNewerThanOther(candidateProjectFileVersion, otherProjectFileVersion);
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
    
    std::vector<caf::FilePath*> filePaths;
    fieldsByType(this, filePaths);

    for (caf::FilePath* filePath : filePaths)
    {
        bool foundFile = false;
        std::vector<QString> searchedPaths;

        QString newFilePath = RimTools::relocateFile(filePath->path(), newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
        filePath->setPath(newFilePath);
    }

    // Loop over all cases and update file path

    std::vector<RimCase*> cases;
    allCases(cases);
    for (size_t i = 0; i < cases.size(); i++)
    {
        cases[i]->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
    }

    for (RimSummaryCase* summaryCase : allSummaryCases())
    {
        summaryCase->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
    }

    // Update path to well path file cache
    for(RimOilField* oilField: oilFields)
    {
        if (oilField == nullptr) continue;
        if (oilField->wellPathCollection() != nullptr)
        {
            oilField->wellPathCollection()->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
        }
        if (oilField->formationNamesCollection() != nullptr)
        {
            oilField->formationNamesCollection()->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
        }
        if (oilField->summaryCaseMainCollection() != nullptr) {
            oilField->summaryCaseMainCollection()->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
        }

        CVF_ASSERT(oilField->fractureDefinitionCollection());
        oilField->fractureDefinitionCollection()->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
    }

    {
        std::vector<RimWellLogFile*> rimWellLogFiles;
        this->descendantsIncludingThisOfType(rimWellLogFiles);

        for (auto rimWellLogFile : rimWellLogFiles)
        {
            rimWellLogFile->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
        }
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
                if (cg == nullptr) continue;

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
std::vector<RimSummaryCase*> RimProject::allSummaryCases() const
{
    std::vector<RimSummaryCase*> sumCases;

    for (RimOilField* oilField: oilFields)
    {
        if(!oilField) continue;
        RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
        if(sumCaseMainColl)
        {
            std::vector<RimSummaryCase*> allSummaryCases = sumCaseMainColl->allSummaryCases();
            sumCases.insert(sumCases.end(), allSummaryCases.begin(), allSummaryCases.end());
        }

        auto observedDataColl = oilField->observedDataCollection();
        if (observedDataColl != nullptr && observedDataColl->allObservedData().size() > 0)
        {
            auto observedData = observedDataColl->allObservedData();
            sumCases.insert(sumCases.end(), observedData.begin(), observedData.end());
        }
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCaseCollection*> RimProject::summaryGroups() const
{
    std::vector<RimSummaryCaseCollection*> groups;

    for (RimOilField* oilField : oilFields)
    {
        if (!oilField) continue;
        RimSummaryCaseMainCollection* sumCaseMainColl = oilField->summaryCaseMainCollection();
        if (sumCaseMainColl)
        {
            std::vector<RimSummaryCaseCollection*> g = sumCaseMainColl->summaryCaseCollections();
            groups.insert(groups.end(), g.begin(), g.end());
        }
    }

    return groups;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection* RimProject::firstSummaryCaseMainCollection() const
{
    if (oilFields.empty()) return nullptr;
    return oilFields[0]->summaryCaseMainCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::allNotLinkedViews(std::vector<RimGridView*>& views)
{
    std::vector<RimCase*> cases;
    allCases(cases);

    std::vector<RimGridView*> alreadyLinkedViews;
    if (viewLinkerCollection->viewLinker())
    {
        viewLinkerCollection->viewLinker()->allViews(alreadyLinkedViews);
    }

    for (size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++)
    {
        RimCase* rimCase = cases[caseIdx];
        if (!rimCase) continue;

        std::vector<Rim3dView*> caseViews = rimCase->views();
        for (size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++)
        {
            RimGridView* gridView = dynamic_cast<RimGridView*>(caseViews[viewIdx]);
            
            if (!gridView) continue;

            bool isLinked = false;
            for (size_t lnIdx = 0; lnIdx < alreadyLinkedViews.size(); lnIdx++)
            {
                if (gridView == alreadyLinkedViews[lnIdx])
                {
                    isLinked = true;
                }
            }
            if (!isLinked)
            {
                views.push_back(gridView);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::allVisibleViews(std::vector<Rim3dView*>& views)
{
    std::vector<RimCase*> cases;
    allCases(cases);
    
    for (size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++)
    {
        RimCase* rimCase = cases[caseIdx];
        if (!rimCase) continue;

        std::vector<Rim3dView*> caseViews = rimCase->views();
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
void RimProject::allVisibleGridViews(std::vector<RimGridView*>& views)
{
    std::vector<Rim3dView*> visibleViews;
    this->allVisibleViews(visibleViews);
    for ( Rim3dView* view : visibleViews )
    {
        RimGridView* gridView = dynamic_cast<RimGridView*>(view);
        if ( gridView ) views.push_back(gridView);
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
        if (rimCase == nullptr) continue;
        std::vector<Rim3dView*> views = rimCase->views();

        for (size_t viewIdx = 0; viewIdx < views.size(); viewIdx++)
        {
            views[viewIdx]->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::allOilFields(std::vector<RimOilField*>& oilFields) const
{
    oilFields.clear();
    for (const auto& oilField : this->oilFields)
    {
        oilFields.push_back(oilField);
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
/// Currently there will be only one oil field in Resinsight, so return hardcoded first oil field
/// from the RimOilField collection.
//--------------------------------------------------------------------------------------------------
const RimOilField * RimProject::activeOilField() const
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

        if (rimCase && rimCase->eclipseCaseData())
        {
            for (size_t gridIdx = 0; gridIdx < rimCase->eclipseCaseData()->gridCount(); gridIdx++ )
            {
                RigGridBase* rigGrid = rimCase->eclipseCaseData()->grid(gridIdx);

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
    caf::CmdFeatureMenuBuilder menuBuilder = RimContextCommandBuilder::commandsFromSelection();

    menuBuilder.appendToMenu(&contextMenu);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimProject::show3DWindow() const
{
    return m_show3DWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimProject::showPlotWindow() const
{
    return m_showPlotWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::reloadCompletionTypeResultsInAllViews()
{
    createDisplayModelAndRedrawAllViews();
    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDialogData* RimProject::dialogData() const
{
    return m_dialogData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimProject::eclipseCases() const
{
    std::vector<RimEclipseCase*> allCases;
    for (const auto& oilField : oilFields)
    {
        const auto& cases = oilField->analysisModels->cases;
        allCases.insert(allCases.end(), cases.begin(), cases.end());
    }
    return allCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimProject::eclipseCaseFromGridFileName(const QString& gridFileName) const
{
    for (RimEclipseCase* eclCase : eclipseCases())
    {
        if (RiaFilePathTools::toInternalSeparator(eclCase->gridFileName()) == RiaFilePathTools::toInternalSeparator(gridFileName))
        {
            return eclCase;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimProject::simulationWellNames() const
{
    std::set<QString> wellNames;

    for (RimOilField* oilField : oilFields)
    {
        auto analysisCaseColl = oilField->analysisModels();
        for (RimEclipseCase* eclCase : analysisCaseColl->cases())
        {
            const auto& eclData = eclCase->eclipseCaseData();
            if (eclData == nullptr) continue;

            const auto names = eclData->simulationWellNames();
            wellNames.insert(names.begin(), names.end());
        }
    }
    return std::vector<QString>(wellNames.begin(), wellNames.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimProject::wellPathFromSimWellName(const QString& simWellName, int branchIndex)
{
    std::vector<RimWellPath*> paths;
    for (RimWellPath* const path : allWellPaths())
    {
        if (QString::compare(path->associatedSimulationWellName(), simWellName) == 0 &&
            (branchIndex < 0 || path->associatedSimulationWellBranch() == branchIndex))
        {
            return path;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RimProject::wellPathByName(const QString& wellPathName) const
{
    for (RimWellPath* const path : allWellPaths())
    {
        if (path->name() == wellPathName) return path;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RimProject::allWellPaths() const
{
    std::vector<RimWellPath*> paths;
    for (const auto& oilField : oilFields())
    {
        auto wellPathColl = oilField->wellPathCollection();
        for (const auto& path : wellPathColl->wellPaths)
        {
            paths.push_back(path);
        }
    }
    return paths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimGeoMechCase*> RimProject::geoMechCases() const
{
    std::vector<RimGeoMechCase*> cases;

    for (size_t oilFieldIdx = 0; oilFieldIdx < oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = oilFields[oilFieldIdx];
        if (!oilField) continue;

        RimGeoMechModels* geomModels = oilField->geoMechModels();
        if (geomModels)
        {
            for (size_t caseIdx = 0; caseIdx < geomModels->cases.size(); caseIdx++)
            {
                cases.push_back(geomModels->cases[caseIdx]);
            }
        }
    }
    return cases;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureTemplateCollection*> RimProject::allFractureTemplateCollections() const
{
    std::vector<RimFractureTemplateCollection*> templColls;
    std::vector<RimOilField*> oilFields;

    allOilFields(oilFields);
    for (RimOilField* oilField : oilFields)
    {
        templColls.push_back(oilField->fractureDefinitionCollection());
    }
    return templColls;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureTemplate*> RimProject::allFractureTemplates() const
{
    std::vector<RimFractureTemplate*> templates;
    std::vector<RimOilField*> oilFields;

    allOilFields(oilFields);
    for (RimFractureTemplateCollection* templColl : allFractureTemplateCollections())
    {
        for (RimFractureTemplate* templ : templColl->fractureTemplates())
        {
            templates.push_back(templ);
        }
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::reloadCompletionTypeResultsForEclipseCase(RimEclipseCase* eclipseCase)
{
    std::vector<Rim3dView*> views = eclipseCase->views();

    for (size_t viewIdx = 0; viewIdx < views.size(); viewIdx++)
    {
        views[viewIdx]->scheduleCreateDisplayModelAndRedraw();
    }

    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews(eclipseCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (uiConfigName == "PlotWindow")
    {
        RimOilField* oilField = activeOilField();
        if (oilField)
        {
            if (oilField->summaryCaseMainCollection())
            {
                uiTreeOrdering.add( oilField->summaryCaseMainCollection() );
            }
            if (oilField->observedDataCollection())
            {
                uiTreeOrdering.add( oilField->observedDataCollection() );
            }
        }

        if (mainPlotCollection)
        {
           if (mainPlotCollection->summaryPlotCollection())
            {
                uiTreeOrdering.add(mainPlotCollection->summaryPlotCollection());
            }

           if (mainPlotCollection->summaryCrossPlotCollection())
           {
               uiTreeOrdering.add(mainPlotCollection->summaryCrossPlotCollection());
           }

           if (mainPlotCollection->wellLogPlotCollection())
            {
                uiTreeOrdering.add(mainPlotCollection->wellLogPlotCollection());
            }

           if (mainPlotCollection->rftPlotCollection())
           {
               uiTreeOrdering.add(mainPlotCollection->rftPlotCollection());
           }
           
           if (mainPlotCollection->pltPlotCollection())
           {
               uiTreeOrdering.add(mainPlotCollection->pltPlotCollection());
           }

           if (mainPlotCollection->flowPlotCollection())
            {
                uiTreeOrdering.add(mainPlotCollection->flowPlotCollection());
            }
        }
    }
    else
    {
        if (viewLinkerCollection()->viewLinker())
        {
            // Use object instead of field to avoid duplicate entries in the tree view
            uiTreeOrdering.add(viewLinkerCollection());
        }

        RimOilField* oilField = activeOilField();
        if (oilField)
        {
            if (oilField->analysisModels())                 uiTreeOrdering.add(oilField->analysisModels());
            if (oilField->geoMechModels())                  uiTreeOrdering.add(oilField->geoMechModels());
            if (oilField->wellPathCollection())             uiTreeOrdering.add(oilField->wellPathCollection());
            if (oilField->formationNamesCollection())       uiTreeOrdering.add(oilField->formationNamesCollection());
            if (oilField->fractureDefinitionCollection())   uiTreeOrdering.add(oilField->fractureDefinitionCollection());
        }

        uiTreeOrdering.add(scriptCollection());
    }
        
    uiTreeOrdering.skipRemainingChildren(true);
}

