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

#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"
#include "RimSummaryCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition()
: m_summaryCase(nullptr)
, m_ensemble(nullptr)

{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition(RimSummaryCase* summaryCase,
                                                     const RifEclipseSummaryAddress& summaryAddress,
                                                     RimSummaryCaseCollection* ensemble)
    : m_summaryCase(summaryCase)
    , m_ensemble(ensemble)
    , m_summaryAddress(summaryAddress)
{
  
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RiaSummaryCurveDefinition::summaryCase() const
{
    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RiaSummaryCurveDefinition::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RifEclipseSummaryAddress& RiaSummaryCurveDefinition::summaryAddress() const
{
    return m_summaryAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::isEnsembleCurve() const
{
    return m_ensemble != nullptr;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinition::resultValues(const RiaSummaryCurveDefinition& curveDefinition, std::vector<double>* values)
{
    CVF_ASSERT(values);

    if (!curveDefinition.summaryAddress().isValid()) return;
    if (!curveDefinition.summaryCase()) return;
    
    RifSummaryReaderInterface* reader = curveDefinition.summaryCase()->summaryReader();
    if (!reader) return;

    reader->values(curveDefinition.summaryAddress(), values);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RiaSummaryCurveDefinition::timeSteps(const RiaSummaryCurveDefinition& curveDefinition)
{
    static std::vector<time_t> dummy;

    if (!curveDefinition.summaryAddress().isValid()) return dummy;
    if (!curveDefinition.summaryCase()) return dummy;

    RifSummaryReaderInterface* reader = curveDefinition.summaryCase()->summaryReader();
    if (!reader) return dummy;

    return reader->timeSteps(curveDefinition.summaryAddress());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaSummaryCurveDefinition::curveDefinitionText() const
{
    QString caseName;
    if (summaryCase() ) caseName = summaryCase()->caseName();

    return RiaSummaryCurveDefinition::curveDefinitionText(caseName, summaryAddress());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaSummaryCurveDefinition::curveDefinitionText(const QString& caseName, const RifEclipseSummaryAddress& summaryAddress)
{
    QString txt;

    txt += caseName;
    txt += ", ";

    txt += QString::fromStdString(summaryAddress.uiText());

    return txt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::operator<(const RiaSummaryCurveDefinition& other) const
{
    if (m_summaryCase != other.summaryCase())
    {
        return m_summaryCase < other.summaryCase();
    }

    if (m_ensemble != other.ensemble())
    {
        return (m_ensemble < other.ensemble());
    }
    
    return (m_summaryAddress < other.summaryAddress());
}

