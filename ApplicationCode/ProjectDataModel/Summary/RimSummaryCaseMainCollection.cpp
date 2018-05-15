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
#include "RifSummaryCaseRestartSelector.h"
#include "RifCaseRealizationParametersReader.h"

#include "RimEclipseResultCase.h"
#include "RimFileSummaryCase.h"
#include "RimGridSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include <QDir>
#include "cafProgressInfo.h"


CAF_PDM_SOURCE_INIT(RimSummaryCaseMainCollection,"SummaryCaseCollection");

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
void addCaseRealizationParametersIfFound(RimSummaryCase& sumCase, const QString modelFolderOrFile)
{
    QString parametersFile = RifCaseRealizationParametersFileLocator::locate(modelFolderOrFile);
    if (!parametersFile.isEmpty())
    {
        RifCaseRealizationParametersReader reader(parametersFile);

        // Try parse case realization parameters
        try
        {
            reader.parse();
            sumCase.setCaseRealizationParameters(reader.parameters());
        }
        catch (...) {}
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::RimSummaryCaseMainCollection()
{
    CAF_PDM_InitObject("Summary Cases",":/SummaryCases16x16.png","","");

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

                for (size_t sccIdx = 0; sccIdx < m_caseCollections.size() && !isFound; ++sccIdx)
                {
                    for (RimSummaryCase* sumCase : m_caseCollections[sccIdx]->allSummaryCases())
                    {
                        RimGridSummaryCase* grdSumCase = dynamic_cast<RimGridSummaryCase*>(sumCase);
                        if (grdSumCase && grdSumCase->associatedEclipseCase() == eclResCase)
                        {
                            isFound = true;
                            break;
                        }
                    }
                }                

                if (!isFound)
                {
                    // Create new GridSummaryCase
                    QStringList summaryFileNames = RifSummaryCaseRestartSelector::getSummaryFilesFromGridFiles(QStringList({ eclResCase->gridFileName() }));
                    if (!summaryFileNames.isEmpty())
                    {
                        RifSummaryCaseFileResultInfo fileInfo(summaryFileNames.front(), false);
                        createAndAddSummaryCasesFromFileInfos({ fileInfo });
                    }
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

    for (auto collection : m_caseCollections)
    {
        for (RimSummaryCase* sumCase : collection->allSummaryCases())
        {
            RimGridSummaryCase* gridSummaryCase = dynamic_cast<RimGridSummaryCase*>(sumCase);
            if (gridSummaryCase && gridSummaryCase->associatedEclipseCase()->gridFileName() == eclipseResultCase->gridFileName())
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

    for (auto collection : m_caseCollections)
    {
        for (RimSummaryCase* summaryCase : collection->allSummaryCases())
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
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::convertGridSummaryCasesToFileSummaryCases(RimGridSummaryCase* gridSummaryCase)
{
    RimFileSummaryCase* fileSummaryCase = gridSummaryCase->createFileSummaryCaseCopy();
    addCaseRealizationParametersIfFound(*fileSummaryCase, fileSummaryCase->summaryHeaderFilename());

    RimSummaryCaseCollection* collection;
    gridSummaryCase->firstAncestorOrThisOfType(collection);

    removeCase(gridSummaryCase);
    delete gridSummaryCase;

    if (collection)
    {
        collection->addCase(fileSummaryCase);
        collection->updateConnectedEditors();
    }
    else
    {
        this->addCase(fileSummaryCase);
        this->updateConnectedEditors();
    }
    loadSummaryCaseData({ fileSummaryCase });
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCases(const std::vector<RimSummaryCase*> cases)
{
    for (RimSummaryCase* sumCase : cases)
    {
        addCase(sumCase);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCase(RimSummaryCase* summaryCase)
{
    m_cases.push_back(summaryCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeCase(RimSummaryCase* summaryCase)
{
    m_cases.removeChildObject(summaryCase);
    for (RimSummaryCaseCollection* summaryCaseCollection : m_caseCollections)
    {
        summaryCaseCollection->removeCase(summaryCase);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCaseCollection(std::vector<RimSummaryCase*> summaryCases, const QString& collectionName, bool isEnsemble)
{
    RimSummaryCaseCollection* summaryCaseCollection = new RimSummaryCaseCollection();
    if(!collectionName.isEmpty()) summaryCaseCollection->setName(collectionName);
    summaryCaseCollection->setAsEnsemble(isEnsemble);

    for (RimSummaryCase* summaryCase : summaryCases)
    {
        RimSummaryCaseCollection* currentSummaryCaseCollection = nullptr;
        summaryCase->firstAncestorOrThisOfType(currentSummaryCaseCollection);
        
        if (currentSummaryCaseCollection)
        {
            currentSummaryCaseCollection->removeCase(summaryCase);
        }
        else
        {
            m_cases.removeChildObject(summaryCase);
        }

        summaryCaseCollection->addCase(summaryCase);
    }

    m_caseCollections.push_back(summaryCaseCollection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeCaseCollection(RimSummaryCaseCollection* caseCollection)
{
    m_caseCollections.removeChildObject(caseCollection);
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
size_t RimSummaryCaseMainCollection::summaryCaseCount() const
{
    return m_cases.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::allSummaryCases() const
{
    std::vector<RimSummaryCase*> cases;
    this->descendantsIncludingThisOfType(cases);

    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::topLevelSummaryCases() const
{
    std::vector<RimSummaryCase*> cases;
    for (const auto& sumCase : m_cases)
    {
        cases.push_back(sumCase);
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCaseCollection*> RimSummaryCaseMainCollection::summaryCaseCollections() const
{
    std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
    for (const auto& caseColl : m_caseCollections)
    {
        summaryCaseCollections.push_back(caseColl);
    }
    return summaryCaseCollections;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadAllSummaryCaseData()
{
    std::vector<RimSummaryCase*> sumCases = allSummaryCases();

    RimSummaryCaseMainCollection::loadSummaryCaseData(sumCases);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadSummaryCaseData(std::vector<RimSummaryCase*> summaryCases)
{
    caf::ProgressInfo progInfo(summaryCases.size(), "Loading Summary Cases");

    for (int cIdx = 0; cIdx < static_cast<int>(summaryCases.size()); ++cIdx)
    {
        RimSummaryCase* sumCase = summaryCases[cIdx];
        if (sumCase)
        {
            sumCase->createSummaryReaderInterface();
            addCaseRealizationParametersIfFound(*sumCase, sumCase->summaryHeaderFilename());
        }

        {
            progInfo.incrementProgress();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::createAndAddSummaryCasesFromFileInfos(const std::vector<RifSummaryCaseFileResultInfo>& summaryHeaderFileInfos)
{
    std::vector<RimSummaryCase*> newCases = createSummaryCasesFromFileInfos(summaryHeaderFileInfos);
    addCases(newCases);
    return newCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::createSummaryCasesFromFileInfos(const std::vector<RifSummaryCaseFileResultInfo>& summaryHeaderFileInfos,
                                                                                           bool showProgress)
{
    RimProject* project = RiaApplication::instance()->project();

    std::vector<RimSummaryCase*> sumCases;

    // Split into two stages to be able to use multi threading
    // First stage : Create summary case objects
    // Second stage : Load data
    {
        std::unique_ptr<caf::ProgressInfo> progress;

        if (showProgress)
        {
            progress.reset(new caf::ProgressInfo(summaryHeaderFileInfos.size(), "Creating summary cases"));
        }

        for (const RifSummaryCaseFileResultInfo& fileInfo : summaryHeaderFileInfos)
        {
            RimEclipseCase* eclCase = nullptr;
            QString gridCaseFile    = RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(fileInfo.summaryFileName());
            if (!gridCaseFile.isEmpty())
            {
                eclCase = project->eclipseCaseFromGridFileName(gridCaseFile);
            }

            if (eclCase)
            {
                RimGridSummaryCase* newSumCase = new RimGridSummaryCase();

                newSumCase->setIncludeRestartFiles(fileInfo.includeRestartFiles());
                newSumCase->setAssociatedEclipseCase(eclCase);
                newSumCase->updateOptionSensitivity();
                sumCases.push_back(newSumCase);
            }
            else
            {
                RimFileSummaryCase* newSumCase = new RimFileSummaryCase();

                newSumCase->setIncludeRestartFiles(fileInfo.includeRestartFiles());
                newSumCase->setSummaryHeaderFileName(fileInfo.summaryFileName());
                newSumCase->updateOptionSensitivity();
                sumCases.push_back(newSumCase);
            }

            if (progress != nullptr) progress->incrementProgress();
        }
    }

    RimSummaryCaseMainCollection::loadSummaryCaseData(sumCases);

    return sumCases;
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

