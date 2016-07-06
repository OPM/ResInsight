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
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimEclipseResultCase.h"
#include "RimGridSummaryCase.h"
#include "RifEclipseSummaryTools.h"
#include <QDir>


CAF_PDM_SOURCE_INIT(RimSummaryCaseCollection,"SummaryCaseCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::RimSummaryCaseCollection()
{
    CAF_PDM_InitObject("Summary Cases",":/Cases16x16.png","","");

    CAF_PDM_InitFieldNoDefault(&m_cases,"SummaryCases","","","","");
    m_cases.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::~RimSummaryCaseCollection()
{
    m_cases.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::createSummaryCasesFromRelevantEclipseResultCases()
{
    RimProject* proj = nullptr;
    firstAnchestorOrThisOfType(proj);
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
RimSummaryCase* RimSummaryCaseCollection::summaryCase(size_t idx)
{
    return m_cases[idx];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimSummaryCaseCollection::summaryCaseCount()
{
    return m_cases.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::loadAllSummaryCaseData()
{
    for (RimSummaryCase* sumCase: m_cases)
    {
        if (sumCase) sumCase->loadCase();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridSummaryCase* RimSummaryCaseCollection::createAndAddSummaryCaseFromEclipseResultCase(RimEclipseResultCase* eclResCase)
{
    QString gridFileName = eclResCase->gridFileName();
    if(RifEclipseSummaryTools::hasSummaryFiles(QDir::toNativeSeparators(gridFileName).toStdString()))
    {
        RimGridSummaryCase* newSumCase = new RimGridSummaryCase();
        newSumCase->setAssociatedEclipseCase(eclResCase);
        newSumCase->curveDisplayName = uniqueShortNameForCase(newSumCase);
        newSumCase->updateOptionSensitivity();
        this->m_cases.push_back(newSumCase);
        return newSumCase;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseCollection::uniqueShortNameForCase(RimSummaryCase* summaryCase)
{
    QStringList allAutoShortNames;

    for (RimSummaryCase* sumCase : m_cases)
    {
        if (sumCase && sumCase != summaryCase)
        {
            allAutoShortNames.push_back(sumCase->curveDisplayName());
        }
    }

    bool foundUnique = false;

    QString caseName = summaryCase->caseName();
    QString candidateBase = caseName.left(2);
    QString candidate = candidateBase;
    int autoNumber = 0;
    while (!foundUnique)
    {
        bool foundExisting = false;
        for (QString autoName : allAutoShortNames)
        {
            if (autoName.left(candidate.size()) == candidate)
            {
                candidate = candidateBase + QString::number(autoNumber++);
                foundExisting = true;
            }
        }
        
        foundUnique = !foundExisting;
    }

    return candidate;
}

