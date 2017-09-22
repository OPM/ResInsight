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

#include "RifSummaryReaderInterface.h"

#include <string>

#include <QDateTime>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RifEclipseSummaryAddress>& RifSummaryReaderInterface::allResultAddresses()
{
    return m_allResultAddresses;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifSummaryReaderInterface::fromTimeT(const std::vector<time_t>& timeSteps)
{
    std::vector<QDateTime> a;

    for (size_t i = 0; i < timeSteps.size(); i++)
    {
        QDateTime dt = QDateTime::fromTime_t(timeSteps[i]);
        a.push_back(dt);
    }

    return a;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryReaderInterface::hasAddress(const RifEclipseSummaryAddress& resultAddress)
{
    for (RifEclipseSummaryAddress summaryAddress : m_allResultAddresses)
    {
        if (summaryAddress == resultAddress)
        {
            return true;
        }
    }

    return false;
}
