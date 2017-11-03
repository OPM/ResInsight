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

#include "RimObservedEclipseUserData.h"

#include "RiaLogging.h"

#include "RifColumnBasedUserData.h"
#include "RifColumnBasedUserDataParser.h"
#include "RifKeywordVectorParser.h"
#include "RifKeywordVectorUserData.h"
#include "RifSummaryReaderInterface.h"

#include "cafUtils.h"

#include <QFile>

CAF_PDM_SOURCE_INIT(RimObservedEclipseUserData, "RimObservedEclipseUserData");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedEclipseUserData::RimObservedEclipseUserData()
{
    CAF_PDM_InitObject("Observed RSMSPEC Column Based Data File", ":/Default.png", "", "");
    m_summaryHeaderFilename.uiCapability()->setUiName("File");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedEclipseUserData::~RimObservedEclipseUserData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedEclipseUserData::createSummaryReaderInterface()
{
    m_summeryReader = nullptr;

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

        if (RifKeywordVectorParser::canBeParsed(fileContents))
        {
            RifKeywordVectorUserData* keywordVectorUserData = new RifKeywordVectorUserData();
            if (keywordVectorUserData->parse(fileContents, customWellName()))
            {
                m_summeryReader = keywordVectorUserData;
            }
        }
        else
        {
            RifColumnBasedUserData* columnBaseUserData = new RifColumnBasedUserData();
            if (columnBaseUserData->parse(fileContents))
            {
                m_summeryReader = columnBaseUserData;
            }
        }
    }
    else
    {
        m_summeryReader = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimObservedEclipseUserData::summaryReader()
{
    return m_summeryReader.p();
}
