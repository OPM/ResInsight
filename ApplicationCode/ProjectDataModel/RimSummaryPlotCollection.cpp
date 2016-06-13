/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "RimSummaryPlotCollection.h"

#include "RifEclipseSummaryTools.h"
#include "RifReaderEclipseSummary.h"

#include "RimEclipseResultCase.h"
#include "RimSummaryPlot.h"
#include "RimProject.h"

#include "RiuProjectPropertyView.h"

#include <QDockWidget>
#include "RiuMainWindow.h"


CAF_PDM_SOURCE_INIT(RimSummaryPlotCollection, "SummaryPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection::RimSummaryPlotCollection()
{
    CAF_PDM_InitObject("Summary Plots", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_summaryPlots, "SummaryPlots", "Summary Plots",  "", "", "");
    m_summaryPlots.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection::~RimSummaryPlotCollection()
{
    m_summaryPlots.deleteAllChildObjects();

    for (auto it = m_summaryFileReaders.begin(); it != m_summaryFileReaders.end(); it++)
    {
        delete it->second;
    }
    m_summaryFileReaders.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimSummaryPlotCollection::getOrCreateSummaryFileReader(const QString& eclipseCaseFilePathBasename)
{
    auto it = m_summaryFileReaders.find(eclipseCaseFilePathBasename);
    if (it != m_summaryFileReaders.end())
    {
        return it->second;
    }
    else
    {
        return createSummaryFileReader(eclipseCaseFilePathBasename);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimSummaryPlotCollection::getOrCreateSummaryFileReader(const RimEclipseResultCase* eclipseCase)
{
    if (!eclipseCase) return NULL;

    QString caseName = eclipseCase->gridFileName();
    QString caseNameWithNoExtension = caseName.remove(".egrid", Qt::CaseInsensitive);

    QString caseNameAbsPath = caseNameWithNoExtension.replace("/", "\\");

    return this->getOrCreateSummaryFileReader(caseNameAbsPath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimSummaryPlotCollection::createSummaryFileReader(const QString& eclipseCaseFilePathBasename)
{
    std::string headerFile;
    bool isFormatted = false;
    RifEclipseSummaryTools::findSummaryHeaderFile(eclipseCaseFilePathBasename.toStdString(), &headerFile, &isFormatted);
    
    if (headerFile.empty()) return nullptr;

    std::vector<std::string> dataFiles = RifEclipseSummaryTools::findSummaryDataFiles(eclipseCaseFilePathBasename.toStdString());

    if (!dataFiles.size()) return nullptr;

    RifReaderEclipseSummary* reader = new RifReaderEclipseSummary;
    if (!reader->open(headerFile, dataFiles))
    {
        delete reader;

        return nullptr;
    }
    else
    {
        m_summaryFileReaders.insert(std::make_pair(eclipseCaseFilePathBasename, reader));
        return reader;
    }
}

