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

#include "RifReaderObservedData.h"

#include "RifColumnBasedAsciiParser.h"
#include "RifEclipseSummaryAddress.h"

#include "ert/ecl/ecl_sum.h"

#include <string>
#include <assert.h>

#include <QDateTime>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderObservedData::RifReaderObservedData()
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderObservedData::~RifReaderObservedData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderObservedData::open(const std::string& headerFileName, const std::vector<std::string>& dataFileNames)
{

    if (headerFileName.empty()) return false;
    QString data = headerFileName.data();

    return false;
    //RiaParseAsciiData::parseData(data, settings);
    //m_asciiParser = RifColumnBasedAsciiParser(data)
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderObservedData::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values)
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifReaderObservedData::timeStepCount() const
{
    if (m_asciiParser)
    {
        return static_cast<int>(m_asciiParser->timeSteps().size());
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifReaderObservedData::timeSteps() const
{
    std::vector<time_t> timeStepsTime_t;
    
    if (m_asciiParser)
    {
        for (QDateTime timeStep : m_asciiParser->timeSteps())
        {
            time_t t = timeStep.toTime_t();
            timeStepsTime_t.push_back(t);
        }
    }

    return timeStepsTime_t;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderObservedData::buildMetaData()
{
    if (m_allResultAddresses.size() == 0)
    {

    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifReaderObservedData::address(const AsciiData& asciiData, std::string identifierName, RifEclipseSummaryAddress::SummaryVarCategory summaryCategory)
{
    std::string        quantityName;
    int                regionNumber(-1);
    int                regionNumber2(-1);
    std::string        wellGroupName;
    std::string        wellName;
    int                wellSegmentNumber(-1);
    std::string        lgrName;
    int                cellI(-1);
    int                cellJ(-1);
    int                cellK(-1);

    switch (summaryCategory)
    {
    case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        wellGroupName = identifierName;
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL:
        wellName = identifierName;
        break;
    case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        lgrName = identifierName;
        break;
    default:
        break;
    }

    return RifEclipseSummaryAddress(summaryCategory,
        quantityName,
        regionNumber,
        regionNumber2,
        wellGroupName,
        wellName,
        wellSegmentNumber,
        lgrName,
        cellI, cellJ, cellK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifReaderObservedData::unitName(const RifEclipseSummaryAddress& resultAddress)
{
    std::string str = "";
    return str;
}
