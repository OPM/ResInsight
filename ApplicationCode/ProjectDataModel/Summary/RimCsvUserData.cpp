/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimCsvUserData.h"

#include "RiaLogging.h"

#include "RifCsvUserData.h"
#include "RifColumnBasedUserDataParser.h"
#include "RifKeywordVectorUserData.h"
#include "RifSummaryReaderInterface.h"

#include "cafUtils.h"

#include <QFile>

CAF_PDM_SOURCE_INIT(RimCsvUserData, "RimCsvUserData");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCsvUserData::RimCsvUserData()
{
    CAF_PDM_InitObject("Observed CSV Data File", ":/Default.png", "", "");
    m_summaryHeaderFilename.uiCapability()->setUiName("File");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCsvUserData::~RimCsvUserData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCsvUserData::createSummaryReaderInterface()
{
    m_summaryReader = nullptr;

    if (caf::Utils::fileExists(this->summaryHeaderFilename()))
    {
        QFile file(this->summaryHeaderFilename());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            RiaLogging::error(QString("Failed to open %1").arg(this->summaryHeaderFilename()));

            return;
        }

        QTextStream in(&file);
        QString fileContents = in.readAll();

        RifCsvUserData* csvUserData = new RifCsvUserData();
        if (csvUserData->parse(fileContents, m_parseOptions, &m_errorText))
        {
            m_summaryReader = csvUserData;
        }
    }
    else
    {
        m_summaryReader = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimCsvUserData::summaryReader()
{
    return m_summaryReader.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCsvUserData::errorMessagesFromReader()
{
    return m_errorText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCsvUserData::setParseOptions(const AsciiDataParseOptions &parseOptions)
{
    m_parseOptions = parseOptions;
}
