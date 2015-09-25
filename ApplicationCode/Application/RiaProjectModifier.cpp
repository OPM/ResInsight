/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RiaProjectModifier.h"

#include "RimProject.h"
#include "RimEclipseCaseCollection.h"
#include "RimOilField.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimCaseCollection.h"
#include "RimEclipseResultCase.h"

#include "RimEclipseView.h"
#include "RimWellPathCollection.h"
#include "RimScriptCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipsePropertyFilter.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimEclipseCellColors.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"

#include <QFileInfo>
#include <QDir>




//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaProjectModifier::RiaProjectModifier()
:   m_replaceCase_caseId(UNDEFINED),
    m_replaceSourceCases_caseGroupId(UNDEFINED)
{
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceCaseFirstOccurence(QString newGridFileName)
{
    m_replaceCase_caseId = FIRST_OCCURENCE;
    m_replaceCase_gridFileName = makeFilePathAbsolute(newGridFileName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceCase(int caseIdToReplace, QString newGridFileName)
{
    if (caseIdToReplace >= 0)
    {
        m_replaceCase_caseId = caseIdToReplace;
        m_replaceCase_gridFileName = makeFilePathAbsolute(newGridFileName);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceSourceCasesFirstOccurence(std::vector<QString> newGridFileNames)
{
    m_replaceSourceCases_caseGroupId = FIRST_OCCURENCE;

    m_replaceSourceCases_gridFileNames.clear();
    foreach (QString fn, newGridFileNames)
    {
        m_replaceSourceCases_gridFileNames.push_back(makeFilePathAbsolute(fn));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaProjectModifier::setReplaceSourceCasesById(int caseGroupIdToReplace, std::vector<QString> newGridFileNames)
{
    if (caseGroupIdToReplace >= 0)
    {
        m_replaceSourceCases_caseGroupId = caseGroupIdToReplace;

        m_replaceSourceCases_gridFileNames.clear();
        foreach (QString fn, newGridFileNames)
        {
            m_replaceSourceCases_gridFileNames.push_back(makeFilePathAbsolute(fn));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaProjectModifier::applyModificationsToProject(RimProject* project) 
{
    if (m_replaceCase_caseId != UNDEFINED && !m_replaceCase_gridFileName.isEmpty())
    {
        replaceCase(project);
    }

    if (m_replaceSourceCases_caseGroupId != UNDEFINED && m_replaceSourceCases_gridFileNames.size() > 0)
    {
        replaceSourceCases(project);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaProjectModifier::replaceSourceCases(RimProject* project)
{
    bool didReplacement = false;

    for (size_t oilFieldIdx = 0; oilFieldIdx < project->oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = project->oilFields[oilFieldIdx];
        RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : NULL;
        if (analysisModels)
        {
            const size_t numCaseGroups = analysisModels->caseGroups.size();
            for (size_t caseGrpIdx = 0; caseGrpIdx < numCaseGroups; ++caseGrpIdx)
            {
                RimIdenticalGridCaseGroup* caseGroup = analysisModels->caseGroups[caseGrpIdx];
                if (m_replaceSourceCases_caseGroupId == FIRST_OCCURENCE ||
                    m_replaceSourceCases_caseGroupId == caseGroup->groupId)
                {
                    RimCaseCollection* caseCollection = caseGroup->caseCollection;
                    caseCollection->reservoirs.deleteAllChildObjects();

                    for (size_t i = 0; i < m_replaceSourceCases_gridFileNames.size(); i++)
                    {
                        QString fileName = m_replaceSourceCases_gridFileNames[i];
                        QString caseName = caseNameFromGridFileName(fileName);

                        // Use this slightly hackish method in order to get a new unique ID
                        RimEclipseResultCase* resCase = new RimEclipseResultCase;
                        resCase->setCaseInfo(caseName, fileName);

                        caseCollection->reservoirs.push_back(resCase);
                    }

                    didReplacement = true;

                    if (m_replaceSourceCases_caseGroupId == FIRST_OCCURENCE)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return didReplacement;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaProjectModifier::replaceCase(RimProject* project)
{
    bool didReplacement = false;
    std::vector<RimCase*> allCases;
    project->allCases(allCases);

    for (size_t caseIdx = 0; caseIdx < allCases.size(); ++caseIdx)
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>(allCases[caseIdx]);
        if (resultCase)
        {
            if (m_replaceCase_caseId == FIRST_OCCURENCE ||
                m_replaceCase_caseId == resultCase->caseId())
            {
                resultCase->setGridFileName(m_replaceCase_gridFileName);
                resultCase->caseUserDescription = caseNameFromGridFileName(m_replaceCase_gridFileName);
                didReplacement = true;

                if (m_replaceCase_caseId == FIRST_OCCURENCE)
                {
                    return true;
                }
            }
        }
    }

    return didReplacement;
}


//--------------------------------------------------------------------------------------------------
/// Returns absolute path name to the specified file. 
/// 
/// If \a relOrAbsolutePath is a relative, the current working directory for the process will be
/// used in order to make the path absolute.
//--------------------------------------------------------------------------------------------------
QString RiaProjectModifier::makeFilePathAbsolute(QString relOrAbsolutePath)
{
    QFileInfo theFile(relOrAbsolutePath);
    theFile.makeAbsolute();
    return theFile.filePath();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaProjectModifier::caseNameFromGridFileName(QString fullGridFilePathName)
{
    QString fn = QDir::fromNativeSeparators(fullGridFilePathName);
    
    // Extract file name plus the 'deepest' directory
    QString deepestDirPlusFileName = fn.section('/', -2, -1);

    deepestDirPlusFileName.replace("/", "--");

    return deepestDirPlusFileName;
}


