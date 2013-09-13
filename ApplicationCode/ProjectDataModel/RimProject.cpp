/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "RimProject.h"
#include "cafAppEnum.h"

#include "RimOilField.h"
#include "RimAnalysisModels.h"

#include "RimReservoirView.h"
#include "RimScriptCollection.h"
#include "RimIdenticalGridCaseGroup.h"

#include "RiaApplication.h"
#include "RiaVersionInfo.h"

#include "RigGridManager.h"
#include "RigCaseData.h"
#include "RimResultCase.h"
#include "RimWellPathCollection.h"


#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimWellCollection.h"
#include "RimCaseCollection.h"
#include "RimResultSlot.h"
#include "RimStatisticsCase.h"

CAF_PDM_SOURCE_INIT(RimProject, "ResInsightProject");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimProject::RimProject(void)
{
    CAF_PDM_InitFieldNoDefault(&m_projectFileVersionString, "ProjectFileVersionString", "", "", "", "");
    m_projectFileVersionString.setUiHidden(true);

    CAF_PDM_InitField(&nextValidCaseId, "NextValidCaseId", 0, "Next Valid Case ID", "", "" ,"");
    nextValidCaseId.setUiHidden(true);

    CAF_PDM_InitField(&nextValidCaseGroupId, "NextValidCaseGroupId", 0, "Next Valid Case Group ID", "", "" ,"");
    nextValidCaseGroupId.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&oilFields, "OilFields", "Oil Fields",  "", "", "");
    oilFields.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&scriptCollection, "ScriptCollection", "Scripts", ":/Default.png", "", "");
    CAF_PDM_InitFieldNoDefault(&treeViewState, "TreeViewState", "",  "", "", "");
    treeViewState.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&wellPathImport, "WellPathImport", "WellPathImport", "", "", "");
    wellPathImport = new RimWellPathImport();
    wellPathImport.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&commandObjects, "CommandObjects", "CommandObjects", "", "", "");
    //wellPathImport.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&currentModelIndexPath, "TreeViewCurrentModelIndexPath", "",  "", "", "");
    currentModelIndexPath.setUiHidden(true);

    // Obsolete fields. The content is moved to OilFields and friends
    CAF_PDM_InitFieldNoDefault(&casesObsolete, "Reservoirs", "",  "", "", "");
    casesObsolete.setUiHidden(true);
    casesObsolete.setIOWritable(false); // read but not write, they will be moved into RimAnalysisGroups
    CAF_PDM_InitFieldNoDefault(&caseGroupsObsolete, "CaseGroups", "",  "", "", "");
    caseGroupsObsolete.setUiHidden(true);
    caseGroupsObsolete.setIOWritable(false); // read but not write, they will be moved into RimAnalysisGroups

    // Initialization

    scriptCollection = new RimScriptCollection();
    scriptCollection->directory.setUiHidden(true);

    // For now, create a default first oilfield that contains the rest of the project
    oilFields.push_back(new RimOilField);

    initScriptDirectories();
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
    oilFields.deleteAllChildObjects();
    oilFields.push_back(new RimOilField);

    casesObsolete.deleteAllChildObjects();
    caseGroupsObsolete.deleteAllChildObjects();

    wellPathImport = new RimWellPathImport();

    commandObjects.deleteAllChildObjects();

    fileName = "";

    nextValidCaseId = 0;
    nextValidCaseGroupId = 0;
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
    RimAnalysisModels* analysisModels = activeOilField() ? activeOilField()->analysisModels() : NULL;
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
    RimAnalysisModels* analysisModels = activeOilField() ? activeOilField()->analysisModels() : NULL;
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
        RimCase* sourceCase = casesObsolete[cIdx];
        if (analysisModels)
        {
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
        oilField->wellPathCollection->setProject(this);
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
        RimAnalysisModels* analysisModels = oilField ? oilField->analysisModels() : NULL;
        if (analysisModels == NULL) continue;

        for (size_t caseIdx = 0; caseIdx < analysisModels->cases.size(); caseIdx++)
        {
            cases.push_back(analysisModels->cases[caseIdx]);
        }
        for (size_t cgIdx = 0; cgIdx < analysisModels->caseGroups.size(); cgIdx++)
        {
            // Load the Main case of each IdenticalGridCaseGroup
            RimIdenticalGridCaseGroup* cg = analysisModels->caseGroups[cgIdx];
            if (cg == NULL) continue;

            for (size_t caseIdx = 0; caseIdx < cg->statisticsCaseCollection()->reservoirs.size(); caseIdx++)
            {
                cases.push_back(cg->statisticsCaseCollection()->reservoirs[caseIdx]);
            }

            for (size_t caseIdx = 0; caseIdx < cg->caseCollection()->reservoirs.size(); caseIdx++)
            {
                cases.push_back(cg->caseCollection()->reservoirs[caseIdx]);
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

        for (size_t viewIdx = 0; viewIdx < rimCase->reservoirViews.size(); viewIdx++)
        {
            RimReservoirView* reservoirView = rimCase->reservoirViews[viewIdx];
            reservoirView->scheduleCreateDisplayModelAndRedraw();
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
        RimCase* rimCase = cases[i];

        if (rimCase && rimCase->reservoirData())
        {
            for (size_t gridIdx = 0; gridIdx < rimCase->reservoirData()->gridCount(); gridIdx++ )
            {
                RigGridBase* rigGrid = rimCase->reservoirData()->grid(gridIdx);

                projectBB.add(rigGrid->boundingBox());
            }
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

