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
#include "RimScriptCollection.h"
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
    for (size_t i = 0; i < reservoirs.size(); i++)
    {
        m_gridCollection->removeCase(reservoirs[i]->reservoirData());
    }

    reservoirs.deleteAllChildObjects();

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
void RimProject::registerEclipseCase(RigEclipseCase* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    RigMainGrid* equalGrid = m_gridCollection->findEqualGrid(eclipseCase->mainGrid());

    if (equalGrid)
    {
        // Replace the grid with an already registered grid
        eclipseCase->setMainGrid(equalGrid);
    }
    else
    {
        // This is the first insertion of this grid, compute cached data
        eclipseCase->mainGrid()->computeCachedData();

        std::vector<RigGridBase*> grids;
        eclipseCase->allGrids(&grids);

        size_t i;
        for (i = 0; i < grids.size(); i++)
        {
            grids[i]->computeFaults();
        }
    }

    m_gridCollection->addCase(eclipseCase);
}
