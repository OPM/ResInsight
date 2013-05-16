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

    CAF_PDM_InitFieldNoDefault(&reservoirs, "Reservoirs", "",  "", "", "");
    CAF_PDM_InitFieldNoDefault(&caseGroups, "CaseGroups", "",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&scriptCollection, "ScriptCollection", "Scripts", ":/Default.png", "", "");
    CAF_PDM_InitFieldNoDefault(&treeViewState, "TreeViewState", "",  "", "", "");
    treeViewState.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&currentModelIndexPath, "TreeViewCurrentModelIndexPath", "",  "", "", "");
    currentModelIndexPath.setUiHidden(true);

    scriptCollection = new RimScriptCollection();
    scriptCollection->directory.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&wellPathCollection, "WellPathCollection", "Well Paths", ":/WellCollection.png", "", "");

    m_gridCollection = new RigGridManager;

    initScriptDirectories();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimProject::~RimProject(void)
{
    close();

    if (scriptCollection()) delete scriptCollection();
    if (wellPathCollection()) delete wellPathCollection();

    reservoirs.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::close()
{
    m_gridCollection->clear();

    reservoirs.deleteAllChildObjects();
    caseGroups.deleteAllChildObjects();
    if (wellPathCollection != NULL) delete wellPathCollection;

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
    {
        int largestGroupId = -1;

        for (size_t i = 0; i < caseGroups.size(); i++)
        {
            RimIdenticalGridCaseGroup* cg = caseGroups()[i];

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
        for (size_t i = 0; i < caseGroups.size(); i++)
        {
            RimIdenticalGridCaseGroup* cg = caseGroups()[i];

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

    if (wellPathCollection) wellPathCollection->setProject(this);
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
RimIdenticalGridCaseGroup* RimProject::createIdenticalCaseGroupFromMainCase(RimCase* mainCase)
{
    CVF_ASSERT(mainCase);

    RigCaseData* rigEclipseCase = mainCase->reservoirData();
    RigMainGrid* equalGrid = registerCaseInGridCollection(rigEclipseCase);
    CVF_ASSERT(equalGrid);

    RimIdenticalGridCaseGroup* group = new RimIdenticalGridCaseGroup;
    assignIdToCaseGroup(group);

    RimCase* createdCase = group->createAndAppendStatisticsCase();
    assignCaseIdToCase(createdCase);

    group->addCase(mainCase);
    caseGroups().push_back(group);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::moveEclipseCaseIntoCaseGroup(RimCase* rimReservoir)
{
    CVF_ASSERT(rimReservoir);

    RigCaseData* rigEclipseCase = rimReservoir->reservoirData();
    RigMainGrid* equalGrid = registerCaseInGridCollection(rigEclipseCase);
    CVF_ASSERT(equalGrid);

    // Insert in identical grid group
    bool foundGroup = false;

    for (size_t i = 0; i < caseGroups.size(); i++)
    {
        RimIdenticalGridCaseGroup* cg = caseGroups()[i];

        if (cg->mainGrid() == equalGrid)
        {
            cg->addCase(rimReservoir);
            foundGroup = true;
        }
    }

    if (!foundGroup)
    {
        RimIdenticalGridCaseGroup* group = new RimIdenticalGridCaseGroup;
        assignIdToCaseGroup(group);

        group->addCase(rimReservoir);

        caseGroups().push_back(group);
    }

    // Remove reservoir from main container
    reservoirs().removeChildObject(rimReservoir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::removeCaseFromAllGroups(RimCase* reservoir)
{
    m_gridCollection->removeCase(reservoir->reservoirData());

    for (size_t i = 0; i < caseGroups.size(); i++)
    {
        RimIdenticalGridCaseGroup* cg = caseGroups()[i];

        cg->removeCase(reservoir);
    }

    reservoirs().removeChildObject(reservoir);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimProject::registerCaseInGridCollection(RigCaseData* rigEclipseCase)
{
    CVF_ASSERT(rigEclipseCase);

    RigMainGrid* equalGrid = m_gridCollection->findEqualGrid(rigEclipseCase->mainGrid());

    if (equalGrid)
    {
        // Replace the grid with an already registered grid
        rigEclipseCase->setMainGrid(equalGrid);
    }
    else
    {
        // This is the first insertion of this grid, compute cached data
        rigEclipseCase->mainGrid()->computeCachedData();

        std::vector<RigGridBase*> grids;
        rigEclipseCase->allGrids(&grids);

        size_t i;
        for (i = 0; i < grids.size(); i++)
        {
            grids[i]->computeFaults();
        }

        equalGrid = rigEclipseCase->mainGrid();
    }

    m_gridCollection->addCase(rigEclipseCase);

    return equalGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::insertCaseInCaseGroup(RimIdenticalGridCaseGroup* caseGroup, RimCase* rimReservoir)
{
    CVF_ASSERT(rimReservoir);

    RigCaseData* rigEclipseCase = rimReservoir->reservoirData();
    registerCaseInGridCollection(rigEclipseCase);

    caseGroup->addCase(rimReservoir);
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

    // Loop over all reservoirs and update file path

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
    for (size_t i = 0; i < reservoirs.size(); i++)
    {
        cases.push_back(reservoirs[i]);
    }

    for (size_t i = 0; i < caseGroups.size(); i++)
    {
        RimIdenticalGridCaseGroup* cg = caseGroups()[i];

        for (size_t i = 0; i < cg->statisticsCaseCollection()->reservoirs.size(); i++)
        {
            cases.push_back(cg->statisticsCaseCollection()->reservoirs[i]);
        }

        for (size_t i = 0; i < cg->caseCollection()->reservoirs.size(); i++)
        {
            cases.push_back(cg->caseCollection()->reservoirs[i]);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::createDisplayModelAndRedrawAllViews()
{
    for (size_t caseIdx = 0; caseIdx < reservoirs.size(); caseIdx++)
    {
        RimCase* rimCase = reservoirs[caseIdx];
        for (size_t viewIdx = 0; viewIdx < rimCase->reservoirViews.size(); viewIdx++)
        {
            RimReservoirView* reservoirView = rimCase->reservoirViews[viewIdx];
            reservoirView->createDisplayModelAndRedraw();
        }
    }
}

