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

int RifSummaryReaderInterface::m_nextSerialNumber = 0;

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
const std::set<RifEclipseSummaryAddress>& RifSummaryReaderInterface::allErrorAddresses() const
{
    return m_allErrorAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RifSummaryReaderInterface::errorAddress( const RifEclipseSummaryAddress& resultAddress ) const
{
    RifEclipseSummaryAddress errAddr = resultAddress;
    errAddr.setAsErrorResult();

    return m_allErrorAddresses.find( errAddr ) != m_allErrorAddresses.end() ? errAddr : RifEclipseSummaryAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifSummaryReaderInterface::buildMetaData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifSummaryReaderInterface::keywordCount() const
{
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifSummaryReaderInterface::serialNumber() const
{
    return m_serialNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifSummaryReaderInterface::increaseSerialNumber()
{
#pragma omp critical
    m_serialNumber = m_nextSerialNumber++;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface::RifSummaryReaderInterface()
{
    increaseSerialNumber();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSummaryReaderInterface::hasAddress( const RifEclipseSummaryAddress& resultAddress ) const
{
    static const RifEclipseSummaryAddress defaultAdr = RifEclipseSummaryAddress();
    if ( resultAddress == defaultAdr ) return true;

    if ( !m_allResultAddresses.empty() )
    {
        return ( m_allResultAddresses.count( resultAddress ) > 0 );
    }

    const auto& [isOk, vals] = values( resultAddress );
    return isOk && !vals.empty();
}
