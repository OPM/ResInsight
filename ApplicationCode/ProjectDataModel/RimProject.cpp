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

#include "RIStdInclude.h"

#include "RimProject.h"
#include "RIApplication.h"
#include "RIVersionInfo.h"

#include "RigGridCollection.h"
#include "RigEclipseCase.h"


CAF_PDM_SOURCE_INIT(RimProject, "ResInsightProject");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimProject::RimProject(void)
{
    CAF_PDM_InitFieldNoDefault(&m_projectFileVersionString, "ProjectFileVersionString", "", "", "", "");
    m_projectFileVersionString.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&reservoirs, "Reservoirs", "",  "", "", "");
    CAF_PDM_InitFieldNoDefault(&caseGroups, "CaseGroups", "",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&scriptCollection, "ScriptCollection", "Scripts", ":/Default.png", "", "");
    
    scriptCollection = new RimScriptCollection();
    scriptCollection->directory.setUiHidden(true);

    m_gridCollection = new RigGridCollection;

    initAfterRead();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimProject::~RimProject(void)
{
    close();

    if (scriptCollection()) delete scriptCollection();

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

    fileName = "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::initAfterRead()
{
    //
    // TODO : Must store content of scripts in project file and notify user if stored content is different from disk on execute and edit
    // 
    RIApplication* app = RIApplication::instance();
    QString scriptDirectory = app->scriptDirectory();

    this->setUserScriptPath(scriptDirectory);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::setupBeforeSave()
{
    m_projectFileVersionString = STRPRODUCTVER;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject::setUserScriptPath(const QString& scriptDirectory)
{
    scriptCollection->calcScripts().deleteAllChildObjects();
    scriptCollection->subDirectories().deleteAllChildObjects();


    QDir dir(scriptDirectory);
    if (!scriptDirectory.isEmpty() && dir.exists())
    {
        RimScriptCollection* sharedScriptLocation = new RimScriptCollection;
        sharedScriptLocation->directory = scriptDirectory;
        sharedScriptLocation->setUiName(dir.dirName());

        sharedScriptLocation->readContentFromDisc();

        scriptCollection->subDirectories.push_back(sharedScriptLocation);
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
void RimProject::moveEclipseCaseIntoCaseGroup(RimReservoir* rimReservoir)
{
    CVF_ASSERT(rimReservoir);

    RigEclipseCase* rigEclipseCase = rimReservoir->reservoirData();

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
    }

    m_gridCollection->addCase(rigEclipseCase);


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
        group->addCase(rimReservoir);

        caseGroups().push_back(group);
    }

    // Remove reservoir from main container
    reservoirs().removeChildObject(rimReservoir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimProject::removeEclipseCaseFromAllGroups(RimReservoir* reservoir)
{
    m_gridCollection->removeCase(reservoir->reservoirData());

    std::vector<RimIdenticalGridCaseGroup*> emptyCaseGroups;
    for (size_t i = 0; i < caseGroups.size(); i++)
    {
        RimIdenticalGridCaseGroup* cg = caseGroups()[i];

        cg->caseCollection()->reservoirs().removeChildObject(reservoir);

        if (cg->caseCollection()->reservoirs().size() == 0)
        {
            emptyCaseGroups.push_back(cg);
        }
    }

    for (size_t i = 0; i < emptyCaseGroups.size(); i++)
    {
        caseGroups().removeChildObject(emptyCaseGroups[i]);
        delete emptyCaseGroups[i];
    }

    reservoirs().removeChildObject(reservoir);
}
