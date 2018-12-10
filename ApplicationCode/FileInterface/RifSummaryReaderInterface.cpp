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
const std::set<RifEclipseSummaryAddress>& RifSummaryReaderInterface::allResultAddresses() const
{
    return m_allResultAddresses;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifSummaryReaderInterface::errorAddress(const RifEclipseSummaryAddress& resultAddress) const
{
    RifEclipseSummaryAddress errAddr = resultAddress;
    errAddr.setAsErrorResult();

    return m_allErrorAddresses.find(errAddr) != m_allErrorAddresses.end() ? errAddr : RifEclipseSummaryAddress();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryReaderInterface::hasAddress(const RifEclipseSummaryAddress& resultAddress) const
{
    for (const RifEclipseSummaryAddress& summaryAddress : m_allResultAddresses)
    {
        if (summaryAddress == resultAddress)
        {
            return true;
        }
    }

    return false;
}
