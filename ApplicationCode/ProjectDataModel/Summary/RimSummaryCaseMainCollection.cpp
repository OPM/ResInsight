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
#include "RimSummaryCaseMainCollection.h"

#include "RifEclipseSummaryTools.h"

#include "RimEclipseResultCase.h"
#include "RimFileSummaryCase.h"
#include "RimGridSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include <QDir>


CAF_PDM_SOURCE_INIT(RimSummaryCaseMainCollection,"SummaryCaseCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::RimSummaryCaseMainCollection()
{
    CAF_PDM_InitObject("Summary Cases",":/Cases16x16.png","","");

    CAF_PDM_InitFieldNoDefault(&m_cases, "SummaryCases", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_caseCollections, "SummaryCaseCollections", "", "", "", "");

    m_cases.uiCapability()->setUiHidden(true);
    m_caseCollections.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::~RimSummaryCaseMainCollection()
{
    m_cases.deleteAllChildObjects();
    m_caseCollections.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::createSummaryCasesFromRelevantEclipseResultCases()
{
    RimProject* proj = nullptr;
    firstAncestorOrThisOfType(proj);
    if (proj)
    {
        std::vector<RimCase*> all3DCases;
        proj->allCases(all3DCases);
        for (RimCase* aCase: all3DCases)
        {
            RimEclipseResultCase* eclResCase = dynamic_cast<RimEclipseResultCase*>(aCase);
            if (eclResCase)
            {
                // If we have no summary case corresponding to this eclipse case,
                // try to create one.
                bool isFound = false;
                for (size_t scIdx = 0; scIdx < m_cases.size(); ++scIdx)
                {
                    RimGridSummaryCase* grdSumCase = dynamic_cast<RimGridSummaryCase*>(m_cases[scIdx]);
                    if (grdSumCase)
                    {
                        if (grdSumCase->associatedEclipseCase() == eclResCase)
                        {
                            isFound = true;
                            break;
                        }
                    }
                }

                if (!isFound)
                {
                    // Create new GridSummaryCase
                    createAndAddSummaryCaseFromEclipseResultCase(eclResCase);

                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::findSummaryCaseFromEclipseResultCase(RimEclipseResultCase* eclipseResultCase) const
{
    for (RimSummaryCase* summaryCase : m_cases)
    {
        RimGridSummaryCase* gridSummaryCase = dynamic_cast<RimGridSummaryCase*>(summaryCase);
        if (gridSummaryCase && gridSummaryCase->associatedEclipseCase())
        {
            if (gridSummaryCase->associatedEclipseCase()->gridFileName() == eclipseResultCase->gridFileName())
            {
                return gridSummaryCase;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::findSummaryCaseFromFileName(const QString& fileName) const
{
    // Use QFileInfo object to compare two file names to avoid mix of / and \\

    QFileInfo incomingFileInfo(fileName);

    for (RimSummaryCase* summaryCase : m_cases)
    {
        RimFileSummaryCase* fileSummaryCase = dynamic_cast<RimFileSummaryCase*>(summaryCase);
        if (fileSummaryCase)
        {
            QFileInfo summaryFileInfo(fileSummaryCase->summaryHeaderFilename());
            if (incomingFileInfo == summaryFileInfo)
            {
                return fileSummaryCase;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::deleteCase(RimSummaryCase* summaryCase)
{
    m_cases.removeChildObject(summaryCase);
    for (RimSummaryCaseCollection* summaryCaseCollection : m_caseCollections)
    {
        summaryCaseCollection->deleteCase(summaryCase);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCaseCollection(std::vector<RimSummaryCase*> summaryCases)
{
    RimSummaryCaseCollection* summaryCaseCollection = new RimSummaryCaseCollection();
    for (RimSummaryCase* summaryCase : summaryCases)
    {
        m_cases.removeChildObject(summaryCase);
        summaryCaseCollection->addCase(summaryCase);
    }
    m_caseCollections.push_back(summaryCaseCollection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::summaryCase(size_t idx)
{
    return m_cases[idx];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimSummaryCaseMainCollection::summaryCaseCount()
{
    return m_cases.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::allSummaryCases()
{
    std::vector<RimSummaryCase*> cases;
    this->descendantsIncludingThisOfType(cases);

    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadAllSummaryCaseData()
{
    for (RimSummaryCase* sumCase: allSummaryCases())
    {
        if (sumCase) sumCase->loadCase();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::createAndAddSummaryCaseFromEclipseResultCase(RimEclipseResultCase* eclResCase)
{
    QString gridFileName = eclResCase->gridFileName();
    if(RifEclipseSummaryTools::hasSummaryFiles(QDir::toNativeSeparators(gridFileName).toStdString()))
    {
        RimGridSummaryCase* newSumCase = new RimGridSummaryCase();
        this->m_cases.push_back(newSumCase);
        newSumCase->setAssociatedEclipseCase(eclResCase);
        newSumCase->updateOptionSensitivity();
        return newSumCase;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::createAndAddSummaryCaseFromFileName(const QString& fileName)
{
    RimFileSummaryCase* newSumCase = new RimFileSummaryCase();

    this->m_cases.push_back(newSumCase);
    newSumCase->setSummaryHeaderFilename(fileName);
    newSumCase->updateOptionSensitivity();

    return newSumCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseMainCollection::uniqueShortNameForCase(RimSummaryCase* summaryCase)
{
    std::set<QString> allAutoShortNames;

    for (RimSummaryCase* sumCase : m_cases)
    {
        if (sumCase && sumCase != summaryCase)
        {
            allAutoShortNames.insert(sumCase->shortName());
        }
    }

    bool foundUnique = false;

    QString caseName = summaryCase->caseName();
    QString shortName;

    if (caseName.size() > 2)
    {
        QString candidate;
        candidate += caseName[0];

        for (int i = 1; i < caseName.size(); ++i )
        {
            if (allAutoShortNames.count(candidate + caseName[i]) == 0) 
            {
                shortName = candidate + caseName[i];
                foundUnique = true;
                break;
            }
        }
    }
    else
    {
        shortName = caseName.left(2);
        if(allAutoShortNames.count(shortName) == 0)
        {
            foundUnique = true;
        }
    }

    QString candidate = shortName;
    int autoNumber = 0;

    while (!foundUnique)
    {
        candidate = shortName + QString::number(autoNumber++);
        if(allAutoShortNames.count(candidate) == 0)
        {
            shortName = candidate;
            foundUnique = true;
        }
    }

    return shortName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::updateFilePathsFromProjectPath(const QString & newProjectPath, const QString & oldProjectPath)
{
    for (auto summaryCase : m_cases)
    {
        summaryCase->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
    }
}

