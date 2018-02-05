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
QString RimFileSummaryCase::caseName()
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
    m_summaryFileReader = RimFileSummaryCase::findRelatedFilesAndCreateReader(this->summaryHeaderFilename());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimFileSummaryCase::findRelatedFilesAndCreateReader(const QString& headerFileName)
{
    QString headerFileNameStd;
    QStringList dataFileNames;
    QString nativeSumHeadFileName = QDir::toNativeSeparators(headerFileName);
    RifEclipseSummaryTools::findSummaryFiles(nativeSumHeadFileName, &headerFileNameStd, &dataFileNames);

    RifReaderEclipseSummary* summaryFileReader = new RifReaderEclipseSummary;

    if (!summaryFileReader->open(headerFileNameStd, dataFileNames))
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
RifSummaryReaderInterface* RimFileSummaryCase::summaryReader()
{
    return m_summaryFileReader.p();
}
