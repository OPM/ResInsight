/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RiaSummaryCurveAddress.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress::RiaSummaryCurveAddress( const RifEclipseSummaryAddress& summaryAddressX, const RifEclipseSummaryAddress& summaryAddressY )
    : m_summaryAddressX( summaryAddressX )
    , m_summaryAddressY( summaryAddressY )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress::RiaSummaryCurveAddress( const RifEclipseSummaryAddress& summaryAddressY )
    : m_summaryAddressX( RifEclipseSummaryAddress::timeAddress() )
    , m_summaryAddressY( summaryAddressY )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RiaSummaryCurveAddress::summaryAddressX() const
{
    return m_summaryAddressX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RiaSummaryCurveAddress::summaryAddressY() const
{
    return m_summaryAddressY;
}
