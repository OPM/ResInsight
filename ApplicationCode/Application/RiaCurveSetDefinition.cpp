/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaCurveSetDefinition.h"

#include "RifSummaryReaderInterface.h"
#include "RimSummaryCaseCollection.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaCurveSetDefinition::RiaCurveSetDefinition() :
    m_ensemble(nullptr)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaCurveSetDefinition::RiaCurveSetDefinition(RimSummaryCaseCollection* ensemble, const RifEclipseSummaryAddress& summaryAddress) :
    m_ensemble(ensemble),
    m_summaryAddress(summaryAddress)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RiaCurveSetDefinition::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RifEclipseSummaryAddress& RiaCurveSetDefinition::summaryAddress() const
{
    return m_summaryAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaCurveSetDefinition::operator<(const RiaCurveSetDefinition& other) const
{
    if (m_ensemble != other.ensemble())
    {
        return (m_ensemble < other.ensemble());
    }

    return (m_summaryAddress < other.summaryAddress());
}
