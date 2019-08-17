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

#include "RimFileSummaryCase.h"

#include "RiaLogging.h"

#include "RifEclipseSummaryTools.h"
#include "RifReaderEclipseSummary.h"
#include "RifReaderFmuRft.h"

#include "RimTools.h"

#include <QDir>
#include <QFileInfo>


//==================================================================================================
//
// 
//
//==================================================================================================
CAF_PDM_SOURCE_INIT(RimFileSummaryCase,"FileSummaryCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFileSummaryCase::RimFileSummaryCase()
{
    CAF_PDM_InitField(&m_includeRestartFiles, "IncludeRestartFiles", false, "Include Restart Files", "", "", "");
    CAF_PDM_InitField(&m_includeFmuRftFiles, "IncludeFmuRftFiles", true, "Include FMU Rft Files", "", "", "");
    m_includeRestartFiles.uiCapability()->setUiHidden(true);
    m_includeFmuRftFiles.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFileSummaryCase::~RimFileSummaryCase()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFileSummaryCase::summaryHeaderFilename() const
{
    return m_summaryHeaderFilename();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimFileSummaryCase::caseName() const
{
    QFileInfo caseFileName(this->summaryHeaderFilename());

    return caseFileName.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::updateFilePathsFromProjectPath(const QString & newProjectPath, const QString & oldProjectPath)
{
    m_summaryHeaderFilename = RimTools::relocateFile(m_summaryHeaderFilename(), newProjectPath, oldProjectPath, nullptr, nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::createSummaryReaderInterface()
{
    m_summaryFileReader = RimFileSummaryCase::findRelatedFilesAndCreateReader(this->summaryHeaderFilename(), m_includeRestartFiles);    
    if (m_includeFmuRftFiles)
    {
        findFmuRftDataAndCreateReaders(this->summaryHeaderFilename());
    }
    if (!m_summaryFmuRftReaders.empty())
    {
        RiaLogging::info(QString("Found RFT FMU Data for %1").arg(this->summaryHeaderFilename()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimFileSummaryCase::findRelatedFilesAndCreateReader(const QString& headerFileName, bool includeRestartFiles)
{
    RifReaderEclipseSummary* summaryFileReader = new RifReaderEclipseSummary;

    if (!summaryFileReader->open(headerFileName, includeRestartFiles))
    {
        RiaLogging::warning(QString("Failed to open summary file %1").arg(headerFileName));

        delete summaryFileReader;
        summaryFileReader = nullptr;
    }

    return summaryFileReader;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::findFmuRftDataAndCreateReaders(const QString& headerFileName)
{
    QFileInfo fileInfo(headerFileName);
    QString folder = fileInfo.absolutePath();
    QStringList subDirsContainingFmuRftData = RifReaderFmuRft::findSubDirectoriesWithFmuRftData(folder);

    for (QString subDir : subDirsContainingFmuRftData)
    {
        cvf::ref<RifReaderFmuRft> fmuRftReader(new RifReaderFmuRft(subDir));
        QString errorMsg;
        if (fmuRftReader->initialize(&errorMsg))
        {
            m_summaryFmuRftReaders.push_back(fmuRftReader);
        }
        else
        {
            RiaLogging::error(QString("FMU Rft Error: %1").arg(errorMsg));
        }
    }    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimFileSummaryCase::summaryReader()
{
    return m_summaryFileReader.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::setIncludeRestartFiles(bool includeRestartFiles)
{
    m_includeRestartFiles = includeRestartFiles;
}
