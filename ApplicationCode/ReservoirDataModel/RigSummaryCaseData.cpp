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

#include "RigSummaryCaseData.h"

#include "RifEclipseSummaryTools.h"
#include "RifReaderEclipseSummary.h"

#include <QDir>
#include <QString>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSummaryCaseData::RigSummaryCaseData(const QString& summaryHeaderFileName)
{
    std::string headerFileName;
    std::vector<std::string> dataFileNames;
    std::string nativeSumHeadFileName = QDir::toNativeSeparators(summaryHeaderFileName).toStdString();
    RifEclipseSummaryTools::findSummaryFiles(nativeSumHeadFileName, &headerFileName, &dataFileNames);

    m_summaryFileReader = new RifReaderEclipseSummary();
    if (!m_summaryFileReader->open(headerFileName, dataFileNames))
    {
        m_summaryFileReader = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSummaryCaseData::~RigSummaryCaseData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RigSummaryCaseData::summaryReader()
{
    return m_summaryFileReader.p();
}


